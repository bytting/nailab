#include "mcalib.h"

#include <windows.h>
#include <citypes.h>
#include <crackers.h>
#include <spasst.h>
#include <sad.h>
#include <ci_files.h>
#include <campdef.h>
#include <cam_n.h>
#include <utility.h>
#include <sad_nest.h>

#include <QString>
#include <QByteArray>

namespace mca
{

static HMEM hDSC = NULL;
static bool hasOpenMCA = false;

StatusCode initializeVDM()
{      
    if(hDSC) // Exit if already initialized
        return SUCCESS;

    // Initialize Genie 2000 environment
    vG2KEnv();

    // Create connection to VDM server
    if(iUtlCreateFileDSC2(&hDSC, 0, 0))
        return ERROR_OPEN;

    return SUCCESS;
}

void closeVDM()
{
    if(hDSC)
        SadDeleteDSC(hDSC);
    hDSC = NULL;
}

StatusCode openMCA(const QString& detector)
{
    if(hasOpenMCA)
        return ERROR_OPEN;

    QByteArray ba = detector.toLocal8Bit();
    char* mca = const_cast<char*>(ba.data());
    //if(SadOpenDataSource(hDSC, mca, CIF_Detector, ACC_Exclusive | ACC_SysWrite | ACC_ReadWrite, TRUE, ""))
    if(SadOpenDataSource(hDSC, mca, CIF_Detector, ACC_ReadOnly, FALSE, ""))
        return ERROR_OPEN;

    hasOpenMCA = true;
    return SUCCESS;
}

void closeMCA()
{
    if(hasOpenMCA)
    {
        SadCloseDataSource(hDSC);
        hasOpenMCA = false;
    }
}

StatusCode isBusy(const QString& detector, bool &status)
{
    // Check if mca is busy or not
    StatusCode statusCode = SUCCESS;
    if((statusCode = openMCA(detector)) != SUCCESS)
        return statusCode;

    DSQuery_T stInfo;
    if(SadQueryDataSource(hDSC, DSQ_Status, &stInfo))
    {
        closeMCA();
        return ERROR_QUERY;
    }

    status = (stInfo.stDS.fsStatus & DSS_Busy) ? true : false;
    closeMCA();
    return SUCCESS;
}

StatusCode maxChannels(const QString& detector, int &channels)
{
    // Get max number of channels for mca
    StatusCode statusCode = SUCCESS;
    if((statusCode = openMCA(detector)) != SUCCESS)
        return statusCode;

    DSQuery_T stInfo;
    if(SadQueryDataSource(hDSC, DSQ_ChansPer, &stInfo))
    {
        closeMCA();
        return ERROR_QUERY;
    }

    channels = (int)stInfo.ulChannels;
    closeMCA();
    return SUCCESS;
}

StatusCode hasHighVoltage(const QString& detector, bool &status)
{
    // Get voltage status for mca
    StatusCode statusCode = SUCCESS;
    if((statusCode = openMCA(detector)) != SUCCESS)
        return statusCode;

    LONG hvstat = 0L;
    if(SadGetParam(hDSC, CAM_L_HVPSFSTAT, 0, 0, &hvstat, sizeof(LONG)))
    {
        closeMCA();
        return ERROR_QUERY;
    }

    status = hvstat ? true : false;
    closeMCA();
    return SUCCESS;
}

} // namespace mca
