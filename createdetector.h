#ifndef CREATEDETECTOR_H
#define CREATEDETECTOR_H

#include <QDialog>

namespace Ui {
class CreateDetector;
}

class CreateDetector : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDetector(QWidget *parent = 0);
    ~CreateDetector();

    QString name() const;
    void setName(QString name);
    bool enabled() const;
    int searchRegion() const;
    double significanceTreshold() const;
    double tolerance() const;
    int peakAreaRegion() const;
    double continuum() const;
    QString continuumFunction() const;
    bool criticalLevelTest() const;
    bool useFixedFWHM() const;
    bool useFixedTailParameter() const;
    bool fitSinglets() const;
    bool displayROIs() const;
    bool rejectZeroAreaPeaks() const;
    double maxFWHMsBetweenPeaks() const;
    double maxFWHMsForLeftLimit() const;
    double maxFWHMsForRightLimit() const;
    QString backgroundSubtract() const;
    QString efficiencyCalibrationType() const;    
    QString presetType1() const;
    double presetType1Value() const;
    QString presetType2() const;
    double presetType2Value() const;
    int spectrumCounter() const;

private:
    Ui::CreateDetector *ui;
};

#endif // CREATEDETECTOR_H
