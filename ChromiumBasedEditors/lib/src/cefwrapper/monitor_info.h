#ifndef MONITIOR_INFO_COMMON_H
#define MONITIOR_INFO_COMMON_H

#include "./../../include/base.h"

namespace NSMonitor
{
    double GetRawMonitorScale(const unsigned int& xDpi, const unsigned int& yDpi);
    double GetRawMonitorDpi(WindowHandleId handle);
}

#endif
