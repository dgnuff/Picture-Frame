#pragma once

#include "sspf.h"

namespace Util
{

bool    WinsockOK();

uint32  Clamp(uint32 value, uint32 low, uint32 high);
uint32  Clamp(uint32 value, int32 low, uint32 high);
uint32  Clamp(uint32 value, uint32 low, int32 high);
uint32  Clamp(uint32 value, int32 low, int32 high);
int32   Clamp(int32 value, int32 low, int32 high);
int32   Clamp(int32 value, uint32 low, int32 high);
int32   Clamp(int32 value, int32 low, uint32 high);
int32   Clamp(int32 value, uint32 low, uint32 high);
float   Clamp(float value, float low, float high);

#if 0
explicit uint32 Clamp(uint32 value, uint32 low, uint32 high)
{
    return value < low ? low : value > high ? high : value;
}

explicit uint32 Clamp(uint32 value, int32 low, uint32 high)
{
    if (low < 0)
    {
        low = 0;
    }
    return value < (uint32) low ? (uint32) low : value > high ? high : value;
}

explicit uint32 Clamp(uint32 value, uint32 low, int32 high)
{
    if (high < 0)
    {
        high = 0;
    }
    return value < low ? low : value >(uint32) high ? (uint32) high : value;
}

explicit uint32 Clamp(uint32 value, int32 low, int32 high)
{
    if (low < 0)
    {
        low = 0;
    }
    if (high < 0)
    {
        high = 0;
    }
    return value < (uint32) low ? (uint32) low : value >(uint32) high ? (uint32) high : value;
}

explicit int32  Clamp(int32 value, int32 low, int32 high)
{
    return value < low ? low : value > high ? high : value;
}

explicit int32  Clamp(int32 value, uint32 low, int32 high)
{
    if ((int32) low < 0)
    {
        low = 0;
    }
    return value < (int32) low ? (int32) low : value > high ? high : value;
}

explicit int32  Clamp(int32 value, int32 low, uint32 high)
{
    if ((int32) high < 0)
    {
        high = 0;
    }
    return value < low ? low : value >(int32) high ? (int32) high : value;
}

explicit int32  Clamp(int32 value, uint32 low, uint32 high)
{
    if ((int32) low < 0)
    {
        low = 0;
    }
    if ((int32) high < 0)
    {
        high = 0;
    }
    return value < (int32) low ? (int32) low : value >(int32) high ? (int32) high : value;
}

explicit float  Clamp(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}
#endif

float   Lerp(float v0, float v1, float l);

bool    IsJPG(uint8 *data, uint32 size);
bool    IsPNG(uint8 *data, uint32 size);
bool    IsBMP(uint8 *data, uint32 size);
bool    IsBPG(uint8 *data, uint32 size);
bool    GetJPGDimensions(uint8 *data, uint32 size, int *wp, int *hp);
bool    GetPNGDimensions(uint8 *data, uint32 size, int *wp, int *hp);
bool    GetBMPDimensions(uint8 *data, uint32 size, int *wp, int *hp);
bool    GetBPGDimensions(uint8 *data, uint32 size, int *wp, int *hp);

void    GeneratePermutation(uint32 *data, int32 size);
void    RandomHalf(uint32 *data, int32 size);

//void    SetErrorLog(char const *errorLog);
//void    logFile.Report(char const *fmt, ...);
//void    WriteErrors();
//void    DebugLog(int level, char *message);

INT_PTR Atoi(char const *str);

void    BlankOpt(char const *str);
bool    StartThread();
bool    ShowBlack();
void    SetDelay(int32 delay);
void    SetTimes(int32 startTime, int32 endTime);
int32   GetStartTime();
int32   GetEndTime();
int32   GetCurrentMinute();

void    ParseQueueSize(char const *str);
uint32  GetQueueMin();
uint32  GetQueueMax();

void    SetIgnoreMouse(uint32 delay);
bool    ShouldIgnoreMouse();

void    ScreenOff();
void    ScreenOn();

uint32  Aspect(uint32 scrx, uint32 scry);
uint32  FindAspect(char const *str);
uint32  ParseAspect(std::string const &, std::string &resourceFile);
float   AspectDistance(uint32 aspect1, uint32 aspect2);

bool    MouseMoveHeuristic(bool init);

std::vector<std::string> Explode(std::string const &s, char delim = ' ');

class Fraction
{
public:
    Fraction() : num_(0), denom_(1) {}
    Fraction(int num) : num_(num), denom_(1) {}
    Fraction(int num, int denom) : num_(num), denom_(denom)
    {
        Normalize_();
    }

    bool operator<(Fraction const &rhs) const
    {
        double const l = (double) num_ / (double) denom_;
        double const r = (double) rhs.num_ / (double) rhs.denom_;
        return l < r;
    }

    bool operator==(Fraction const &rhs) const
    {
        return num_ == rhs.num_ && denom_ == rhs.denom_;
    }

    void Get(int &n, int &d) const
    {
        n = num_;
        d = denom_;
    }

private:
    int num_;
    int denom_;

    void Normalize_()
    {
        int sign = 1;
        if (num_ < 0)
        {
            num_ = -num_;
            sign = -1;
        }
        if (denom_ < 0)
        {
            denom_ = -denom_;
            sign = -sign;
        }
        else if (denom_ == 0)
        {
            denom_ = 1;
        }

        unsigned int a = num_;
        unsigned int b = denom_;
        if (a < b)
        {
            std::swap(a, b);
        }
        while (b != 0)
        {
            unsigned int t = b;
            b = a % b;
            a = t;
        }
        num_ = num_ / a * sign;
        denom_ = denom_ / a;
    }
};



#ifdef DEBUG
template<typename T>
class SmartArray
{
public:
    SmartArray()
    {
        m_data = NULL;
        m_size = 0;
    }

    SmartArray(T* data, uint32 size)
    {
        m_data = data;
        m_size = size;
    }

    T &operator[](uint32 index)
    {
        if (index >= m_size)
        {
            DebugBreak();
        }
        return m_data[index];
    }

private:
    T*      m_data;
    uint32  m_size;
};

void    BuildSmartArray(std::vector<SmartArray<uint8>> &data_c, uint8 **data, uint32 w, uint32 h);
#endif

};
