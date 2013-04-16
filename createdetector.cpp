#include "createdetector.h"
#include "ui_createdetector.h"

CreateDetector::CreateDetector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDetector)
{
    ui->setupUi(this);

    QStringList items;
    items << "" << tr("LINEAR") << tr("STEP");
    ui->cboxContinuumFunction->addItems(items);
    items.clear();
    items << "" << tr("LINEAR") << tr("DUAL") << tr("EMP") << tr("INTERP");
    ui->cboxEfficiencyCalibrationType->addItems(items);
    items.clear();
    items << "" << tr("NONE") << tr("AREA") << tr("INTEGRAL") << tr("COUNT") << tr("REALTIME") << tr("LIVETIME");
    ui->cboxPresetType->addItems(items);
}

CreateDetector::~CreateDetector()
{
    delete ui;
}

QString CreateDetector::name() const
{
    return ui->tbName->text();
}

void CreateDetector::setName(QString name)
{
    ui->tbName->setText(name);
}

bool CreateDetector::enabled() const
{
    return ui->cbEnabled->isChecked();
}

int CreateDetector::searchRegion() const
{
    return ui->tbSearchRegion->text().toInt();
}

double CreateDetector::significanceTreshold() const
{
    return ui->tbSignificanceTreshold->text().toDouble();
}

double CreateDetector::tolerance() const
{
    return ui->tbTolerance->text().toDouble();
}

int CreateDetector::peakAreaRegion() const
{
    return ui->tbPeakAreaRegion->text().toInt();
}

double CreateDetector::continuum() const
{
    return ui->tbContinuum->text().toDouble();
}

QString CreateDetector::continuumFunction() const
{
    return ui->cboxContinuumFunction->currentText();
}

bool CreateDetector::criticalLevelTest() const
{
    return ui->cbCriticalLevelTest->isChecked();
}

bool CreateDetector::useFixedFWHM() const
{
    return ui->cbUseFixedFWHM->isChecked();
}

bool CreateDetector::useFixedTailParameter() const
{
    return ui->cbUseFixedTailParameter->isChecked();
}

bool CreateDetector::fitSinglets() const
{
    return ui->cbFixSinglets->isChecked();
}

bool CreateDetector::displayROIs() const
{
    return ui->cbDisplayROIs->isChecked();
}

bool CreateDetector::rejectZeroAreaPeaks() const
{
    return ui->cbRejectZeroAreaPeaks->isChecked();
}

double CreateDetector::maxFWHMsBetweenPeaks() const
{
    return ui->tbMaxFWHMsBetweenPeaks->text().toDouble();
}

double CreateDetector::maxFWHMsForLeftLimit() const
{
    return ui->tbMaxFWHMsForLeftLimit->text().toDouble();
}

double CreateDetector::maxFWHMsForRightLimit() const
{
    return ui->tbMaxFWHMsForRightLimit->text().toDouble();
}

QString CreateDetector::backgroundSubtract() const
{
    return ui->tbBackgroundSubtract->text();
}

QString CreateDetector::efficiencyCalibrationType() const
{
    return ui->cboxEfficiencyCalibrationType->currentText();
}

QString CreateDetector::presetType() const
{
    return ui->cboxPresetType->currentText();
}

double CreateDetector::areaPreset() const
{
    return ui->tbAreaPreset->text().toDouble();
}

int CreateDetector::integralPreset() const
{
    return ui->tbIntegralPreset->text().toInt();
}

int CreateDetector::countPreset() const
{
    return ui->tbCountPreset->text().toInt();
}

int CreateDetector::realTime() const
{
    return ui->tbRealtime->text().toInt();
}

int CreateDetector::liveTime() const
{
    return ui->tbLivetime->text().toInt();
}
