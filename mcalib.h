#ifndef MCALIB_H
#define MCALIB_H

#include <QString>

namespace mca
{

enum StatusCode {
    SUCCESS,
    ERROR_OPEN,
    ERROR_QUERY
};

StatusCode initializeVDM();
void closeVDM();

StatusCode openMCA(const QString& detector);
void closeMCA();

StatusCode isBusy(const QString& detector, bool &status);
StatusCode maxChannels(const QString& detector, int &channels);
StatusCode hasHighVoltage(const QString& detector, bool &status);

} // namespace mca

#endif // MCALIB_H
