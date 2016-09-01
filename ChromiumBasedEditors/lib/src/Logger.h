#ifndef LOGGER_PRIVATE_H
#define LOGGER_PRIVATE_H

#include <string>
#include "../../../../core/DesktopEditor/graphics/Timer.h"

#if 0
class CLoggerSpeed
{
public:
    DWORD m_dwTime;

public:
    CLoggerSpeed()
    {
        m_dwTime = NSTimers::GetTickCount();
    }
    void Lap(const std::string& details)
    {
        DWORD dwCur = NSTimers::GetTickCount();
        FILE* f = fopen("D:\\doctrenderer.speed", "a+");
        std::string sTmp = details + ": %d\n";
        fprintf(f, sTmp.c_str(), (int)(dwCur - m_dwTime));
        fclose(f);
        m_dwTime = dwCur;
    }
    static void Log(const std::wstring& details)
    {
        FILE* f = fopen("D:\\core.log", "a+");
        std::string detailsA = U_TO_UTF8(details);
        detailsA += "\n";
        fprintf(f, detailsA.c_str());
        fclose(f);
    }
    static void Log(const std::string& details)
    {
        FILE* f = fopen("D:\\core.log", "a+");
        std::string detailsA = details;
        detailsA += "\n";
        fprintf(f, detailsA.c_str());
        fclose(f);
    }
};

#define LOGGER_SPEED_START CLoggerSpeed __logger_speed;
#define LOGGER_SPEED_LAP(__logger_param) __logger_speed.Lap(#__logger_param);
#define LOGGER_STRING(__logger_param) CLoggerSpeed::Log(#__logger_param);
#define LOGGER_STRING2(__logger_param) CLoggerSpeed::Log(__logger_param);
#else
#define LOGGER_SPEED_START
#define LOGGER_SPEED_LAP(__logger_param)
#define LOGGER_STRING(__logger_param)
#define LOGGER_STRING2(__logger_param)
#endif

#endif // LOGGER_PRIVATE_H
