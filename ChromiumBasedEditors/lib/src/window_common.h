#ifndef ASC_CEF_WINDOW_COMMON_H
#define ASC_CEF_WINDOW_COMMON_H

#include "../include/base.h"

#ifdef WIN32
void SetParentNULL(WindowHandleId handle)
{
    SetParent(handle, NULL);
}
#endif

#if defined(_LINUX) && !defined(_MAC)
void SetParentNULL(WindowHandleId handle)
{
}
#endif

#ifdef _MAC
void SetParentNULL(WindowHandleId handle);
#endif

#endif // ASC_CEF_WINDOW_COMMON_H
