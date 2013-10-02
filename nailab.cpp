#include <QStringList>
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>
#include <QListWidgetItem>
#include <qglobal.h>
#include "nailab.h"
#include "dbutils.h"
#include "winutils.h"
#include "mcalib.h"
#include "sampleinput.h"

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
    return true;
}

bool Nailab::setupEnvironment()
{    
    envRootDirectory.setPath(qgetenv(NAILAB_ENVIRONMENT_VARIABLE).constData());
    if(!envRootDirectory.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("Environment variable not found, or invalid: ") + NAILAB_ENVIRONMENT_VARIABLE);
        return false;
    }

    envConfigurationDirectory.setPath(envRootDirectory.path() + "/CONFIGURATION/");
    envArchiveDirectory.setPath(envRootDirectory.path() + "/ARCHIVE/");
    envTempDirectory.setPath(envRootDirectory.path() + "/TEMP/");
    envLibraryDirectory.setPath(envRootDirectory.path() + "/LIBRARY/");

    if(!envConfigurationDirectory.exists()
        || !envArchiveDirectory.exists()
        || !envTempDirectory.exists()
        || !envLibraryDirectory.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("One or more nailab system directories not found (CONFIGURATION, ARCHIVE, TEMP, LIBRARY)"));
        return false;
    }

    envSettingsFile.setFileName(envConfigurationDirectory.path() + "/settings.xml");
    if(!envSettingsFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envSettingsFile.fileName());
        return false;
    }

    envBeakerFile.setFileName(envConfigurationDirectory.path() + "/beaker.xml");
    if(!envBeakerFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envBeakerFile.fileName());
        return false;
    }

    envDetectorFile.setFileName(envConfigurationDirectory.path() + "/mca.xml");
    if(!envDetectorFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envDetectorFile.fileName());
        return false;
    }

    envQuantityUnitFile.setFileName(envConfigurationDirectory.path() + "/quantity_units.xml");
    if(!envQuantityUnitFile.exists())
    {
        QMessageBox::information(this, tr("Error"), tr("File not found: ") + envQuantityUnitFile.fileName());
        return false;
    }
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

    if(mca::initializeVDM() != mca::SUCCESS)
    {
        QMessageBox::information(this, tr("Error"), tr("Failed to open connection to VDM"));
        return false;
    }
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
    QStringList items;
    items << "" << tr("LINEAR") << tr("STEP");
    ui.cboxAdminDetectorContinuumFunction->addItems(items);
    items.clear();
    items << "" << tr("LINEAR") << tr("DUAL") << tr("EMP") << tr("INTERP");
    ui.cboxAdminDetectorEfficiencyCalibrationType->addItems(items);
    items.clear();
    items << "" << tr("HEADER");
    ui.cboxAdminGeneralSectionName->addItems(items);
    items.clear();
    items << "" << tr("NONE") << tr("AREA") << tr("INTEGRAL") << tr("COUNT");
    ui.cboxAdminDetectorPresetType1->addItems(items);
    ui.cboxInputSamplePresetType1->addItems(items);
    items.clear();
    items << "" << tr("NONE") << tr("REALTIME") << tr("LIVETIME");
    ui.cboxAdminDetectorPresetType2->addItems(items);
    ui.cboxInputSamplePresetType2->addItems(items);

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
    connect(ui.lwMenu, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onMenuSelect(QListWidgetItem*)));

    // Dates
    ui.dtInputSampleNoneSampleDate->setDate(QDate::currentDate());
    ui.dtInputSampleDepositBeginDate->setDate(QDate::currentDate());
    ui.dtInputSampleDepositEndDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradBeginDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradEndDate->setDate(QDate::currentDate());
}

void Nailab::updateSettings()
{
    ui.tbAdminGeneralGenieFolder->setText(settings.genieFolder);    
    ui.tbAdminGeneralNIDLibrary->setText(settings.NIDLibrary);
    ui.tbAdminGeneralNIDConfidenceTreshold->setText(QString::number(settings.NIDConfidenceTreshold));
    ui.tbAdminGeneralNIDConfidenceFactor->setText(QString::number(settings.NIDConfidenceFactor));
    ui.cbAdminGeneralPerformMDATest->setChecked(settings.performMDATest);
    ui.cbAdminGeneralInhibitATDCorrection->setChecked(settings.inhibitATDCorrection);
    ui.cbAdminGeneralUseStoredLibrary->setChecked(settings.useStoredLibrary);
    ui.tbAdminGeneralTemplateName->setText(settings.templateName);
    ui.cboxAdminGeneralSectionName->setCurrentText(settings.sectionName);
    ui.tbAdminGeneralErrorMultiplier->setText(QString::number(settings.errorMultiplier));
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

    bool found, status;
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
        {
            item->setHidden(true);
        }
        else if(!found)
        {
            disableListWidgetItem(item);
        }
        else
        {
            /*if(mca::isBusy(detector.name, status) == mca::SUCCESS) // TODO
            {
                if(status)
                    disableListWidgetItem(item);
            }
            else disableListWidgetItem(item);*/
        }
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
        sampleInput.builduptype = "";
        sampleInput.startTime = ui.dtInputSampleNoneSampleDate->text();
        break;
    }
    // FIXME: Not finished
}

bool Nailab::startJob(SampleInput& sampleInput)
{
    // FIXME: Not finished
    QFile jobfile(envTempDirectory.path() + "/job01.bat");
    if(!jobfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::information(this, tr("Error"), tr("Unable to open job file"));
        return false;
    }
    QTextStream stream(&jobfile);

    // pars
    stream << "pars /PRUSESTRLIB=" << (settings.useStoredLibrary ? "1" : "0") << " /MDACONFID=" << settings.NIDConfidenceFactor << "\n";

    // nid_intf
    stream << "nid_intf /LIBRARY=\"" << settings.NIDLibrary << "\" /CONFID=" << settings.NIDConfidenceTreshold;
    if(settings.performMDATest)
        stream << " /MDA_TEST";
    if(settings.inhibitATDCorrection)
        stream << " /NOACQDECAY";
    stream << "\n";

    // startmca
    stream << "startmca ";

    QString presetType;
    if(ui.cboxInputSamplePresetType1->currentText() == "AREA")
    {
        presetType = "/AREAPRESET=";
    }
    else if(ui.cboxInputSamplePresetType1->currentText() == "INTEGRAL")
    {
        presetType = "/INTPRESET=";
    }
    else if(ui.cboxInputSamplePresetType1->currentText() == "COUNT")
    {
        presetType = "/CNTSPRESET=";
    }

    stream << presetType << ui.tbInputSamplePresetType1->text() << "," << "0" << "," << "512" << " "; // FIXME: Get channels from mcalib

    if(ui.cboxInputSamplePresetType2->currentText() == "REALTIME")
    {
        presetType = "/REALTIME=";
    }
    else if(ui.cboxInputSamplePresetType2->currentText() == "LIVETIME")
    {
        presetType = "/LIVETIME=";
    }

    stream << presetType << ui.tbInputSamplePresetType2->text() << "\n";

    jobfile.close();

    // FIXME: Update spectrum counter
    return true;
}

void Nailab::onQuit()
{
    mca::closeVDM();
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
        ui.pages->setCurrentWidget(ui.pageInput);
        ui.tabsInput->setCurrentWidget(ui.tabInputSample);
        ui.toolsInputSample->setCurrentIndex(0); // Parameters
        ui.tabsInputSampleBuildupType->setCurrentIndex(2); // None
        ui.lblInputSampleDetector->setText(item->text());

        const Detector* det = getDetectorByName(item->text());
        if(!det)
            return; // FIXME: report error

        QString username;
        if(!getWindowsUsername(username))
            return; // FIXME: report error

        ui.tbInputSampleCollector->setText(username);
        ui.tbInputSampleSpecterRef->setText(QString::number(det->spectrumCounter));

        ui.cbInputSampleGeometry->clear();
        foreach(QString key, det->beakers.keys())
            ui.cbInputSampleGeometry->addItem(key);
        ui.cbInputSampleGeometry->setCurrentText(det->defaultBeaker);

        ui.cboxInputSamplePresetType1->setCurrentText(det->presetType1);
        ui.tbInputSamplePresetType1->setText(QString::number(det->presetType1Value));
        ui.cboxInputSamplePresetType2->setCurrentText(det->presetType2);
        ui.tbInputSamplePresetType2->setText(QString::number(det->presetType2Value));
        ui.tbInputSampleRandomError->setText(QString::number(det->randomError));
        ui.tbInputSampleSystematicError->setText(QString::number(det->systematicError));
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
    settings.NIDLibrary = ui.tbAdminGeneralNIDLibrary->text();
    settings.NIDConfidenceTreshold = ui.tbAdminGeneralNIDConfidenceTreshold->text().toDouble();
    settings.NIDConfidenceFactor = ui.tbAdminGeneralNIDConfidenceFactor->text().toDouble();
    settings.performMDATest = ui.cbAdminGeneralPerformMDATest->isChecked();
    settings.inhibitATDCorrection = ui.cbAdminGeneralInhibitATDCorrection->isChecked();
    settings.useStoredLibrary = ui.cbAdminGeneralUseStoredLibrary->isChecked();
    settings.templateName = ui.tbAdminGeneralTemplateName->text();
    settings.sectionName = ui.cboxAdminGeneralSectionName->currentText();
    settings.errorMultiplier = ui.tbAdminGeneralErrorMultiplier->text().toDouble();

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
    detector.searchRegion = dlgNewDetector->searchRegion();
    detector.significanceTreshold = dlgNewDetector->significanceTreshold();
    detector.tolerance = dlgNewDetector->tolerance();
    detector.peakAreaRegion = dlgNewDetector->peakAreaRegion();
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
    detector.presetType2 = dlgNewDetector->presetType2();
    detector.presetType2Value = dlgNewDetector->presetType2Value();
    detector.randomError = dlgNewDetector->randomError();
    detector.systematicError = dlgNewDetector->systematicError();
    detector.spectrumCounter = 0;

    detectors.push_back(detector);
    writeDetectorXml(envDetectorFile, detectors);
    updateDetectorViews();

    // FIXME: Get max channels
    /*int channels;
    if(mca::maxChannels(detector.name, channels) == mca::SUCCESS)
        detector.maxChannels = channels;
    else detector.maxChannels = 1024; */
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

void Nailab::onAdminDetectorsAccepted()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    QString detectorName = ui.lvAdminDetectors->selectedItems()[0]->text();
    Detector* detector = getDetectorByName(detectorName);

    detector->inUse = ui.cbAdminDetectorInUse->isChecked();
    detector->searchRegion = ui.tbAdminDetectorSearchRegion->text().toInt();
    detector->significanceTreshold = ui.tbAdminDetectorSignificanceTreshold->text().toDouble();
    detector->tolerance = ui.tbAdminDetectorTolerance->text().toDouble();
    detector->peakAreaRegion = ui.tbAdminDetectorPeakAreaRegion->text().toInt();
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
    detector->presetType2 = ui.cboxAdminDetectorPresetType2->currentText();
    detector->presetType2Value = ui.tbAdminDetectorPresetType2Value->text().toDouble();
    detector->randomError = ui.tbAdminDetectorRandomError->text().toDouble();
    detector->systematicError = ui.tbAdminDetectorSystematicError->text().toDouble();

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
    ui.lblAdminDetectorDetector->setText(detector->name);

    ui.cbAdminDetectorInUse->setChecked(detector->inUse);
    ui.tbAdminDetectorSearchRegion->setText(QString::number(detector->searchRegion));
    ui.tbAdminDetectorSignificanceTreshold->setText(QString::number(detector->significanceTreshold));
    ui.tbAdminDetectorTolerance->setText(QString::number(detector->tolerance));
    ui.tbAdminDetectorPeakAreaRegion->setText(QString::number(detector->peakAreaRegion));
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
    ui.cboxAdminDetectorPresetType2->setCurrentText(detector->presetType2);
    ui.tbAdminDetectorPresetType2Value->setText(QString::number(detector->presetType2Value));
    ui.tbAdminDetectorRandomError->setText(QString::number(detector->randomError));
    ui.tbAdminDetectorSystematicError->setText(QString::number(detector->systematicError));

    showBeakersForDetector(detector);
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

void Nailab::onBrowseBackgroundSubtract()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select background file"), envLibraryDirectory.path(), tr("Background files (%1)").arg("*.cnf"));
    if(QFile::exists(filename))
    {
        ui.tbAdminDetectorBackgroundSubtract->setText(filename);
    }
}

void Nailab::onBrowseTemplateName()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select template file"), envLibraryDirectory.path(), tr("Template files (%1)").arg("*.tpl"));
    if(QFile::exists(filename))
    {
        ui.tbAdminGeneralTemplateName->setText(filename);
    }
}

void Nailab::onBrowseNIDLibrary()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select library file"), envLibraryDirectory.path(), tr("Library files (%1)").arg("*.nlb"));
    if(QFile::exists(filename))
    {
        ui.tbAdminGeneralNIDLibrary->setText(filename);
    }
}

void Nailab::onBrowseGenieFolder()
{
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Select Genie folder"));
    ui.tbAdminGeneralGenieFolder->setText(dirname);
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
        dlgNewDetectorBeaker->setBeakers(beakerlist);
        dlgNewDetectorBeaker->setCalFilePath(envLibraryDirectory.path());
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

    detector->defaultBeaker = "";

    writeDetectorXml(envDetectorFile, detectors);
    showBeakersForDetector(detector);
}

void Nailab::onDefaultDetectorBeaker()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    int row = ui.twAdminDetectorBeaker->currentRow();
    if(row < 0)
        return;

    Detector* detector = getDetectorByName(ui.lvAdminDetectors->selectedItems()[0]->text());

    QLabel *lbl = dynamic_cast<QLabel*>(ui.twAdminDetectorBeaker->cellWidget(row, 0));
    QString def = lbl->text();

    detector->defaultBeaker = def;

    writeDetectorXml(envDetectorFile, detectors);
}
