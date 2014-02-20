#include <QStringList>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QDate>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QtConcurrent/QtConcurrent>
#include <qglobal.h>
#include "nailab.h"
#include "dbutils.h"
#include "winutils.h"
#include "sampleinput.h"
#include "exceptions.h"

Nailab::Nailab(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);    
    //qApp->setStyle("fusion");
}

Nailab::~Nailab()
{
}

bool Nailab::Initialize()
{	            
    if(!setupEnvironment())
        return false;

    setupDialogs();

    if(!readSettingsXml(envSettingsFile, settings))
        return false;

    if(!readBeakerXml(envBeakerFile, beakers))
        return false;

    if(!readDetectorXml(envDetectorFile, detectors))
        return false;

    if(!setupMCA())
        return false;

    configureWidgets();
    updateSettings();
    updateBeakerViews();
    updateDetectorViews();    

    onPagesChanged(ui.pages->currentIndex());

    bAdminDetectorsEnabled = bAdminBeakersEnabled = bFinishedJobsSelected = true;

    idleTimer = new QTimer();
    connect(idleTimer, SIGNAL(timeout()), this, SLOT(onIdle()));
    idleTimer->start(32);

    return true;
}

bool Nailab::setupEnvironment()
{    
    rootDirectory = QDir::toNativeSeparators(qgetenv(NAILAB_ENVIRONMENT_VARIABLE).constData());
    if(!QDir(rootDirectory).exists())
    {
        QMessageBox::information(this, tr("Error"), tr("Environment variable not found, or invalid: ") + NAILAB_ENVIRONMENT_VARIABLE);
        return false;
    }

    configurationDirectory = QDir::toNativeSeparators(rootDirectory + "/CONFIGURATION/");
    archiveDirectory = QDir::toNativeSeparators(rootDirectory + "/ARCHIVE/");
    tempDirectory = QDir::toNativeSeparators(rootDirectory + "/TEMP/");
    libraryDirectory = QDir::toNativeSeparators(rootDirectory + "/LIBRARY/");

    if(!QDir(configurationDirectory).exists()
        || !QDir(archiveDirectory).exists()
        || !QDir(tempDirectory).exists()
        || !QDir(libraryDirectory).exists())
    {
        QMessageBox::information(this, tr("Error"), tr("One or more nailab system directories not found (CONFIGURATION, ARCHIVE, TEMP, LIBRARY)"));
        return false;
    }

    envSettingsFile.setFileName(configurationDirectory + "settings.xml");
    if(!envSettingsFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envSettingsFile.fileName());
        return false;
    }

    envBeakerFile.setFileName(configurationDirectory + "beaker.xml");
    if(!envBeakerFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envBeakerFile.fileName());
        return false;
    }

    envDetectorFile.setFileName(configurationDirectory + "mca.xml");
    if(!envDetectorFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envDetectorFile.fileName());
        return false;
    }

    envQuantityUnitFile.setFileName(configurationDirectory + "quantity_units.xml");
    if(!envQuantityUnitFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envQuantityUnitFile.fileName());
        return false;
    }

    if(!getWindowsUsername(username))
        return false;

    return true;
}

void Nailab::setupDialogs()
{
    dlgNewBeaker = new CreateBeaker(this);
    connect(dlgNewBeaker, SIGNAL(accepted()), this, SLOT(onNewBeakerAccepted()));

    dlgNewDetector = new CreateDetector(this);
    connect(dlgNewDetector, SIGNAL(accepted()), this, SLOT(onNewDetectorAccepted()));        

    dlgNewDetectorBeaker = new createdetectorbeaker(this);
    connect(dlgNewDetectorBeaker, SIGNAL(accepted()), this, SLOT(onNewDetectorBeakerAccepted()));

    dlgEditDetectorBeaker = new editdetectorbeaker(this);
    connect(dlgEditDetectorBeaker, SIGNAL(accepted()), this, SLOT(onEditDetectorBeakerAccepted()));
}

bool Nailab::setupMCA()
{
    bool found;
    QList<QString> newDetectorNames;

    readGenieDetectorConfig(detectorNames);

    foreach(const QString& dn, detectorNames)
    {
        found = false;
        foreach(const Detector& d, detectors)
        {
            if(d.name == dn)
            {
                found = true;
                break;
            }
        }

        if(!found)
            newDetectorNames.append(dn);
    }

    foreach(const QString& dn, newDetectorNames)
    {
        dlgNewDetector->setName(dn);
        dlgNewDetector->exec();
    }

    vdm = VDM::instance();
    vdm->initialize();

    for(int i=0; i<detectors.count(); i++)    
        detectors[i].maxChannels = vdm->maxChannels(detectors[i].name);    

    return true;
}

void Nailab::configureWidgets()
{
    // ToolGroups
    toolGroups[ui.pageMenu] = new QActionGroup(this);
    toolGroups[ui.pageInput] = new QActionGroup(this);
    toolGroups[ui.pageArchive] = new QActionGroup(this);
    toolGroups[ui.pageAdmin] = new QActionGroup(this);
    toolGroups[ui.pageJobs] = new QActionGroup(this);
    toolGroups[ui.pageDetectors] = new QActionGroup(this);

    toolGroups[ui.tabAdminGeneral] = new QActionGroup(this);
    toolGroups[ui.tabAdminDetectors] = new QActionGroup(this);
    toolGroups[ui.tabAdminBeakers] = new QActionGroup(this);
    toolGroups[ui.tabAdminQA] = new QActionGroup(this);

    toolGroups[ui.tabAdminBeakers]->addAction(ui.actionNewBeaker);
    toolGroups[ui.tabAdminDetectors]->addAction(ui.actionNewDetector);

    // Static menus
    QMenu* menu = new QMenu("File");
    menu->insertAction(NULL, ui.actionExit);
    menu->insertAction(ui.actionExit, ui.actionAdmin);
    ui.menubar->insertMenu(NULL, menu);

    QStringList items;
    items << "STEP" << "LINEAR";
    ui.cboxAdminDetectorContinuumFunction->addItems(items);
    items.clear();
    items << "INTERP" << "LINEAR" << "DUAL" << "EMP";
    ui.cboxAdminDetectorEfficiencyCalibrationType->addItems(items);
    items.clear();
    items << "ALL";
    ui.cboxAdminGeneralSectionName->addItems(items);
    ui.cboxAdminGeneralSectionName->setDisabled(true); // FIXME: Deactivated because there is only one item
    items.clear();
    items << "" << "AREA" << "INTEGRAL" << "COUNT";
    ui.cboxAdminDetectorPresetType1->addItems(items);
    ui.cboxInputSamplePresetType1->addItems(items);
    items.clear();
    items << "" << "REALTIME" << "LIVETIME";
    ui.cboxAdminDetectorPresetType2->addItems(items);
    ui.cboxInputSamplePresetType2->addItems(items);    
    items.clear();
    items << "SECONDS" << "MINUTES" << "HOURS";
    ui.cboxAdminDetectorPresetType2Unit->addItems(items);
    ui.cboxInputSamplePresetType2Unit->addItems(items);

    QStringList quantityUnitList;
    readQuantityUnitsXml(envQuantityUnitFile, quantityUnitList);
    ui.cbInputSampleQuantityUnits->addItems(quantityUnitList);

    // Default tab pages
    ui.pages->setCurrentWidget(ui.pageMenu);
    ui.tabsAdmin->setCurrentWidget(ui.tabAdminGeneral);

    listItemJobs = new QListWidgetItem(QIcon(":/Nailab/Resources/jobs64.png"), tr("Jobs"), ui.lwMenu);
    listItemJobs->setStatusTip(tr("Currently running jobs"));
    listItemDetectors = new QListWidgetItem(QIcon(":/Nailab/Resources/detectors64.png"), tr("Detectors"), ui.lwMenu);
    listItemDetectors->setStatusTip(tr("Available detectors"));
    listItemArchive = new QListWidgetItem(QIcon(":/Nailab/Resources/archive64.png"), tr("Archive"), ui.lwMenu);
    listItemArchive->setStatusTip(tr("Show file archive"));
    connect(ui.lwMenu, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(onMenuSelect(QListWidgetItem*)));

    // Archive view
    modelArchive = new QFileSystemModel(this);
    modelArchive->setNameFilters(QStringList() << "*.RPT");
    modelArchive->setNameFilterDisables(false);
    ui.tvArchive->setModel(modelArchive);
    ui.tvArchive->setRootIndex(modelArchive->setRootPath(archiveDirectory));

    // Running Jobs view
    modelRunningJobs = new QFileSystemModel(this);
    modelRunningJobs->setNameFilters(QStringList() << "*.BAT");
    modelRunningJobs->setNameFilterDisables(false);
    ui.lvRunningJobs->setModel(modelRunningJobs);
    ui.lvRunningJobs->setRootIndex(modelRunningJobs->setRootPath(tempDirectory));
    //ui.lvRunningJobs->setStyleSheet("QListView::item {background-image: url(:/Nailab/Resources/jobs64.png);}");

    // Finished Jobs view
    modelFinishedJobs = new QFileSystemModel(this);
    modelFinishedJobs->setNameFilters(QStringList() << "*.DONE");
    modelFinishedJobs->setNameFilterDisables(false);
    ui.lvFinishedJobs->setModel(modelFinishedJobs);
    ui.lvFinishedJobs->setRootIndex(modelFinishedJobs->setRootPath(tempDirectory));
    //ui.lvFinishedJobs->setStyleSheet("QListView::item {image: url(:/Nailab/Resources/jobs64.png);}");

    // Dates
    ui.dtInputSampleNoneSampleDate->setDate(QDate::currentDate());
    ui.dtInputSampleDepositBeginDate->setDate(QDate::currentDate());
    ui.dtInputSampleDepositEndDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradBeginDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradEndDate->setDate(QDate::currentDate());    
}

void Nailab::enableControlTree(QObject *parent, bool enable)
{
    if(parent->isWidgetType())
    {
        qobject_cast<QWidget*>(parent)->setEnabled(enable);
        foreach(QObject *obj, parent->children())
            enableControlTree(obj, enable);
    }
}

void Nailab::updateSettings()
{
    ui.tbAdminGeneralGenieFolder->setText(settings.genieFolder);    
    ui.tbAdminGeneralTemplateName->setText(settings.templateName);
    ui.cboxAdminGeneralSectionName->setCurrentText(settings.sectionName);
    ui.tbAdminGeneralErrorMultiplier->setText(QString::number(settings.errorMultiplier));
    ui.tbAdminGeneralNAIImport->setText(settings.NAIImportFolder);
    ui.tbAdminGeneralRPTExport->setText(settings.RPTExportFolder);
}

void Nailab::updateBeakerViews()
{
    // Set up admin beaker list
    ui.lvAdminBeakers->clear();
    QListWidgetItem *item = 0;
    foreach(const Beaker &beaker, beakers)
    {
        item = new QListWidgetItem(QIcon(":/Nailab/Resources/beaker16.png"), beaker.name, ui.lvAdminBeakers);
    }
}

void Nailab::updateDetectorViews()
{    
    // Remove existing items
    ui.lvAdminDetectors->clear();        
    ui.lwDetectors->clear();

    bool found;
    QListWidgetItem *item = 0;

    // Set up admin detector list and detector buttons
    foreach(const Detector &detector, detectors)
    {
        found = false;
        item = new QListWidgetItem(QIcon(":/Nailab/Resources/detector16.png"), detector.name, ui.lvAdminDetectors);
        item = new QListWidgetItem(QIcon(":/Nailab/Resources/detector64.png"), detector.name, ui.lwDetectors);

        foreach(const QString& d, detectorNames)
        {
            if(detector.name == d)
                found = true;
        }

        if(!detector.inUse)        
            item->setHidden(true);        
        else if(!found)        
            disableListWidgetItem(item);        
        else if(vdm->isBusy(detector.name))
            disableListWidgetItem(item);
    }
    connect(ui.lwDetectors, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onDetectorSelect(QListWidgetItem*)));
}

void Nailab::disableListWidgetItem(QListWidgetItem *item)
{            
    item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
}

Detector* Nailab::getDetectorByName(const QString& name)
{
    for(int i=0; i<detectors.count(); i++)
    {
        if(detectors[i].name == name)
            return &detectors[i];
    }
    return NULL;
}

bool Nailab::validateSampleInput()
{
    // FIXME: Not implemented
    return true;
}

void Nailab::storeSampleInput(SampleInput& sampleInput)
{
    sampleInput.detector = ui.lblInputSampleDetector->text();
    sampleInput.title = ui.tbInputSampleProject->text();
    sampleInput.username = ui.tbInputSampleCollector->text();
    sampleInput.description = ui.tbInputSampleDescription->text();
    sampleInput.specterref = ui.tbInputSampleSpecterRef->text();
    sampleInput.ID = ui.tbInputSampleID->text();
    sampleInput.type = ui.tbInputSampleType->text();
    sampleInput.quantity = ui.tbInputSampleQuantity->text();
    sampleInput.quantityError = ui.tbInputSampleQuantityUncertainty->text();
    sampleInput.units = ui.cbInputSampleQuantityUnits->currentText();
    sampleInput.geometry = ui.cbInputSampleGeometry->currentText();
    switch(ui.tabsInputSampleBuildupType->currentIndex())
    {
    case 0:
        sampleInput.builduptype = "DEPOSIT";
        sampleInput.startTime = ui.dtInputSampleDepositBeginDate->text();
        sampleInput.endTime = ui.dtInputSampleDepositEndDate->text();
        break;
    case 1:
        sampleInput.builduptype = "IRRAD";
        sampleInput.startTime = ui.dtInputSampleIrradBeginDate->text();
        sampleInput.endTime = ui.dtInputSampleIrradEndDate->text();
        break;
    case 2:
        sampleInput.builduptype = "NONE";
        sampleInput.startTime = ui.dtInputSampleNoneSampleDate->text();
        break;
    }
    sampleInput.randomError = ui.tbInputSampleRandomError->text();
    sampleInput.systematicError = ui.tbInputSampleSystematicError->text();
    sampleInput.presetType1 = ui.cboxInputSamplePresetType1->currentText();
    sampleInput.presetType1Value = ui.tbInputSamplePresetType1->text();
    sampleInput.presetType1StartChannel = ui.tbInputSampleStartChannel->text();
    sampleInput.presetType1EndChannel = ui.tbInputSampleEndChannel->text();
    sampleInput.presetType2 = ui.cboxInputSamplePresetType2->currentText();
    sampleInput.presetType2Value = ui.tbInputSamplePresetType2->text();
}

bool Nailab::startJob(SampleInput& sampleInput)
{
    QString baseFilename = tempDirectory + sampleInput.detector;

    QFile jobfile(baseFilename + ".BAT");
    if(!jobfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::information(this, tr("Error"), tr("Unable to open job file"));
        return false;
    }
    QTextStream stream(&jobfile);    

    Detector* detector = getDetectorByName(sampleInput.detector);
    QString specname = detector->name + "-" + QString::number(detector->spectrumCounter + 1);

    startJobCommand(stream, "pars");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "/stitle=", sampleInput.title);
    addJobParamQuoted(stream, "/scollname=", username);
    addJobParamQuoted(stream, "/sdesc1=", sampleInput.description);
    addJobParamQuoted(stream, "/sdesc4=", sampleInput.specterref);
    addJobParamQuoted(stream, "/sident=", sampleInput.ID);
    addJobParamQuoted(stream, "/stype=", sampleInput.type);
    addJobParam(stream, "/squant=", sampleInput.quantity);
    addJobParam(stream, "/squanterr=", sampleInput.quantityError);
    addJobParamQuoted(stream, "/sunits=", sampleInput.units);
    addJobParamQuoted(stream, "/sgeomtry=", sampleInput.geometry);
    addJobParamQuoted(stream, "/builduptype=", sampleInput.builduptype);    
    addJobParamQuoted(stream, "/stime=", sampleInput.startTime);
    if(sampleInput.builduptype == "IRRAD" || sampleInput.builduptype == "DEPOSIT")
        addJobParamQuoted(stream, "/sdeposit=", sampleInput.endTime);
    endJobCommand(stream);

    startJobCommand(stream, "pars");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParam(stream, "/ssyserr=", sampleInput.randomError);
    addJobParam(stream, "/ssysterr=", sampleInput.systematicError);
    endJobCommand(stream);    

    startJobCommand(stream, "movedata");    
    addJobParamQuoted(stream, "", detector->beakers[sampleInput.geometry]);
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamSingle(stream, "/effcal");
    addJobParamSingle(stream, "/overwrite");
    endJobCommand(stream);

    startJobCommand(stream, "startmca");
    addJobParam(stream, "det:", sampleInput.detector);

    QString presetType = "";
    if(sampleInput.presetType1 == "AREA")
        presetType = "/AREAPRESET=";
    else if(sampleInput.presetType1 == "INTEGRAL")
        presetType = "/INTPRESET=";
    else if(sampleInput.presetType1 == "COUNT")
        presetType = "/CNTSPRESET=";        

    if(!presetType.isEmpty())
        addJobParam(stream, presetType, sampleInput.presetType1Value + "," +
                    sampleInput.presetType1StartChannel + "," + sampleInput.presetType1EndChannel);

    presetType = "";
    if(sampleInput.presetType2 == "REALTIME")
        presetType = "/REALPRESET=";
    else if(sampleInput.presetType2 == "LIVETIME")
        presetType = "/LIVEPRESET=";

    if(!presetType.isEmpty())
        addJobParam(stream, presetType, sampleInput.presetType2Value);

    endJobCommand(stream);

    startJobCommand(stream, "wait");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamSingle(stream, "/acq");
    endJobCommand(stream);    

    startJobCommand(stream, "peak_dif");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParam(stream, "/channels=", QString::number(detector->searchRegionStart) + "," + QString::number(detector->searchRegionEnd));
    addJobParam(stream, "/signif=", QString::number(detector->significanceTreshold));
    addJobParam(stream, "/ftol=", QString::number(detector->tolerance));
    endJobCommand(stream);    

    startJobCommand(stream, "area_nl1");
    addJobParam(stream, "det:", sampleInput.detector);    
    addJobParam(stream, "/channels=", QString::number(detector->peakAreaRegionStart) + "," + QString::number(detector->peakAreaRegionEnd));
    addJobParam(stream, "/fcont=", QString::number(detector->continuum));
    if(detector->criticalLevelTest)
        addJobParamSingle(stream, "/critlevel");
    if(detector->useFixedFWHM)
        addJobParamSingle(stream, "/fixfwhm");
    if(detector->useFixedTailParameter)
        addJobParamSingle(stream, "/fixtail");
    if(detector->fitSinglets)
        addJobParamSingle(stream, "/fit");
    if(detector->displayROIs)
        addJobParamSingle(stream, "/display_rois");
    endJobCommand(stream);

    startJobCommand(stream, "pars");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParam(stream, "/roipsbtyp=", detector->continuumFunction);
    addJobParam(stream, "/prreject0pks=", detector->rejectZeroAreaPeaks ? "1" : "0");
    addJobParam(stream, "/prfwhmpkmult=", QString::number(detector->maxFWHMsBetweenPeaks));
    addJobParam(stream, "/prfwhmpkleft=", QString::number(detector->maxFWHMsForLeftLimit));
    addJobParam(stream, "/prfwhmpkrght=", QString::number(detector->maxFWHMsForRightLimit));
    endJobCommand(stream);

    startJobCommand(stream, "areacor");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "/bkgnd=", detector->backgroundSubtract);
    endJobCommand(stream);

    startJobCommand(stream, "effcor");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamSingle(stream, "/" + detector->efficiencyCalibrationType);
    endJobCommand(stream);

    startJobCommand(stream, "nid_intf");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "/LIBRARY=", detector->NIDLibrary);
    addJobParam(stream, "/CONFID=", QString::number(detector->NIDConfidenceTreshold));
    if(detector->performMDATest)
        addJobParamSingle(stream, "/MDA_TEST");
    if(detector->inhibitATDCorrection)
        addJobParamSingle(stream, "/NOACQDECAY");
    endJobCommand(stream);

    startJobCommand(stream, "pars");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParam(stream, "/PRUSESTRLIB=", detector->useStoredLibrary ? "1" : "0");
    addJobParam(stream, "/MDACONFID=", QString::number(detector->MDAConfidenceFactor));
    endJobCommand(stream);

    startJobCommand(stream, "MDA");
    addJobParam(stream, "det:", sampleInput.detector);
    endJobCommand(stream);

    startJobCommand(stream, "pars");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "/activunits=", "Bq");
    addJobParam(stream, "/ACTIVMULT=", "37000");
    endJobCommand(stream);

    startJobCommand(stream, "report");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "/template=", settings.templateName);
    addJobParamSingle(stream, "/newfile");
    addJobParamSingle(stream, "/firstpg");
    addJobParamSingle(stream, "/newpg");
    addJobParamQuoted(stream, "/outfile=", baseFilename + ".RPT");
    addJobParamQuoted(stream, "/section=", "");
    addJobParam(stream, "/EM=", QString::number(settings.errorMultiplier));
    endJobCommand(stream);

    /*startJobCommand(stream, "dataplot");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParam(stream, "/scale=", "log");
    addJobParamSingle(stream, "/enhplot");
    endJobCommand(stream);*/

    startJobCommand(stream, "movedata");
    addJobParam(stream, "det:", sampleInput.detector);
    addJobParamQuoted(stream, "", baseFilename + ".CNF");
    addJobParamSingle(stream, "/overwrite");
    endJobCommand(stream);

    startJobCommand(stream, "copy");
    addJobParamSingle(stream, "/y NUL \"" + baseFilename + ".DONE\" >NUL");
    endJobCommand(stream);

    jobfile.close();    

    QtConcurrent::run(runJob, baseFilename + ".BAT >" + baseFilename + ".OUT " + "2>" + baseFilename + ".ERR");    

    return true;
}

void Nailab::startJobCommand(QTextStream& s, const QString& cmd)
{
    s << cmd << " ";
}

void Nailab::endJobCommand(QTextStream& s)
{
    s << "\n";
}

void Nailab::addJobParam(QTextStream& s, const QString& p, const QString& v)
{
    s << p << v << " ";
}

void Nailab::addJobParamSingle(QTextStream& s, const QString& v)
{
    s << v << " ";
}

void Nailab::addJobParamQuoted(QTextStream& s, const QString& p, const QString& v)
{
    s << p << "\"" << v << "\" ";
}

bool Nailab::detectorHasJob(const Detector* det)
{
    QString filename = tempDirectory + det->name + ".BAT";

    if(QFile::exists(filename))
        return true;

    return false;
}

void Nailab::showBeakersForDetector(Detector *detector)
{
    for(int i=0; i<ui.twAdminDetectorBeaker->rowCount(); i++)
        ui.twAdminDetectorBeaker->removeRow(i);

    ui.twAdminDetectorBeaker->setRowCount(detector->beakers.count());

    QMapIterator<QString, QString> iter(detector->beakers);
    int i = 0;
    while (iter.hasNext())
    {
        iter.next();
        ui.twAdminDetectorBeaker->setCellWidget(i, 0, new QLabel(iter.key()));
        ui.twAdminDetectorBeaker->setCellWidget(i, 1, new QLabel(iter.value()));
        i++;
    }
}

void Nailab::onIdle()
{
    if(ui.lvAdminDetectors->selectedItems().count() > 0 && !bAdminDetectorsEnabled)
    {
        enableControlTree(ui.tabsAdminDetectors, true);
        bAdminDetectorsEnabled = true;
    }
    else if(ui.lvAdminDetectors->selectedItems().count() <= 0 && bAdminDetectorsEnabled)
    {
        enableControlTree(ui.tabsAdminDetectors, false);
        bAdminDetectorsEnabled = false;
    }

    if(ui.lvAdminBeakers->selectedItems().count() > 0 && !bAdminBeakersEnabled)
    {
        enableControlTree(ui.frameAdminBeakers, true);
        bAdminBeakersEnabled = true;
    }
    else if(ui.lvAdminBeakers->selectedItems().count() <= 0 && bAdminBeakersEnabled)
    {
        enableControlTree(ui.frameAdminBeakers, false);
        bAdminBeakersEnabled = false;
    }

    if(ui.lvFinishedJobs->selectionModel()->selectedIndexes().count() > 0 && !bFinishedJobsSelected)
    {
        ui.btnJobShow->setEnabled(true);
        ui.btnJobPrint->setEnabled(true);
        ui.btnJobStore->setEnabled(true);
        ui.btnJobReject->setEnabled(true);
        bFinishedJobsSelected = true;
    }
    else if(ui.lvFinishedJobs->selectionModel()->selectedIndexes().count() <= 0 && bFinishedJobsSelected)
    {
        ui.btnJobShow->setEnabled(false);
        ui.btnJobPrint->setEnabled(false);
        ui.btnJobStore->setEnabled(false);
        ui.btnJobReject->setEnabled(false);
        bFinishedJobsSelected = false;
    }
}

void Nailab::onQuit()
{
    vdm->close();
    qApp->quit();
}

void Nailab::onBack()
{
    QWidget *page = ui.pages->currentWidget();
    if(page == ui.pageAdmin || page == ui.pageJobs || page == ui.pageDetectors || page == ui.pageArchive)
        ui.pages->setCurrentWidget(ui.pageMenu);
    else if(page == ui.pageInput)
        ui.pages->setCurrentWidget(ui.pageDetectors);           
}

void Nailab::onAdmin()
{
    ui.pages->setCurrentWidget(ui.pageAdmin);
}

void Nailab::onMenuSelect(QListWidgetItem *item)
{        
    if(item == listItemJobs)    
        ui.pages->setCurrentWidget(ui.pageJobs);    
    else if(item == listItemDetectors)
        ui.pages->setCurrentWidget(ui.pageDetectors);
    else if(item == listItemArchive)
        ui.pages->setCurrentWidget(ui.pageArchive);

    item->setSelected(false);
}

void Nailab::onDetectorSelect(QListWidgetItem *item)
{    
    if(item->flags() & Qt::ItemIsSelectable)
    {
        item->setSelected(false);

        const Detector* det = getDetectorByName(item->text());
        if(!det)
            return; // FIXME: report error

        if(detectorHasJob(det))
        {
            QMessageBox::information(this, tr("Message"), tr("Detector ") + det->name + tr(" has a job"));
            return;
        }

        if(!vdm->hasHighVoltage(det->name))
        {
            QMessageBox::information(this, tr("Message"), tr("Detector ") + det->name + tr(" is powered off"));
            return;
        }

        ui.pages->setCurrentWidget(ui.pageInput);
        ui.tabsInput->setCurrentWidget(ui.tabInputSample);
        ui.toolsInputSample->setCurrentIndex(0); // Parameters
        ui.tabsInputSampleBuildupType->setCurrentIndex(2); // None
        ui.lblInputSampleDetector->setText(item->text());        
        ui.tbInputSampleCollector->setText(username);
        ui.tbInputSampleSpecterRef->setText(QString::number(det->spectrumCounter));
        ui.cbInputSampleGeometry->clear();        
        ui.cbInputSampleGeometry->addItems(det->beakers.keys());
        ui.cboxInputSamplePresetType1->setCurrentText(det->presetType1);
        ui.tbInputSamplePresetType1->setText(QString::number(det->presetType1Value));        
        ui.cboxInputSamplePresetType2->setCurrentText(det->presetType2);
        ui.tbInputSamplePresetType2->setText(QString::number(det->presetType2Value));        
        ui.tbInputSampleStartChannel->setText(QString::number(det->presetType1ChannelStart));
        ui.tbInputSampleEndChannel->setText(QString::number(det->presetType1ChannelEnd));
        ui.tbInputSampleRandomError->setText(QString::number(det->randomError));
        ui.tbInputSampleSystematicError->setText(QString::number(det->systematicError));
        // FIXME: Not finished...
    }
}

void Nailab::onPagesChanged(int index)
{
    foreach(QWidget *w, toolGroups.keys())
        toolGroups.value(w)->setVisible(false);

    if(toolGroups.contains(ui.pages->widget(index)))
        toolGroups[ui.pages->widget(index)]->setVisible(true);

    if(ui.pages->currentWidget() == ui.pageAdmin)
        onTabsAdminChanged(ui.tabsAdmin->currentIndex());    
}

void Nailab::onTabsAdminChanged(int index)
{
    for(int i = 0; i < ui.tabsAdmin->count(); i++)
        if(toolGroups.contains(ui.tabsAdmin->widget(i)))
            toolGroups[ui.tabsAdmin->widget(i)]->setVisible(false);

    if(toolGroups.contains(ui.tabsAdmin->widget(index)))
        toolGroups[ui.tabsAdmin->widget(index)]->setVisible(true);
}

void Nailab::onAdminGeneralAccepted()
{       
    settings.genieFolder = ui.tbAdminGeneralGenieFolder->text();        
    settings.templateName = ui.tbAdminGeneralTemplateName->text();
    settings.sectionName = ui.cboxAdminGeneralSectionName->currentText();
    settings.errorMultiplier = ui.tbAdminGeneralErrorMultiplier->text().toDouble();
    settings.NAIImportFolder = ui.tbAdminGeneralNAIImport->text();
    settings.RPTExportFolder = ui.tbAdminGeneralRPTExport->text();

    writeSettingsXml(envSettingsFile, settings);
}

void Nailab::onNewBeaker()
{    
    dlgNewBeaker->exec();
}

void Nailab::onNewBeakerAccepted()
{
    Beaker beaker(dlgNewBeaker->name(), dlgNewBeaker->manufacturer(), dlgNewBeaker->enabled());
    beakers.push_back(beaker);
    writeBeakerXml(envBeakerFile, beakers);
    updateBeakerViews();
}

void Nailab::onAdminBeakersAccepted()
{
    if(ui.lvAdminBeakers->selectedItems().count() < 1)
        return;

    QString beakerName = ui.lvAdminBeakers->selectedItems()[0]->text();
    for(int i=0; i<beakers.count(); i++)
    {
        if(beakers[i].name == beakerName)
        {
            beakers[i].manufacturer = ui.tbAdminBeakerManufacturer->text();
            beakers[i].enabled = ui.cbAdminBeakerEnabled->isChecked();
            break;
        }
    }
    writeBeakerXml(envBeakerFile, beakers);
}

void Nailab::onNewDetector()
{	    
    //dlgNewDetector->setName("d8"); // FIXME
    //dlgNewDetector->exec();
}

void Nailab::onNewDetectorAccepted()
{
    // FIXME: Validate input...
    Detector detector;
    detector.name = dlgNewDetector->name();
    detector.enabled = dlgNewDetector->enabled();
    detector.inUse = true;
    detector.searchRegionStart = dlgNewDetector->searchRegionStart();
    detector.searchRegionEnd = dlgNewDetector->searchRegionEnd();
    detector.significanceTreshold = dlgNewDetector->significanceTreshold();
    detector.tolerance = dlgNewDetector->tolerance();
    detector.peakAreaRegionStart = dlgNewDetector->peakAreaRegionStart();
    detector.peakAreaRegionEnd = dlgNewDetector->peakAreaRegionEnd();
    detector.continuum = dlgNewDetector->continuum();
    detector.continuumFunction = dlgNewDetector->continuumFunction();
    detector.criticalLevelTest = dlgNewDetector->criticalLevelTest();
    detector.useFixedFWHM = dlgNewDetector->useFixedFWHM();
    detector.useFixedTailParameter = dlgNewDetector->useFixedTailParameter();
    detector.fitSinglets = dlgNewDetector->fitSinglets();
    detector.displayROIs = dlgNewDetector->displayROIs();
    detector.rejectZeroAreaPeaks = dlgNewDetector->rejectZeroAreaPeaks();
    detector.maxFWHMsBetweenPeaks = dlgNewDetector->maxFWHMsBetweenPeaks();
    detector.maxFWHMsForLeftLimit = dlgNewDetector->maxFWHMsForLeftLimit();
    detector.maxFWHMsForRightLimit = dlgNewDetector->maxFWHMsForRightLimit();
    detector.backgroundSubtract = dlgNewDetector->backgroundSubtract();
    detector.efficiencyCalibrationType = dlgNewDetector->efficiencyCalibrationType();    
    detector.presetType1 = dlgNewDetector->presetType1();
    detector.presetType1Value = dlgNewDetector->presetType1Value();
    detector.presetType1ChannelStart = 1; // FIXME
    detector.presetType1ChannelEnd = 1024; // FIXME
    detector.presetType2 = dlgNewDetector->presetType2();
    detector.presetType2Value = dlgNewDetector->presetType2Value();
    detector.presetType2Unit = dlgNewDetector->presetType2Unit();
    detector.randomError = dlgNewDetector->randomError();
    detector.systematicError = dlgNewDetector->systematicError();
    detector.spectrumCounter = 0;
    // FIXME: Set at creation
    detector.NIDLibrary = "";
    detector.NIDConfidenceTreshold = 0.1;
    detector.MDAConfidenceFactor = 0.1;
    detector.performMDATest = false;
    detector.inhibitATDCorrection = false;
    detector.useStoredLibrary = false;

    detectors.push_back(detector);
    writeDetectorXml(envDetectorFile, detectors);
    updateDetectorViews();

    detector.maxChannels = vdm->maxChannels(detector.name);
}

void Nailab::onNewDetectorBeakerAccepted()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    QString beaker = dlgNewDetectorBeaker->beaker();
    QString calfile = dlgNewDetectorBeaker->calfile();
    if(beaker.isEmpty() || calfile.isEmpty())
        return;

    Detector* d = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());
    d->beakers[beaker] = calfile;
    writeDetectorXml(envDetectorFile, detectors);
    showBeakersForDetector(d);
}

void Nailab::onEditDetectorBeakerAccepted()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    QString beaker = dlgEditDetectorBeaker->beaker();
    QString calfile = dlgEditDetectorBeaker->calfile();
    if(beaker.isEmpty() || calfile.isEmpty())
        return;

    Detector* d = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());
    d->beakers[beaker] = calfile;
    writeDetectorXml(envDetectorFile, detectors);
    showBeakersForDetector(d);
}

void Nailab::onAdminDetectorsAccepted()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;    

    QString detectorName = ui.lvAdminDetectors->selectedItems()[0]->text();
    Detector* detector = getDetectorByName(detectorName);

    detector->inUse = ui.cbAdminDetectorInUse->isChecked();
    detector->searchRegionStart = ui.tbAdminDetectorSearchRegionStart->text().toInt();
    detector->searchRegionEnd = ui.tbAdminDetectorSearchRegionEnd->text().toInt();
    detector->significanceTreshold = ui.tbAdminDetectorSignificanceTreshold->text().toDouble();
    detector->tolerance = ui.tbAdminDetectorTolerance->text().toDouble();
    detector->peakAreaRegionStart = ui.tbAdminDetectorPeakAreaRegionStart->text().toInt();
    detector->peakAreaRegionEnd = ui.tbAdminDetectorPeakAreaRegionEnd->text().toInt();
    detector->continuum = ui.tbAdminDetectorContinuum->text().toDouble();
    detector->continuumFunction = ui.cboxAdminDetectorContinuumFunction->currentText();
    detector->criticalLevelTest = ui.cbAdminDetectorCriticalLevelTest->isChecked();
    detector->useFixedFWHM = ui.cbAdminDetectorUseFixedFWHM->isChecked();
    detector->useFixedTailParameter = ui.cbAdminDetectorUseFixedTailParameter->isChecked();
    detector->fitSinglets = ui.cbAdminDetectorFitSinglets->isChecked();
    detector->displayROIs = ui.cbAdminDetectorDisplayROIs->isChecked();
    detector->rejectZeroAreaPeaks = ui.cbAdminDetectorRejectZeroAreaPeaks->isChecked();
    detector->maxFWHMsBetweenPeaks = ui.tbAdminDetectorMaxFWHMsBetweenPeaks->text().toDouble();
    detector->maxFWHMsForLeftLimit = ui.tbAdminDetectorMaxFWHMsForLeftLimit->text().toDouble();
    detector->maxFWHMsForRightLimit = ui.tbAdminDetectorMaxFWHMsForRightLimit->text().toDouble();
    detector->backgroundSubtract = ui.tbAdminDetectorBackgroundSubtract->text();
    detector->efficiencyCalibrationType = ui.cboxAdminDetectorEfficiencyCalibrationType->currentText();
    detector->presetType1 = ui.cboxAdminDetectorPresetType1->currentText();
    detector->presetType1Value = ui.tbAdminDetectorPresetType1Value->text().toDouble();
    detector->presetType1ChannelStart = ui.tbAdminDetectorPresetType1StartChannel->text().toInt();
    detector->presetType1ChannelEnd = ui.tbAdminDetectorPresetType1EndChannel->text().toInt();
    detector->presetType2 = ui.cboxAdminDetectorPresetType2->currentText();
    detector->presetType2Value = ui.tbAdminDetectorPresetType2Value->text().toDouble();
    detector->presetType2Unit = ui.cboxAdminDetectorPresetType2Unit->currentText();
    detector->randomError = ui.tbAdminDetectorRandomError->text().toDouble();
    detector->systematicError = ui.tbAdminDetectorSystematicError->text().toDouble();    
    detector->NIDLibrary = ui.tbAdminDetectorNIDLibrary->text();
    detector->NIDConfidenceTreshold = ui.tbAdminDetectorNIDConfidenceTreshold->text().toDouble();
    detector->MDAConfidenceFactor = ui.tbAdminDetectorMDAConfidenceFactor->text().toDouble();
    detector->performMDATest = ui.cbAdminDetectorPerformMDATest->isChecked();
    detector->inhibitATDCorrection = ui.cbAdminDetectorInhibitATDCorrection->isChecked();
    detector->useStoredLibrary = ui.cbAdminDetectorUseStoredLibrary->isChecked();

    writeDetectorXml(envDetectorFile, detectors);
    updateDetectorViews();
}

void Nailab::onLvAdminBeakersCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if(!current)
        return;                

    QString beakerName = current->text();
    ui.lblAdminBeakerBeaker->setText(beakerName);
    foreach(const Beaker &beaker, beakers)
    {
        if(beaker.name == beakerName)
        {
            ui.tbAdminBeakerManufacturer->setText(beaker.manufacturer);
            ui.cbAdminBeakerEnabled->setChecked(beaker.enabled);            
            break;
        }
    }
}

void Nailab::onLvAdminDetectorsCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);        

    if(!current)
        return;        

    Detector* detector = getDetectorByName(current->text());
    if(!detector)
        return;

    ui.lblAdminDetectorDetector->setText(detector->name);
    ui.cbAdminDetectorInUse->setChecked(detector->inUse);
    ui.tbAdminDetectorSearchRegionStart->setText(QString::number(detector->searchRegionStart));
    ui.tbAdminDetectorSearchRegionEnd->setText(QString::number(detector->searchRegionEnd));
    ui.tbAdminDetectorSignificanceTreshold->setText(QString::number(detector->significanceTreshold));
    ui.tbAdminDetectorTolerance->setText(QString::number(detector->tolerance));
    ui.tbAdminDetectorPeakAreaRegionStart->setText(QString::number(detector->peakAreaRegionStart));
    ui.tbAdminDetectorPeakAreaRegionEnd->setText(QString::number(detector->peakAreaRegionEnd));
    ui.tbAdminDetectorContinuum->setText(QString::number(detector->continuum));
    ui.cboxAdminDetectorContinuumFunction->setCurrentText(detector->continuumFunction);
    ui.cbAdminDetectorCriticalLevelTest->setChecked(detector->criticalLevelTest);
    ui.cbAdminDetectorUseFixedFWHM->setChecked(detector->useFixedFWHM);
    ui.cbAdminDetectorUseFixedTailParameter->setChecked(detector->useFixedTailParameter);
    ui.cbAdminDetectorFitSinglets->setChecked(detector->fitSinglets);
    ui.cbAdminDetectorDisplayROIs->setChecked(detector->displayROIs);
    ui.cbAdminDetectorRejectZeroAreaPeaks->setChecked(detector->rejectZeroAreaPeaks);
    ui.tbAdminDetectorMaxFWHMsBetweenPeaks->setText(QString::number(detector->maxFWHMsBetweenPeaks));
    ui.tbAdminDetectorMaxFWHMsForLeftLimit->setText(QString::number(detector->maxFWHMsForLeftLimit));
    ui.tbAdminDetectorMaxFWHMsForRightLimit->setText(QString::number(detector->maxFWHMsForRightLimit));
    ui.tbAdminDetectorBackgroundSubtract->setText(detector->backgroundSubtract);
    ui.cboxAdminDetectorEfficiencyCalibrationType->setCurrentText(detector->efficiencyCalibrationType);
    ui.cboxAdminDetectorPresetType1->setCurrentText(detector->presetType1);
    ui.tbAdminDetectorPresetType1Value->setText(QString::number(detector->presetType1Value));
    ui.tbAdminDetectorPresetType1StartChannel->setText(QString::number(detector->presetType1ChannelStart));
    ui.tbAdminDetectorPresetType1EndChannel->setText(QString::number(detector->presetType1ChannelEnd));
    ui.cboxAdminDetectorPresetType2->setCurrentText(detector->presetType2);
    ui.tbAdminDetectorPresetType2Value->setText(QString::number(detector->presetType2Value));
    ui.cboxAdminDetectorPresetType2Unit->setCurrentText(detector->presetType2Unit);
    ui.tbAdminDetectorRandomError->setText(QString::number(detector->randomError));
    ui.tbAdminDetectorSystematicError->setText(QString::number(detector->systematicError));
    ui.tbAdminDetectorNIDLibrary->setText(detector->NIDLibrary);
    ui.tbAdminDetectorNIDConfidenceTreshold->setText(QString::number(detector->NIDConfidenceTreshold));
    ui.tbAdminDetectorMDAConfidenceFactor->setText(QString::number(detector->MDAConfidenceFactor));
    ui.cbAdminDetectorPerformMDATest->setChecked(detector->performMDATest);
    ui.cbAdminDetectorInhibitATDCorrection->setChecked(detector->inhibitATDCorrection);
    ui.cbAdminDetectorUseStoredLibrary->setChecked(detector->useStoredLibrary);

    showBeakersForDetector(detector);
}

void Nailab::onBrowseBackgroundSubtract()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select background file"), libraryDirectory, tr("Background files (%1)").arg("*.cnf"));
    if(QFile::exists(filename))
    {
        ui.tbAdminDetectorBackgroundSubtract->setText(QDir::toNativeSeparators(filename));
    }
}

void Nailab::onBrowseTemplateName()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select template file"), libraryDirectory, tr("Template files (%1)").arg("*.tpl"));
    if(QFile::exists(filename))
    {
        ui.tbAdminGeneralTemplateName->setText(QDir::toNativeSeparators(filename));
    }
}

void Nailab::onBrowseNIDLibrary()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select library file"), libraryDirectory, tr("Library files (%1)").arg("*.nlb"));
    if(QFile::exists(filename))
    {
        ui.tbAdminDetectorNIDLibrary->setText(QDir::toNativeSeparators(filename));
    }
}

void Nailab::onBrowseGenieFolder()
{
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Select Genie folder"));
    ui.tbAdminGeneralGenieFolder->setText(QDir::toNativeSeparators(dirname + "/"));
}

void Nailab::onBrowseNAIImport()
{
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Select NAI import folder"));
    ui.tbAdminGeneralNAIImport->setText(QDir::toNativeSeparators(dirname + "/"));
}

void Nailab::onBrowseRPTExport()
{
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Select RPT export folder"));
    ui.tbAdminGeneralRPTExport->setText(QDir::toNativeSeparators(dirname + "/"));
}

void Nailab::onInputSampleAccepted()
{
    if(!validateSampleInput())
        return;

    SampleInput sampleInput;

    storeSampleInput(sampleInput);

    if(!startJob(sampleInput))
        return;
}

void Nailab::onAddDetectorBeaker()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    QStringList beakerlist;
    for(int j=0; j<beakers.count(); j++)
        beakerlist.append(beakers[j].name);

    Detector* detector = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());

    foreach(QString key, detector->beakers.keys())
    {
        if(beakerlist.contains(key))
            beakerlist.removeOne(key);
    }

    if(beakerlist.count() > 0)
    {
        dlgNewDetectorBeaker->setDetector(detector->name);
        dlgNewDetectorBeaker->setBeakers(beakerlist);
        dlgNewDetectorBeaker->setCalFilePath(libraryDirectory);
        dlgNewDetectorBeaker->exec();
    }
}

void Nailab::onDeleteDetectorBeaker()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    int row = ui.twAdminDetectorBeaker->currentRow();
    if(row < 0)
        return;

    Detector* detector = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());

    QLabel *lbl = dynamic_cast<QLabel*>(ui.twAdminDetectorBeaker->cellWidget(row, 0));
    QString doe = lbl->text();

    if(detector->beakers.contains(doe))
        detector->beakers.remove(doe);    

    writeDetectorXml(envDetectorFile, detectors);
    showBeakersForDetector(detector);
}

void Nailab::onEditDetectorBeaker()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    int row = ui.twAdminDetectorBeaker->currentRow();
    if(row < 0)
        return;

    Detector* detector = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());

    QLabel *lbl = dynamic_cast<QLabel*>(ui.twAdminDetectorBeaker->cellWidget(row, 0));
    QString beakerName = lbl->text();

    dlgEditDetectorBeaker->setDetector(detector->name);
    dlgEditDetectorBeaker->setBeaker(beakerName);
    dlgEditDetectorBeaker->setCalFilePath(libraryDirectory);
    dlgEditDetectorBeaker->exec();
}

void Nailab::onSampleBeakerChanged(QString beaker)
{
    QString detectorName = ui.lblInputSampleDetector->text();
    QString filter = detectorName + beaker + "*.cal";
    QStringList filters;
    filters.append(filter);
    QDir dir(libraryDirectory);
    QStringList calfiles = dir.entryList(filters, QDir::Files);
    ui.cboxInputSampleCalFiles->clear();
    for(int i=0; i<calfiles.count(); i++)
        ui.cboxInputSampleCalFiles->addItem(libraryDirectory + calfiles[i]);
    Detector* det = getDetectorByName(detectorName);
    ui.cboxInputSampleCalFiles->setCurrentText(det->beakers[beaker]);
}

void Nailab::onShowJob()
{
    QModelIndex idx = ui.lvFinishedJobs->selectionModel()->currentIndex();
    QString doneFile = modelFinishedJobs->filePath(idx);
    QString rptFile = tempDirectory + QFileInfo(doneFile).completeBaseName() + ".RPT";
    if(QFile::exists(rptFile))
        QProcess::startDetached("notepad.exe", QStringList() << rptFile);
}

void Nailab::onPrintJob()
{
    QModelIndex idx = ui.lvFinishedJobs->selectionModel()->currentIndex();
    QString doneFile = modelFinishedJobs->filePath(idx);
    QString detName = QFileInfo(doneFile).completeBaseName();
    QString baseFilename = tempDirectory + detName;

    QFile jobfile(baseFilename + ".PNT");
    if(!jobfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::information(this, tr("Error"), tr("Unable to open print file"));
        return;
    }
    QTextStream stream(&jobfile);

    startJobCommand(stream, "report");
    addJobParam(stream, "det:", detName);
    addJobParamQuoted(stream, "/template=", settings.templateName);
    addJobParamSingle(stream, "/newfile");
    addJobParamSingle(stream, "/firstpg");
    addJobParamSingle(stream, "/newpg");
    addJobParamQuoted(stream, "/outfile=", baseFilename + ".RPT"); // FIXME
    addJobParamQuoted(stream, "/section=", "");
    addJobParam(stream, "/EM=", QString::number(settings.errorMultiplier));
    addJobParamSingle(stream, "/PRINT");
    endJobCommand(stream);

    startJobCommand(stream, "dataplot");
    addJobParam(stream, "det:", detName);
    addJobParam(stream, "/scale=", "log");
    addJobParamSingle(stream, "/enhplot");
    endJobCommand(stream);

    jobfile.close();

    QtConcurrent::run(runJob, baseFilename + ".PNT");
}

void Nailab::onStoreJob()
{
    QModelIndex idx = ui.lvFinishedJobs->selectionModel()->currentIndex();
    QString doneFile = modelFinishedJobs->filePath(idx);
    QString detName = QFileInfo(doneFile).completeBaseName();
    QString baseFilename = tempDirectory + detName;

    Detector* det = getDetectorByName(detName);
    if(!updateDetectorSpectrumCounter(envDetectorFile, det))
        return; // FIXME
    QDate date = QDate::currentDate();
    int iyear = date.year() % 1000;
    QString fname = QString("%1%2%3").arg(detName).arg(iyear, 2, 10, QChar('0')).arg(det->spectrumCounter, 4, 10, QChar('0'));

    QString reportFilename = baseFilename + ".RPT";
    QString batchFilename = baseFilename + ".BAT";
    QString outFilename = baseFilename + ".OUT";
    QString errFilename = baseFilename + ".ERR";
    QString specFilename = baseFilename + ".CNF";
    QString doneFilename = baseFilename + ".DONE";
    QString printFilename = baseFilename + ".PNT";

    QString currPath = archiveDirectory + QString::number(date.year()) + QDir::separator() + detName + QDir::separator();

    if(!QDir(archiveDirectory + QString::number(date.year())).exists())
        QDir().mkdir(archiveDirectory + QString::number(date.year()));

    if(!QDir(currPath).exists())
        QDir().mkdir(currPath);

    if(QFile::exists(reportFilename))
        QFile::rename(reportFilename, (currPath + fname + ".RPT").toUpper());
    if(QFile::exists(batchFilename))
        QFile::rename(batchFilename, (currPath + fname + ".BAT").toUpper());
    if(QFile::exists(outFilename))
        QFile::rename(outFilename, (currPath + fname + ".OUT").toUpper());
    if(QFile::exists(errFilename))
        QFile::rename(errFilename, (currPath + fname + ".ERR").toUpper());
    if(QFile::exists(specFilename))
        QFile::rename(specFilename, (currPath + fname + ".CNF").toUpper());
    if(QFile::exists(doneFilename))
        QFile::remove(doneFilename);
    if(QFile::exists(printFilename))
        QFile::remove(printFilename);
}

void Nailab::onRejectJob()
{
    QModelIndex idx = ui.lvFinishedJobs->selectionModel()->currentIndex();
    QString doneFile = modelFinishedJobs->filePath(idx);
    QString detName = QFileInfo(doneFile).completeBaseName();
    QString baseFilename = tempDirectory + detName;

    QString reportFilename = baseFilename + ".RPT";
    QString batchFilename = baseFilename + ".BAT";
    QString outFilename = baseFilename + ".OUT";
    QString errFilename = baseFilename + ".ERR";
    QString specFilename = baseFilename + ".CNF";
    QString doneFilename = baseFilename + ".DONE";
    QString printFilename = baseFilename + ".PNT";

    if(QFile::exists(reportFilename))
        QFile::remove(reportFilename);
    if(QFile::exists(batchFilename))
        QFile::remove(batchFilename);
    if(QFile::exists(outFilename))
        QFile::remove(outFilename);
    if(QFile::exists(errFilename))
        QFile::remove(errFilename);
    if(QFile::exists(specFilename))
        QFile::remove(specFilename);
    if(QFile::exists(doneFilename))
        QFile::remove(doneFilename);
    if(QFile::exists(printFilename))
        QFile::remove(printFilename);
}
