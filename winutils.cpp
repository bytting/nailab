#include <windows.h>
#include <QFile>
#include <QTextStream>
#include "winutils.h"

bool readGenieDetectorConfig(QList<QString> &mcaList)
{
    DWORD bufCharCount = 1024;
    TCHAR infoBuf[1024];

    if(!GetComputerName(infoBuf, &bufCharCount))
        return false;
    QString compName = QString::fromStdWString(std::wstring(infoBuf));
    QString filename = "C:\\GENIE2K\\MIDFILES\\" + compName + ".WSP";
    if(!QFile::exists(filename))
        return false;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    mcaList.clear();
    QTextStream fin(&file);
    while(!fin.atEnd())
        mcaList.push_back(fin.readLine());
    file.close();
    return true;
}

bool getWindowsUsername(QString& username)
{
    TCHAR uname[256];
    DWORD nUname = sizeof(uname);
    if(!GetUserName(uname, &nUname))
        return false;
    username = QString::fromStdWString(std::wstring(uname));
    return true;
}
