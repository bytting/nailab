#ifndef NAILAB_H
#define NAILAB_H

#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QList>
#include <QString>
#include <QActionGroup>
#include <QStandardItemModel>
#include "ui_nailab.h"
#include "settings.h"
#include "createbeaker.h"
#include "createdetector.h"
#include "createdetectorbeaker.h"
#include "beaker.h"
#include "detector.h"

#define NAILAB_ENVIRONMENT_VARIABLE "NAIROOT"

struct SampleInput;

class Nailab : public QMainWindow
{
    Q_OBJECT

public:

    Nailab(QWidget *parent = 0);
    ~Nailab();

    bool Initialize();

private:

    Ui::MainWindow ui;    
    CreateBeaker *dlgNewBeaker;
    CreateDetector *dlgNewDetector;    
    createdetectorbeaker *dlgNewDetectorBeaker;

    QString username;
    QDir envRootDirectory;
    QDir envConfigurationDirectory, envArchiveDirectory, envTempDirectory, envLibraryDirectory;
    QFile envSettingsFile, envBeakerFile, envDetectorFile, envQuantityUnitFile;

    QMap<QWidget*, QActionGroup*> toolGroups;
    Settings settings;
    QList<Beaker> beakers;
    QList<Detector> detectors;
    QList<QString> detectorNames;    
    QListWidgetItem *listItemJobs, *listItemDetectors, *listItemArchive;    

    bool setupEnvironment();
    void setupDialogs();
    bool setupMCA();    
    void configureWidgets();
    void updateSettings();
    void updateBeakerViews();
    void updateDetectorViews();

    void disableListWidgetItem(QListWidgetItem *item);
    Detector* getDetectorByName(const QString& name);

    void showBeakersForDetector(Detector *detector);

    bool validateSampleInput();
    void storeSampleInput(SampleInput& sampleInput);
    bool startJob(SampleInput& sampleInput);

private slots:

    void onQuit();
    void onBack();
    void onAdmin();
    void onMenuSelect(QListWidgetItem* item);
    void onDetectorSelect(QListWidgetItem* item);

    void onPagesChanged(int index);
    void onTabsAdminChanged(int index);

    void onAdminGeneralAccepted();    

    void onNewBeaker();
    void onNewBeakerAccepted();
    void onAdminBeakersAccepted();    

    void onNewDetector();
    void onNewDetectorAccepted();
    void onNewDetectorBeakerAccepted();
    void onAdminDetectorsAccepted();    

    void onLvAdminBeakersCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onLvAdminDetectorsCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void onBrowseBackgroundSubtract();
    void onBrowseTemplateName();
    void onBrowseNIDLibrary();
    void onBrowseGenieFolder();

    void onInputSampleAccepted();
    void onAddDetectorBeaker();
    void onDeleteDetectorBeaker();
    void onDefaultDetectorBeaker();
};

#endif // NAILAB_H
