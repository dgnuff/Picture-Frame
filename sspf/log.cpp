#include "pch.h"

#include "log.h"
#include "sspf.h"

using std::function;

LogFile::LogFile()
{
    m_fileName[0] = 0;
    char const *cmdLine = GetCommandLine();
    while (*cmdLine)
    {
        char arg[256];
        cmdLine = cmdline(cmdLine, arg, 256 - 1);
        char *fileName = m_fileName;
        function<void(char const *)> lambda = [fileName](char const *str)
        {
            strncpy(fileName, str, MAX_PATH);
            fileName[MAX_PATH - 1] = 0;
        };

        if ((arg[0] & 0xfd) == 0x2d && (arg[1] | 0x20) == 'e')
        {
            cmdLine = ProcessArg(lambda, arg, cmdLine);
        }
    }
}

LogFile::~LogFile()
{
    Flush();
}

void    LogFile::Report(char const *fmt, ...)
{
    char message[1024];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(message, 1024, "%02d/%02d/%02d %02d:%02d:%02d.%03d ", now.wYear % 100, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(&message[22], 1000, fmt, argptr);
    va_end(argptr);
    m_errorMessages.enqueue(strdup(message));
}

void    LogFile::ReportNow(char const *msg)
{
    char message[1024];
    SYSTEMTIME now;
    FILE *fp;
    GetLocalTime(&now);
    snprintf(message, 1024, "%02d/%02d/%02d %02d:%02d:%02d.%03d %.1000s", now.wYear % 100, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, msg);
    if (m_fileName[0] != 0)
    {
        fp = fopen(m_fileName, "a");
        if (fp != NULL)
        {
            fprintf(fp, "%s\n", message);
            fclose(fp);
        }
    }
#ifdef DEBUG
    OutputDebugString(message);
    OutputDebugString("\r\n");
#endif
}

void    LogFile::Flush()
{
    FILE *fp = NULL;
    char *message;

    if (m_fileName[0] != 0)
    {
        fp = fopen(m_fileName, "a");
    }
    while (m_errorMessages.get(message))
    {
        if (message != NULL)
        {
            if (fp != NULL)
            {
                fprintf(fp, "%s\n", message);
            }
#ifdef DEBUG
            OutputDebugString(message);
            OutputDebugString("\r\n");
#endif
            free(message);
        }
    }
    if (fp != NULL)
    {
        fclose(fp);
    }
}
