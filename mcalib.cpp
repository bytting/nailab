
#include "mcalib.h"
#include <QString>
#include <QByteArray>
#include <citypes.h>
#include <crackers.h>
#include <spasst.h>
#include <sad.h>
#include <ci_files.h>
#include <campdef.h>
#include <cam_n.h>
#include <utility.h>
#include <sad_nest.h>
#include "exceptions.h"

VDM* VDM::instance()
{
    static VDM vdm;
    return &vdm;
}

void VDM::initialize()
{      
    if(mHandle) // Exit if already initialized
        return;

    // Initialize Genie 2000 environment
    vG2KEnv();

    // Create connection to VDM server
    if(iUtlCreateFileDSC2(&mHandle, 0, 0))
        throw BadException(__FILE__, __FUNCTION__, __LINE__, "Connection to VDM server failed");
}

void VDM::close()
{
    if(mHandle)
        SadDeleteDSC(mHandle);
    mHandle = nullptr;
}

void VDM::openMCA(const QString& detector)
{
    if(mHasOpenMCA)
        closeMCA();

    QByteArray ba = detector.toLocal8Bit();
    char* mca = const_cast<char*>(ba.data());
    //if(SadOpenDataSource(hDSC, mca, CIF_Detector, ACC_Exclusive | ACC_SysWrite | ACC_ReadWrite, TRUE, ""))
    if(SadOpenDataSource(mHandle, mca, CIF_Detector, ACC_ReadOnly, FALSE, ""))
        throw BadException(__FILE__, __FUNCTION__, __LINE__, "Open MCA failed");

    mHasOpenMCA = true;
}

void VDM::closeMCA()
{
    if(mHasOpenMCA)
    {
        SadCloseDataSource(mHandle);
        mHasOpenMCA = false;
    }
}

bool VDM::isBusy(const QString& detector)
{    
    openMCA(detector);

    DSQuery_T stInfo;
    if(SadQueryDataSource(mHandle, DSQ_Status, &stInfo))
    {
        closeMCA();
        throw QueryException(__FILE__, __FUNCTION__, __LINE__, "Query MCA status failed");
    }

    bool status = (stInfo.stDS.fsStatus & DSS_Busy) ? true : false;

    closeMCA();
    return status;
}

int VDM::maxChannels(const QString& detector)
{
    // Get max number of channels for mca
    openMCA(detector);

    DSQuery_T stInfo;
    if(SadQueryDataSource(mHandle, DSQ_ChansPer, &stInfo))
    {
        closeMCA();
        throw QueryException(__FILE__, __FUNCTION__, __LINE__, "Query MCA channels failed");
    }

    int channels = (int)stInfo.ulChannels;

    closeMCA();
    return channels;
}

bool VDM::hasHighVoltage(const QString& detector)
{
    // Get voltage status for mca
    openMCA(detector);

    LONG hvstat = 0L;
    if(SadGetParam(mHandle, CAM_L_HVPSFSTAT, 0, 0, &hvstat, sizeof(LONG)))
    {
        closeMCA();
        throw QueryException(__FILE__, __FUNCTION__, __LINE__, "Query MCA voltage failed");
    }

    bool status = hvstat ? true : false;

    closeMCA();
    return status;
}
