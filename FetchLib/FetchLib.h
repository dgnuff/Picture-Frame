#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <clib.h>

#include "types.h"

#define RESOURCE_LEN    4096

struct Resource
{
};

struct Fetcher
{
    char const *Name()
    {
        return m_name;
    }

    int32 Init()
    {
        return m_init ? m_init() : ERROR;
    }

    Resource *Prepare(char const *name)
    {
        return m_prepare ? m_prepare(name) : NULL;
    }

    int32 Gets(Resource *resource, char *buff, int32 size)
    {
        return m_gets ? m_gets(resource, buff, size) : EOF;
    }

    uint32 Alldata(Resource *resource, uint8 **buff)
    {
        return m_alldata ? m_alldata(resource, buff) : 0;
    }

    uint32 AlldataAcquire(Resource *resource, uint8 **buff)
    {
        return m_alldataAcquire ? m_alldataAcquire(resource, buff) : 0;
    }

    void Finish(Resource *resource)
    {
        if (m_finish)
        {
            m_finish(resource);
        }
    }

    void Shutdown()
    {
        if (m_shutdown)
        {
            m_shutdown();
        }
    }

    char *m_name;
    int32 (*m_init)();
    Resource *(*m_prepare)(char const *name);
    int32 (*m_gets)(Resource *resource, char *buff, int32 size);
    uint32(*m_alldata)(Resource *resource, uint8 **buff);
    uint32(*m_alldataAcquire)(Resource *resource, uint8 **buff);
    void (*m_finish)(Resource *resource);
    void (*m_shutdown)();
};

Fetcher *fileFactory();
//Fetcher *httpFactory();
//Fetcher *httpsFactory();
