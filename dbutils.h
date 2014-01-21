#ifndef DBUTILS_H
#define DBUTILS_H

#include <QList>

class QFile;
struct Settings;
struct Beaker;
struct Detector;

bool readSettingsXml(QFile &file, Settings& settings);
bool writeSettingsXml(QFile &file, const Settings& settings);

bool readBeakerXml(QFile &file, QList<Beaker>& beakers);
bool writeBeakerXml(QFile &file, const QList<Beaker>& beakers);

bool readDetectorXml(QFile &file, QList<Detector>& detectors);
bool writeDetectorXml(QFile &file, const QList<Detector>& detectors);
bool updateDetectorSpectrumCounter(QFile& file, Detector* detector);

bool readQuantityUnitsXml(QFile &file, QStringList& units);

#endif // DBUTILS_H
