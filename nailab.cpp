#include "nailab.h"
#include "dbutils.h"
#include "winutils.h"
#include "mcalib.h"
#include <qglobal.h>
#include <QStringList>
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>
#include <QListWidgetItem>

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
    return true;
}

void Nailab::setupDialogs()
{
    dlgNewBeaker = new CreateBeaker(this);
    connect(dlgNewBeaker, SIGNAL(accepted()), this, SLOT(onNewBeakerAccepted()));

    dlgNewDetector = new CreateDetector(this);
    connect(dlgNewDetector, SIGNAL(accepted()), this, SLOT(onNewDetectorAccepted()));        
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
    items << "" << tr("NONE") << tr("AREA") << tr("INTEGRAL") << tr("COUNT") << tr("REALTIME") << tr("LIVETIME");
    ui.cboxAdminDetectorPresetType->addItems(items);
    ui.cboxInputSamplePresetType->addItems(items);

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
    ui.dtInputSampleDepositSampleDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradBeginDate->setDate(QDate::currentDate());
    ui.dtInputSampleIrradSampleDate->setDate(QDate::currentDate());
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

const Detector* Nailab::getDetectorByName(const QString& name)
{
    foreach(const Detector &d, detectors)
    {
        if(d.name == name)
            return &d;
    }
    return NULL;
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
            return; // TODO: report error

        ui.tbInputSampleRandomError->setText("0");
        ui.tbInputSampleSystematicError->setText("0");
        ui.cboxInputSamplePresetType->setCurrentText(det->presetType);
        ui.tbInputSampleIntegralPreset->setText(QString::number(det->integralPreset));
        ui.tbInputSampleAreaPreset->setText(QString::number(det->areaPreset));
        ui.tbInputSampleCountPreset->setText(QString::number(det->countPreset));
        ui.tbInputSampleRealtime->setText(QString::number(det->realTime));
        ui.tbInputSampleLivetime->setText(QString::number(det->liveTime));
        // TODO: Fill in defaults
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
    // TODO: Validate input...
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
    detector.presetType = dlgNewDetector->presetType();
    detector.areaPreset = dlgNewDetector->areaPreset();
    detector.integralPreset = dlgNewDetector->integralPreset();
    detector.countPreset = dlgNewDetector->countPreset();
    detector.realTime = dlgNewDetector->realTime();
    detector.liveTime = dlgNewDetector->liveTime();
    detector.spectrumCounter = 0;

    detectors.push_back(detector);
    writeDetectorXml(envDetectorFile, detectors);
    updateDetectorViews();

    // TODO: Get max channels
    /*int channels;
    if(mca::maxChannels(detector.name, channels) == mca::SUCCESS)
        detector.maxChannels = channels;
    else detector.maxChannels = 1024; */
}

void Nailab::onAdminDetectorsAccepted()
{
    if(ui.lvAdminDetectors->selectedItems().count() < 1)
        return;

    QString detectorName = ui.lvAdminDetectors->selectedItems()[0]->text();
    for(int i=0; i<detectors.count(); i++)
    {
        if(detectors[i].name == detectorName)
        {
            detectors[i].inUse = ui.cbAdminDetectorInUse->isChecked();
            detectors[i].searchRegion = ui.tbAdminDetectorSearchRegion->text().toInt();
            detectors[i].significanceTreshold = ui.tbAdminDetectorSignificanceTreshold->text().toDouble();
            detectors[i].tolerance = ui.tbAdminDetectorTolerance->text().toDouble();
            detectors[i].peakAreaRegion = ui.tbAdminDetectorPeakAreaRegion->text().toInt();
            detectors[i].continuum = ui.tbAdminDetectorContinuum->text().toDouble();
            detectors[i].continuumFunction = ui.cboxAdminDetectorContinuumFunction->currentText();
            detectors[i].criticalLevelTest = ui.cbAdminDetectorCriticalLevelTest->isChecked();
            detectors[i].useFixedFWHM = ui.cbAdminDetectorUseFixedFWHM->isChecked();
            detectors[i].useFixedTailParameter = ui.cbAdminDetectorUseFixedTailParameter->isChecked();
            detectors[i].fitSinglets = ui.cbAdminDetectorFitSinglets->isChecked();
            detectors[i].displayROIs = ui.cbAdminDetectorDisplayROIs->isChecked();
            detectors[i].rejectZeroAreaPeaks = ui.cbAdminDetectorRejectZeroAreaPeaks->isChecked();
            detectors[i].maxFWHMsBetweenPeaks = ui.tbAdminDetectorMaxFWHMsBetweenPeaks->text().toDouble();
            detectors[i].maxFWHMsForLeftLimit = ui.tbAdminDetectorMaxFWHMsForLeftLimit->text().toDouble();
            detectors[i].maxFWHMsForRightLimit = ui.tbAdminDetectorMaxFWHMsForRightLimit->text().toDouble();
            detectors[i].backgroundSubtract = ui.tbAdminDetectorBackgroundSubtract->text();
            detectors[i].efficiencyCalibrationType = ui.cboxAdminDetectorEfficiencyCalibrationType->currentText();
            detectors[i].presetType = ui.cboxAdminDetectorPresetType->currentText();
            detectors[i].areaPreset = ui.tbAdminDetectorAreaPreset->text().toDouble();
            detectors[i].integralPreset = ui.tbAdminDetectorIntegralPreset->text().toInt();
            detectors[i].countPreset = ui.tbAdminDetectorCountPreset->text().toInt();
            detectors[i].realTime = ui.tbAdminDetectorRealtime->text().toInt();
            detectors[i].liveTime = ui.tbAdminDetectorLivetime->text().toInt();
            break;
        }
    }
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

    QString detectorName = current->text();
    ui.lblAdminDetectorDetector->setText(detectorName);
    foreach(const Detector &detector, detectors)
    {
        if(detector.name == detectorName)
        {
            ui.cbAdminDetectorInUse->setChecked(detector.inUse);
            ui.tbAdminDetectorSearchRegion->setText(QString::number(detector.searchRegion));
            ui.tbAdminDetectorSignificanceTreshold->setText(QString::number(detector.significanceTreshold));
            ui.tbAdminDetectorTolerance->setText(QString::number(detector.tolerance));
            ui.tbAdminDetectorPeakAreaRegion->setText(QString::number(detector.peakAreaRegion));
            ui.tbAdminDetectorContinuum->setText(QString::number(detector.continuum));
            ui.cboxAdminDetectorContinuumFunction->setCurrentText(detector.continuumFunction);
            ui.cbAdminDetectorCriticalLevelTest->setChecked(detector.criticalLevelTest);
            ui.cbAdminDetectorUseFixedFWHM->setChecked(detector.useFixedFWHM);
            ui.cbAdminDetectorUseFixedTailParameter->setChecked(detector.useFixedTailParameter);
            ui.cbAdminDetectorFitSinglets->setChecked(detector.fitSinglets);
            ui.cbAdminDetectorDisplayROIs->setChecked(detector.displayROIs);
            ui.cbAdminDetectorRejectZeroAreaPeaks->setChecked(detector.rejectZeroAreaPeaks);
            ui.tbAdminDetectorMaxFWHMsBetweenPeaks->setText(QString::number(detector.maxFWHMsBetweenPeaks));
            ui.tbAdminDetectorMaxFWHMsForLeftLimit->setText(QString::number(detector.maxFWHMsForLeftLimit));
            ui.tbAdminDetectorMaxFWHMsForRightLimit->setText(QString::number(detector.maxFWHMsForRightLimit));
            ui.tbAdminDetectorBackgroundSubtract->setText(detector.backgroundSubtract);
            ui.cboxAdminDetectorEfficiencyCalibrationType->setCurrentText(detector.efficiencyCalibrationType);                       
            ui.cboxAdminDetectorPresetType->setCurrentText(detector.presetType);
            ui.tbAdminDetectorAreaPreset->setText(QString::number(detector.areaPreset));
            ui.tbAdminDetectorIntegralPreset->setText(QString::number(detector.integralPreset));
            ui.tbAdminDetectorCountPreset->setText(QString::number(detector.countPreset));
            ui.tbAdminDetectorRealtime->setText(QString::number(detector.realTime));
            ui.tbAdminDetectorLivetime->setText(QString::number(detector.liveTime));
            break;
        }
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
    // TODO: Validate input
    QFile jobfile(envTempDirectory.path() + "/job01.bat");
    if(!jobfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::information(this, tr("Error"), tr("Unable to open job file"));
        return;
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
    stream << "startmca /INTPRESET=" << ui.tbInputSampleIntegralPreset->text()
           << "," << "0" << "," << "512" << "\n"; // TODO: Get channels from mcalib

    jobfile.close();

    // TODO: Update spectrum counter
}
