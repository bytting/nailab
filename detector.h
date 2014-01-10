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
    int searchRegionStart;
    int searchRegionEnd;
    double significanceTreshold;
    double tolerance;
    int peakAreaRegionStart;
    int peakAreaRegionEnd;
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
    int presetType1ChannelStart;
    int presetType1ChannelEnd;
    double presetType2Value;
    QString presetType2Unit;
    double randomError;
    double systematicError;
    int spectrumCounter;
    QMap<QString, QString> beakers;
    QString NIDLibrary;
    double NIDConfidenceTreshold;
    double MDAConfidenceFactor;
    bool performMDATest;
    bool inhibitATDCorrection;
    bool useStoredLibrary;
};

#endif // DETECTOR_H
