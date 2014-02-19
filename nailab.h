#ifndef NAILAB_H
#define NAILAB_H

#include <QtWidgets/QMainWindow>
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QList>
#include <QString>
#include <QActionGroup>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QTimer>
#include "ui_nailab.h"
#include "settings.h"
#include "createbeaker.h"
#include "createdetector.h"
#include "createdetectorbeaker.h"
#include "editdetectorbeaker.h"
#include "beaker.h"
#include "detector.h"
#include "mcalib.h"

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
    VDM* vdm;
    CreateBeaker *dlgNewBeaker;
    CreateDetector *dlgNewDetector;    
    createdetectorbeaker *dlgNewDetectorBeaker;
    editdetectorbeaker *dlgEditDetectorBeaker;

    QTimer *idleTimer;
    QString username;    
    QString rootDirectory, configurationDirectory, archiveDirectory, tempDirectory, libraryDirectory;
    QFile envSettingsFile, envBeakerFile, envDetectorFile, envQuantityUnitFile;

    QMap<QWidget*, QActionGroup*> toolGroups;
    Settings settings;
    QList<Beaker> beakers;
    QList<Detector> detectors;
    QList<QString> detectorNames;    
    QListWidgetItem *listItemJobs, *listItemDetectors, *listItemArchive;

    QFileSystemModel *modelArchive, *modelRunningJobs, *modelFinishedJobs;

    bool bAdminDetectorsEnabled, bAdminBeakersEnabled, bFinishedJobsSelected;

    bool setupEnvironment();
    void setupDialogs();
    bool setupMCA();    
    void configureWidgets();
    void enableControlTree(QObject *parent, bool enable);
    void updateSettings();
    void updateBeakerViews();
    void updateDetectorViews();

    void disableListWidgetItem(QListWidgetItem *item);
    Detector* getDetectorByName(const QString& name);

    void showBeakersForDetector(Detector *detector);

    bool validateSampleInput();
    void storeSampleInput(SampleInput& sampleInput);
    bool startJob(SampleInput& sampleInput);
    void startJobCommand(QTextStream& s, const QString& cmd);
    void endJobCommand(QTextStream& s);
    void addJobParam(QTextStream& s, const QString& p, const QString& v);
    void addJobParamSingle(QTextStream& s, const QString& v);
    void addJobParamQuoted(QTextStream& s, const QString& p, const QString& v);
    bool detectorHasJob(const Detector* det);

private slots:

    void onIdle();
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
    void onEditDetectorBeakerAccepted();
    void onAdminDetectorsAccepted();    

    void onLvAdminBeakersCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onLvAdminDetectorsCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void onBrowseBackgroundSubtract();
    void onBrowseTemplateName();
    void onBrowseNIDLibrary();
    void onBrowseGenieFolder();
    void onBrowseNAIImport();
    void onBrowseRPTExport();

    void onInputSampleAccepted();
    void onAddDetectorBeaker();    
    void onDeleteDetectorBeaker();
    void onEditDetectorBeaker();
    void onSampleBeakerChanged(QString beaker);    

    void onShowJob();
    void onPrintJob();
    void onStoreJob();
    void onRejectJob();
};

#endif // NAILAB_H
