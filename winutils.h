#ifndef WINUTILS_H
#define WINUTILS_H

#include <QList>
#include <QString>

bool readGenieDetectorConfig(QList<QString>& mcaList);
bool getWindowsUsername(QString& username);

#endif // WINUTILS_H
