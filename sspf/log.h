#pragma once

#include "threadsafequeue.h"

class LogFile
{
public:
    LogFile();
    ~LogFile();
    void Report(char const *msg, ...);
    void ReportNow(char const *msg);
    void Flush();

private:
    char                    m_fileName[MAX_PATH];
    threadSafeQueue<char *> m_errorMessages;
};

inline LogFile logFile;
