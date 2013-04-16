#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

struct Settings
{
    QString genieFolder;    
    QString NIDLibrary;
    double NIDConfidenceTreshold;
    double NIDConfidenceFactor;
    bool performMDATest;
    bool inhibitATDCorrection;
    bool useStoredLibrary;
    QString templateName;
    QString sectionName;
    double errorMultiplier;
};

#endif // SETTINGS_H
