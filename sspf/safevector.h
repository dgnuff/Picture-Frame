#pragma once

#include <exception>
#include <malloc.h>
#include "types.h"
#include "log.h"

template <typename T>
class safevector
{
public:
    void push_back(T v)
    {
        if (avail <= used)
        {
            avail += 256;
            data = static_cast<T *>(realloc(data, avail * sizeof(T)));
            if (data == nullptr)
            {
                logFile.ReportNow("safevector out of memory");
                throw std::runtime_error("safevector out of memory");
            }
        }
        data[used++] = v;
    }

    T &operator[](uint32 i)
    {
        if (i >= used)
        {
            logFile.ReportNow("safevector array index out of bounds");
            throw std::runtime_error("safevector array index out of bounds");
        }
        return data[i];
    }

    size_t size()
    {
        return used;
    }

    void clear()
    {
        free(data);
        data = nullptr;
        avail = 0;
        used = 0;
    }

private:
    inline static T *data = nullptr;
    inline static uint32 avail = 0;
    inline static uint32 used = 0;
};
