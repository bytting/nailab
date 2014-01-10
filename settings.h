#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

struct Settings
{
    QString genieFolder;        
    QString templateName;
    QString sectionName;
    double errorMultiplier;
    QString NAIImportFolder;
    QString RPTExportFolder;
};

#endif // SETTINGS_H
