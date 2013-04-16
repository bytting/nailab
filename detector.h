#ifndef DETECTOR_H
#define DETECTOR_H

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
    QString presetType;
    double areaPreset;
    int integralPreset;
    int countPreset;
    int realTime;
    int liveTime;
    int spectrumCounter;
};

#endif // DETECTOR_H
