#include "pch.h"

#include "sspf.h"
#include "threadsafequeue.h"
#include "util.h"

using std::atomic;
using std::max;
using std::string;
using std::stringstream;
using std::vector;

namespace Util
{

int32   n_startBlankTime = 0;
int32   n_endBlankTime = 0;
uint16  n_blankPort = 0;
uint16  n_blankDuration = 0;
int32   n_blankDelay = 0;
int32   n_startTime = 0;
uint32  n_queueMin = 0;
uint32  n_queueMax = 0;

static atomic<uint32>   n_screenOffCount;
static atomic<uint32>   n_ignoreMouse;

class WSAwrapper
{
public:
    WSAwrapper()
    {
        int32 result;
        WSADATA wsaData;

        result = WSAStartup(0x0002, &wsaData);
        m_ok = result == 0;
    }

    ~WSAwrapper()
    {
        WSACleanup();
    }

    bool ok() const
    {
        return m_ok;
    }

private:
    bool    m_ok;
};

static WSAwrapper wsawrapper;

bool    WinsockOK()
{
    return wsawrapper.ok();
}

uint32  Clamp(uint32 value, uint32 low, uint32 high)
{
    return value < low ? low : value > high ? high : value;
}

uint32  Clamp(uint32 value, int32 low, uint32 high)
{
    if (low < 0)
    {
        low = 0;
    }
    return value < (uint32) low ? (uint32) low : value > high ? high : value;
}

uint32  Clamp(uint32 value, uint32 low, int32 high)
{
    if (high < 0)
    {
        high = 0;
    }
    return value < low ? low : value > (uint32) high ? (uint32) high : value;
}

uint32  Clamp(uint32 value, int32 low, int32 high)
{
    if (low < 0)
    {
        low = 0;
    }
    if (high < 0)
    {
        high = 0;
    }
    return value < (uint32) low ? (uint32) low : value > (uint32) high ? (uint32) high : value;
}

int32   Clamp(int32 value, int32 low, int32 high)
{
    return value < low ? low : value > high ? high : value;
}

int32   Clamp(int32 value, uint32 low, int32 high)
{
    if ((int32) low < 0)
    {
        low = 0;
    }
    return value < (int32) low ? (int32) low : value > high ? high : value;
}

int32   Clamp(int32 value, int32 low, uint32 high)
{
    if ((int32) high < 0)
    {
        high = 0;
    }
    return value < low ? low : value > (int32) high ? (int32) high : value;
}

int32   Clamp(int32 value, uint32 low, uint32 high)
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

float   Clamp(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

float   Lerp(float v0, float v1, float l)
{
    return v0 + l * (v1 - v0);
}

int     GetByte(uint8 *data, uint32 *index, uint32 size)
{
    if (*index + 1 > size)
    {
        return -1;
    }
    return data[(*index)++];
}

int     GetWordBE(uint8 *data, uint32 *index, uint32 size)
{
    if (*index + 2 > size)
    {
        return -1;
    }
    int result = data[(*index)++] << 8;
    result |= data[(*index)++];
    return result;
}

int     GetWordLE(uint8 *data, uint32 *index, uint32 size)
{
    if (*index + 2 > size)
    {
        return -1;
    }
    int result = data[(*index)++];
    result |= data[(*index)++] << 8;
    return result;
}

int     GetDwordBE(uint8 *data, uint32 *index, uint32 size)
{
    if (*index + 4 > size)
    {
        return -1;
    }
    int result = data[(*index)++] << 24;
    result |= data[(*index)++] << 16;
    result |= data[(*index)++] << 8;
    result |= data[(*index)++];
    return result;
}

int     GetDwordLE(uint8 *data, uint32 *index, uint32 size)
{
    if (*index + 4 > size)
    {
        return -1;
    }
    int result = data[(*index)++];
    result |= data[(*index)++] << 8;
    result |= data[(*index)++] << 16;
    result |= data[(*index)++] << 24;
    return result;
}

int  ReadJPGMarker(uint8 *data, uint32 *index, uint32 size)
{
    int marker = GetWordBE(data, index, size);
    if (marker == -1 || (marker & 0xff00) != 0xff00)
    {
        return -1;
    }
    return marker & 0xff;
}

int     SkipPayload(uint8 *data, uint32 *index, uint32 size)
{
    int length = GetWordBE(data, index, size);
    if (length < 2)
    {
        return -1;
    }
    *index += length - 2;
    return 0;
}

bool    ReadSOF(uint8 *data, uint32 *index, uint32 size, int *wp, int *hp)
{
    int length = GetWordBE(data, index, size);
    if (length < 5 || wp == NULL || hp == NULL)
    {
        return false;
    }
    (*index)++;

    int const height = GetWordBE(data, index, size);
    int const width = GetWordBE(data, index, size);
    if (width == -1 || height == -1)
    {
        return false;
    }

    *hp = height;
    *wp = width;

    return true;
}

static int GetUE(uint8 *data, uint32 *index, uint32 size)
{
    int v;
    int ch = GetByte(data, index, size);
    if (ch == -1 || ch == 0x80)
    {
        return -1;
    }

    v = ch & 0x7f;
    while (ch & 0x80)
    {
        ch = GetByte(data, index, size);
        if (ch == -1)
        {
            return -1;
        }
        v = v << 7 | ch & 0x7f;
    }
    return v;
}

bool    IsJPG(uint8 *data, uint32 size)
{
    static uint8 JPGSignature[8] =
    {
        0xff, 0xd8, 0xff,
    };

    return size >= 3 && memcmp(data, JPGSignature, 3) == 0;
}

bool    IsPNG(uint8 *data, uint32 size)
{
    static uint8 PNGSignature[8] =
    {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    };

    return size >= 8 && memcmp(data, PNGSignature, 8) == 0;
}

bool    IsBMP(uint8 *data, uint32 size)
{
    static uint8 BMPSignature[2] =
    {
        'B', 'M',
    };

    return size >= 2 && memcmp(data, BMPSignature, 2) == 0;
}

bool    IsBPG(uint8 *data, uint32 size)
{
    static uint8 BPGSignature[4] =
    {
        'B', 'P', 'G', 0xfb
    };

    return size >= 4 && memcmp(data, BPGSignature, 4) == 0;
}

bool    GetJPGDimensions(uint8 *data, uint32 size, int *wp, int *hp)
{
    if (wp == NULL || hp == NULL || !IsJPG(data, size))
    {
        return false;
    }

    uint32 index = 2;
    for (;;)
    {
        int const marker = ReadJPGMarker(data, &index, size);
        if (marker == -1)
        {
            break;
        }
        if (marker >= 0xc0 && marker <= 0xcf && marker != 0xc4 && marker != 0xc8 && marker != 0xcc)
        {
            return ReadSOF(data, &index, size, wp, hp);
        }
        else
        {
            SkipPayload(data, &index, size);
        }
    }
    return false;
}

bool    GetPNGDimensions(uint8 *data, uint32 size, int *wp, int *hp)
{
    if (wp == NULL || hp == NULL || !IsPNG(data, size))
    {
        return false;
    }

    uint32 index = 8;
    int const length = GetDwordBE(data, &index, size);
    if (length != 13)
    {
        return false;
    }
    int const ihdr = GetDwordBE(data, &index, size);
    if (ihdr != 0x49484452) // 'I' 'H' 'D' 'R'
    {
        return false;
    }

    int const width = GetDwordBE(data, &index, size);
    int const height = GetDwordBE(data, &index, size);
    if (width == -1 || height == -1)
    {
        return false;
    }

    *wp = width;
    *hp = height;

    return true;
}

bool    GetBMPDimensions(uint8 *data, uint32 size, int *wp, int *hp)
{
    if (wp == NULL || hp == NULL || !IsBMP(data, size))
    {
        return false;
    }

    uint32 index = 14;
    int const length = GetDwordLE(data, &index, size);
    if (length == 12)
    {
        int const width = GetWordLE(data, &index, size);
        int const height = GetWordLE(data, &index, size);
        if (width == -1 || height == -1)
        {
            return false;
        }

        *wp = width;
        *hp = height;

        return true;
    }
    else if (length == 40 || length == 64)
    {
        int const width = GetDwordLE(data, &index, size);
        int const height = GetDwordLE(data, &index, size);
        if (width == -1 || height == -1)
        {
            return false;
        }

        *wp = width;
        *hp = height;

        return true;
    }
    return false;
}

bool    GetBPGDimensions(uint8 *data, uint32 size, int *wp, int *hp)
{
    if (wp == NULL || hp == NULL || !IsBPG(data, size))
    {
        return false;
    }

    uint32 index = 6;
    int const width = GetUE(data, &index, size);
    int const height = GetUE(data, &index, size);
    if (width == -1 || height == -1)
    {
        return false;
    }

    *wp = width;
    *hp = height;

    return true;
}

void    GeneratePermutation(uint32 *data, int32 size)
{
    // Classic Fisher-Yates shuffle.
    for (int32 i = 0; i < size; i++)
    {
        int32 const j = rnd32(i + 1);
        if (j != i)
        {
            data[i] = data[j];
        }
        data[j] = i;
    }
}

void    RandomHalf(uint32 *data, int32 size)
{
    GeneratePermutation(data, size);

    uint32 const limit = size / 2;
    for (int32 i = 0; i < size; i++)
    {
        data[i] = data[i] < limit ? 0 : 1;
    }
}

#if 0
void    SetErrorLog(char const *errorLog)
{
    strncpy(n_errorLog, errorLog, MAX_PATH);
    n_errorLog[MAX_PATH - 1] = 0;
}

void    RecordError(char const *fmt, ...)
{
    char message[1024];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(message, 1024, "%02d/%02d/%02d %02d:%02d:%02d.%03d ", now.wYear % 100, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(&message[22], 1000, fmt, argptr);
    va_end(argptr);
    n_errorMessages.enqueue(strdup(message));
}

void    WriteErrors()
{
    FILE *fp = NULL;
    char *message;

    if (n_errorLog[0] != 0)
    {
        fp = fopen(n_errorLog, "a");
    }
    while (n_errorMessages.get(message))
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

void    Util::DebugLog(int level, char *message)
{
    Util::RecordError("%s", message);
}
#endif

INT_PTR Atoi(char const *str)
{
    INT_PTR result = 0;
    bool negative = false;

    if (*str == '-')
    {
        str++;
        negative = true;
    }
    while (isdigit(*str))
    {
        result = result * (INT_PTR) 10 + (INT_PTR) (*str++ - '0');
    }
    return negative ? -result : result;
}

static void     ParseDelay(char const *str)
{
    SetDelay((int32) Util::Atoi(str));
}

static void     ParseTimes(char const *str)
{
    int32 sh, sm, eh, em;
    if (sscanf(str, "%d:%d-%d:%d", &sh, &sm, &eh, &em) == 4)
    {
        sh = Util::Clamp(sh, 0, 23);
        sm = Util::Clamp(sm, 0, 59);
        eh = Util::Clamp(eh, 0, 23);
        em = Util::Clamp(em, 0, 59);
        n_startBlankTime = sh * 60 + sm;
        n_endBlankTime = eh * 60 + em;
        logFile.Report("Util::ParseTimes() says times are %02d:%02d-%02d:%02d or %d to %d", sh, sm, eh, em, n_startBlankTime, n_endBlankTime);
    }
    else
    {
        logFile.Report("Util::ParseTimes() fail: arg is [%s]", str);
    }
}

bool    (*ForceBlank)() = nullptr;

static void     SetBlankDLL(char const *str)
{
    if (ForceBlank != NULL)
    {
        return;
    }

    char fullPath[MAX_PATH + 1024];
    // Needs to be static so it has persistence.  It's entirely possible that this routine
    // will return before the thread in the DLL gets started and uses the arg.
    static char arg[1024];
    char ext[_MAX_EXT];

    strncpy(fullPath, str, MAX_PATH + 1018);
    fullPath[MAX_PATH + 1018] = 0;
    char *argp = strchr(fullPath, ':');
    if (argp != NULL)
    {
        *argp++ = 0;
        strncpy(arg, argp, 1024);
        arg[1023] = 0;
    }
    else
    {
        arg[0] = 0;
    }
    _splitpath(fullPath, NULL, NULL, NULL, ext);
    if (ext[0] == 0)
    {
        strcat(fullPath, ".dll");
    }
    HMODULE hModule = LoadLibrary(fullPath);
    if (hModule != NULL)
    {
        bool (*Init)(char const *) = (bool (*)(char const *)) GetProcAddress(hModule, "Init");
        if (Init == NULL)
        {
            FreeLibrary(hModule);
        }
        ForceBlank = (bool (*)()) GetProcAddress(hModule, "ForceBlank");
        if (ForceBlank == NULL)
        {
            FreeLibrary(hModule);
        }
        if (!(*Init)(arg))
        {
            ForceBlank = NULL;
            FreeLibrary(hModule);
        }
    }
}

bool    ParseBlankPort(char const *str)
{
    int port = 0, delay = 0;

    while (isdigit(*str))
    {
        port = port * 10 + *str++ - '0';
    }
    if (*str++ != ':')
    {
        return false;
    }
    while (isdigit(*str))
    {
        delay = delay * 10 + *str++ - '0';
    }
    if (*str || port < 1024 || port > 65534 || delay < 1 || delay > 120)
    {
        return false;
    }
    n_blankPort = port;
    n_blankDuration = delay;
    logFile.Report("Util::ParseBlankPort() n_blankPort == %d, n_blankDuration == %d", n_blankPort, n_blankDuration);
    return true;
}

void    BlankOpt(char const *str)
{
    if (str[0] == '+')
    {
        ParseDelay(&str[1]);
    }
    else if (strchr(str, '-') != NULL)
    {
        ParseTimes(str);
    }
    else if (!ParseBlankPort(str))
    {
        SetBlankDLL(str);
    }
}

DWORD WINAPI    BlankThread(LPVOID unused)
{
    if (!WinsockOK())
    {
        logFile.Report("Util::BlankThread() Winsock startup fail");
        return 0;
    }

    SOCKET hsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hsocket == INVALID_SOCKET)
    {
        logFile.Report("Util::BlankThread() Socket fail");
        return 0;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(n_blankPort);
    saddr.sin_addr.s_addr = INADDR_ANY;
    memset(saddr.sin_zero, 0, 8);
    if (bind(hsocket, (struct sockaddr *) &saddr, sizeof(saddr)) == SOCKET_ERROR)
    {
        logFile.Report("Util::BlankThread() bind fail");
        closesocket(hsocket);
        return 0;
    }

    for (;;)
    {
        //logFile.Report("Util::BlankThread() top of loop");

        fd_set fdset;
        struct timeval tval;

        FD_ZERO(&fdset);
        FD_SET(hsocket, &fdset);

        tval.tv_sec = 3600;
        tval.tv_usec = 0;
        int i = select(0, &fdset, NULL, NULL, &tval);

        if (i < 0)
        {
            logFile.Report("Util::BlankThread() select error");
            closesocket(hsocket);
            return 0;
        }

        if (i > 0 && FD_ISSET(hsocket, &fdset))
        {
            char buff[256];
            recv(hsocket, buff, sizeof(buff), 0);
            ScreenOff();
            Sleep(n_blankDuration * 1000);
            ScreenOn();
        }
    }

    return 0;
}

bool    StartThread()
{
    if (n_blankPort != 0 && n_blankDuration != 0)
    {
        HANDLE hThread = CreateThread(NULL, 0, BlankThread, NULL, 0, NULL);
        if (hThread == NULL)
        {
            return false;
        }
    }
    return true;
}

static char const *BoolStr(bool value)
{
    return value ? "true" : "false";
}

bool    ShowBlack()
{
    int32 const minute = GetCurrentMinute();
    static int32 lastReport = -1;

    if (ForceBlank != NULL && (*ForceBlank)())
    {
        return true;
    }

    if (n_blankDelay > 0)
    {
        static bool reported = false;
        int32 const now = (int32) timeGetTime();
        int32 const running = now - n_startTime;
        if (running > n_blankDelay * 60 * 1000)
        {
            if (minute != lastReport || !reported)
            {
                reported = true;
                lastReport = minute;
                //logFile.Report("Util::ShowBlack() running %d > blankDelay %d * 60000 == %d - returning true", running, n_blankDelay, n_blankDelay * 60000);
            }
            return true;
        }
        if (minute != lastReport)
        {
            //logFile.Report("Util::ShowBlack() running %d <= blankDelay %d * 60000 == %d", running, n_blankDelay, n_blankDelay * 60000);
        }
    }

    if (n_startBlankTime == n_endBlankTime)
    {
        return false;
    }

    if (n_startBlankTime > n_endBlankTime)
    {
        if (minute != lastReport)
        {
            lastReport = minute;
            //logFile.Report("Util::ShowBlack() minute %d >= n_startBlankTime %d || minute %d <= n_endBlankTime %d - %s",
            //    minute, n_startBlankTime, minute, n_endBlankTime, minute >= n_startBlankTime || minute <= n_endBlankTime ? "true" : "false");
        }
        return minute >= n_startBlankTime || minute <= n_endBlankTime;
    }

    if (minute != lastReport)
    {
        lastReport = minute;
        //logFile.Report("Util::ShowBlack() minute %d >= n_startBlankTime %d && minute %d <= n_endBlankTime %d - %s",
        //    minute, n_startBlankTime, minute, n_endBlankTime, minute >= n_startBlankTime && minute <= n_endBlankTime ? "true" : "false");
    }
    return minute >= n_startBlankTime && minute <= n_endBlankTime;
}

void    SetDelay(int32 delay)
{
    delay = max(delay, 0);
    n_blankDelay = delay;
    n_startTime = (int32) timeGetTime();
    //logFile.Report("Util::Setdelay() blankDelay: %d, startTime: %d", n_blankDelay, n_startTime);
}

void    SetTimes(int32 startTime, int32 endTime)
{
    n_startBlankTime = startTime;
    n_endBlankTime = endTime;
}

int32   GetStartTime()
{
    return n_startBlankTime;
}

int32   GetEndTime()
{
    return n_endBlankTime;
}

int32   GetCurrentMinute()
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    return now.wHour * 60 + now.wMinute;
}

void    ParseQueueSize(char const *str)
{
    uint32 queueMin, queueMax;
    if (sscanf(str, "%d-%d", &queueMin, &queueMax) == 2)
    {
        n_queueMin = queueMin;
        n_queueMax = queueMax;
    }
}

uint32  GetQueueMin()
{
    return n_queueMin;
}

uint32  GetQueueMax()
{
    return n_queueMax;
}

void    SetIgnoreMouse(uint32 delay)
{
    n_ignoreMouse = timeGetTime() + delay;
}

bool    ShouldIgnoreMouse()
{
    int32 const delta = (int32) timeGetTime() - (int32) n_ignoreMouse;
    return delta < 0;
}

void    ScreenOff()
{
    if (n_screenOffCount++ == 0)
    {
        screen_off();
    }
}

void    ScreenOn()
{
    if (--n_screenOffCount == 0)
    {
        screen_on();
    }
}

uint32  Aspect(uint32 scrx, uint32 scry)
{
    uint32 const aspectGcd = gcd(scrx, scry);
    uint16 const aspectx = (uint16) (scrx / aspectGcd);
    uint16 const aspecty = (uint16) (scry / aspectGcd);
    return (uint32) aspectx << 16 | aspecty;
}

uint32  FindAspect(char const *str)
{
    while (*str)
    {
        uint32 x, y;
        if (sscanf(str, "%d%*c%d", &x, &y) == 2 && x >= 1 && x < 65536 && y >= 1 && y < 65536)
        {
            return Aspect(x, y);
        }
        str++;
    }
    return 0x00010001;
}


uint32  ParseAspect(string const &resource, string &resourceFile)
{
    uint32 aspect;
    size_t equalsPos = resource.rfind("=");
    if (equalsPos == string::npos)
    {
        resourceFile = resource;
        char fname[_MAX_FNAME];
        _splitpath(resource.c_str(), nullptr, nullptr, fname, nullptr);
        aspect = FindAspect(fname);
    }
    else
    {
        resourceFile = resource.substr(0, equalsPos);
        aspect = FindAspect(&resource[equalsPos]);
    }
    return aspect;
}

float   AspectDistance(uint32 aspect1, uint32 aspect2)
{
    uint32 const num1 = aspect1 & 0xffff;
    uint32 const denom1 = aspect1 & 0xffff0000 ? aspect1 >> 16 : 1;
    uint32 const num2 = aspect2 & 0xffff;
    uint32 const denom2 = aspect2 & 0xffff0000 ? aspect2 >> 16 : 1;
    float const frac1 = (float) num1 / (float) denom1;
    float const frac2 = (float) num2 / (float) denom2;
    float const dist = abs(frac1 - frac2);
    return dist;
}

bool    MouseMoveHeuristic(bool init)
{
    static uint64 maxDelta;
    static uint64 start;
    static LONG lastX;
    static LONG lastY;

    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);

    POINT p;
    GetCursorPos(&p);

    uint64 delta;

    if (!init)
    {
        delta = current.QuadPart - start;
    }

    if (init)
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        // get max delta for 1/20th second.
        maxDelta = frequency.QuadPart / 50;
        delta = maxDelta + 1;
    }

    if (delta > maxDelta)
    {
        start = current.QuadPart;
        lastX = p.x;
        lastY = p.y;
        return false;
    }

    LONG const dx = p.x - lastX;
    LONG const dy = p.y - lastY;
    float const distSq = (float) dx * (float) dx + (float) dy * (float) dy;
    return distSq > 400.0f;
}

vector<string> Explode(string const &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> result;
    while (getline(ss, item, delim))
    {
        if (item != "")
        {
            result.push_back(item);
        }
    }
    return result;
}

#ifdef DEBUG
using std::vector;

void    BuildSmartArray(vector<SmartArray<uint8>> &data_c, uint8 **data, uint32 w, uint32 h)
{
    data_c.resize(h);
    for (uint32 y = 0; y < h; y++)
    {
        SmartArray<uint8> row(data[y], w);
        data_c[y] = row;
    }
}
#endif

};
