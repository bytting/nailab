#ifndef MCALIB_H
#define MCALIB_H

#include <windows.h>
#include <QString>

class VDM
{
public:

    static VDM* instance();

    void initialize();
    void close();

    bool isBusy(const QString& detector);
    int maxChannels(const QString& detector);
    bool hasHighVoltage(const QString& detector);

private:

    VDM() : mHandle(NULL), mHasOpenMCA(false) {}
    VDM(const VDM&) {}
    VDM& operator = (const VDM&) { return *this; }

    HANDLE mHandle;
    bool mHasOpenMCA;

    void openMCA(const QString& detector);
    void closeMCA();
};

#endif // MCALIB_H
