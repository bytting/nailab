#ifndef WINUTILS_H
#define WINUTILS_H

#include <QList>
#include <QString>

bool readGenieDetectorConfig(QList<QString>& mcaList);
bool getWindowsUsername(QString& username);

bool runJob(const QString& cmd);

#endif // WINUTILS_H
