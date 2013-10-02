#ifndef DETECTOR_H
#define DETECTOR_H

#include <QMap>
#include <QString>

struct Detector
{
    QString name;
    bool enabled;
    bool inUse;
    int maxChannels;
    int searchRegion;
    double significanceTreshold;
    double tolerance;
    int peakAreaRegion;
    double continuum;
    QString continuumFunction;
    bool criticalLevelTest;
    bool useFixedFWHM;
    bool useFixedTailParameter;
    bool fitSinglets;
    bool displayROIs;
    bool rejectZeroAreaPeaks;
    double maxFWHMsBetweenPeaks;
    double maxFWHMsForLeftLimit;
    double maxFWHMsForRightLimit;
    QString backgroundSubtract;
    QString efficiencyCalibrationType;
    QString presetType1;
    QString presetType2;
    double presetType1Value;
    double presetType2Value;
    double randomError;
    double systematicError;
    int spectrumCounter;
    QMap<QString, QString> beakers;
    QString defaultBeaker;
};

#endif // DETECTOR_H
