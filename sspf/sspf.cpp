#include "pch.h"

#include "sspf.h"
#include "peffect.h"
#include "veffect.h"
#include "context.h"
#include "loader.h"
#include "util.h"
#include "fetchlib.h"

using std::atomic;
using std::bind;
using std::function;
using std::vector;
using std::map;
using std::set;
using std::string;
using std::max;
using std::min;

#define TICKS_PER_SEC           1000
#define VALUE_LEN               256

#define GWL_OFFSET_X            0
#define GWL_OFFSET_Y            4
#define GWL_OFFSET_W            8
#define GWL_OFFSET_H            12
#define GWL_OFFSET_CTX          16

static uint32       updateRate;

static bool         fullScreen = true;
static set<int32>   fullScreenMonitors;
static int32        numWindows = 1;
vector<uint32>      windowParams;
static atomic<int32> windowCount;
static atomic<int32> blackCount;
static bool         lastShowBlack = false;

static bool         configMode = false;
static bool         screenSaver = false;
static uint32       effectVolatility = 30;
static uint32       pictureVolatility = 30;

static bool         previewMode = false;
static HWND         previewHWnd = NULL;
static int32        previewW = 0;
static int32        previewH = 0;

static int32        secPerImage = 60;
static char         resourceStr[RESOURCE_LEN];

static vector<HWND> g_windows;

static HANDLE       hHeartBeatEvent;
static HANDLE       hHeartBeatThread;

static char const   *windowClass = "SSPF_WINDOW_CLASS";

char const *stringof(UINT msg)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        return "WM_KEYDOWN";
    case WM_SYSKEYDOWN:
        return "WM_SYSKEYDOWN";
    case WM_LBUTTONDOWN:
        return "WM_LBUTTONDOWN";
    case WM_LBUTTONUP:
        return "WM_LBUTTONUP";
    case WM_RBUTTONDOWN:
        return "WM_RBUTTONDOWN";
    case WM_RBUTTONUP:
        return "WM_RBUTTONUP";
    case WM_MBUTTONDOWN:
        return "WM_MBUTTONDOWN";
    case WM_MBUTTONUP:
        return "WM_MBUTTONUP";

    }
    return "Unknown";
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int32 x;
    int32 y;
    int32 w;
    int32 h;
    Context *context;

    x = GetWindowLong(hWnd, GWL_OFFSET_X);
    y = GetWindowLong(hWnd, GWL_OFFSET_Y);
    w = GetWindowLong(hWnd, GWL_OFFSET_W);
    h = GetWindowLong(hWnd, GWL_OFFSET_H);
    context = (Context *) GetWindowLongPtr(hWnd, GWL_OFFSET_CTX);

    switch (msg)
    {
    case WM_CREATE:
        Util::SetIgnoreMouse(5000);

        CREATESTRUCT * createStruct;
        createStruct = (CREATESTRUCT *) lParam;
        int32 *coords;
        coords = (int32 *) createStruct->lpCreateParams;

        x = coords[0];
        y = coords[1];
        w = coords[2];
        h = coords[3];

        SetWindowLong(hWnd, GWL_OFFSET_X, x);
        SetWindowLong(hWnd, GWL_OFFSET_Y, y);
        SetWindowLong(hWnd, GWL_OFFSET_W, w);
        SetWindowLong(hWnd, GWL_OFFSET_H, h);

        if (x == CW_USEDEFAULT && y == CW_USEDEFAULT)
        {
            RECT rect;
            RECT wrect;

            GetClientRect(hWnd, &rect);
            GetWindowRect(hWnd, &wrect);

            // force client size to what we want.  I found way back when on
            // Win 95 that I had to do this twice to avoid a bug in the
            // system.  I don't know if that bug still exists, but it does
            // no harm to do it twice.  So we do.
            int32 winw = w + wrect.right - wrect.left - rect.right;
            int32 winh = h + wrect.bottom - wrect.top - rect.bottom;
            SetWindowPos(hWnd, 0, 0, 0, winw, winh, SWP_NOMOVE | SWP_NOZORDER);

            GetClientRect(hWnd, &rect);
            GetWindowRect(hWnd, &wrect);
            winw = w + wrect.right - wrect.left - rect.right;
            winh = h + wrect.bottom - wrect.top - rect.bottom;
            SetWindowPos(hWnd, 0, 0, 0, winw, winh, SWP_NOMOVE | SWP_NOZORDER);
        }
        else if (fullScreen && !previewMode)
        {
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetFocus(hWnd);
        }

        context = new Context(hWnd, w, h, fullScreen);
        SetWindowLongPtr(hWnd, GWL_OFFSET_CTX, (LONG_PTR) context);

        if (!context->Init())
        {
            PostMessage(hWnd, WM_SHUTDOWN, 0, 0);
        }

        if (screenSaver)
        {
            Util::MouseMoveHeuristic(true);
        }

        PostMessage(hWnd, WM_POST_CREATE, 0, 0);

        break;

    case WM_POST_CREATE:
        if (!context->StartThread())
        {
            PostMessage(hWnd, WM_SHUTDOWN, 0, 0);
        }

        if (g_windows.size() > 0 && hWnd == g_windows[0])
        {
            SetTimer(hWnd, 1, 1000, NULL);
        }

        break;

    case WM_DESTROY:
        delete context;
        if (g_windows.size() > 0 && hWnd == g_windows[0])
        {
            KillTimer(hWnd, 1);
        }
        if (--windowCount == 0)
        {
            Util::ScreenOn();
            PostQuitMessage(0);
        }
        break;

    case WM_CLOSE:
        logFile.Report("msg == WM_CLOSE says we terminate");
        PostMessage(hWnd, WM_SHUTDOWN, 1, 0);
        break;

    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_CLOSE:
            logFile.Report("msg == WM_SYSCOMMAND, wParam == SC_CLOSE says we terminate");
            PostMessage(hWnd, WM_SHUTDOWN, 1, 0);
            return(0);

        default:
            break;
        }
        return(DefWindowProc(hWnd, msg, wParam, lParam));

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        // For some completely unknown reason, we get WM_KEYDOWN messages with wParam == 0xff when
        // running in the debugger, even though there's no keyboard action going on at all.
        if (msg == WM_KEYDOWN && wParam == 0xff)
        {
            break;
        }
        if (screenSaver || msg == WM_KEYDOWN && wParam == '\033')
        {
            logFile.Report("screensaver: %s, msg == %s, wParam: '\\%03o', terminating", screenSaver ? "true" : "false", stringof(msg), (int) wParam);
            PostMessage(hWnd, WM_SHUTDOWN, 1, 0);
        }
        break;

    case WM_MOUSEMOVE:
        if (screenSaver && !previewMode && !Util::ShouldIgnoreMouse())
        {
            if (Util::MouseMoveHeuristic(false))
            {
                logFile.Report("msg == WM_MOUSEMOVE, MouseMoveHeuristic() says we terminate");
                PostMessage(hWnd, WM_SHUTDOWN, 1, 0);
            }
        }
        break;

    case WM_TIMER:
        bool showBlack;
        showBlack = Util::ShowBlack();
        if (showBlack && !lastShowBlack)
        {
            blackCount = 0;
        }
        lastShowBlack = showBlack;
        break;

    case WM_GONEBLACK:
        if (++blackCount == numWindows)
        {
            Util::ScreenOff();
        }
        break;

    case WM_SCREENON:
        Util::ScreenOn();
        break;

    case WM_NEWTITLE:
        SetWindowText(hWnd, (char const *) lParam);
        break;

    case WM_FIRSTLOAD:
        if (++windowCount == numWindows)
        {
            SetEvent(hHeartBeatEvent);
        }
        break;

    case WM_PREPARE:
        context->StartPrepare();
        break;

    case WM_TRANSITION:
        context->StartTransition();
        break;

    case WM_HEARTBEAT:
        SetEvent(hHeartBeatEvent);
        break;

    case WM_WRITEERRORS:
        logFile.Flush();
        break;

    case WM_SHUTDOWN:
        if (wParam != 0)
        {
            // Trying to stop this by having it kill itself could take an extended period due to the fact that it sleeps for
            // the transition delay. So we don't bother doing that and just suspend it instead.  That will work well enough
            // to allow us to shut down cleanly
            SuspendThread(hHeartBeatThread);
            Loader::StopThread();
            // Now send a WM_SHUTDOWN to all windows with wParam == 0.  That'll execute the else clause of this for all of
            // them, making them all clean themselves up nicely.
            windowCount = numWindows;
            for (auto hw : g_windows)
            {
                PostMessage(hw, WM_SHUTDOWN, 0, 0);
            }
        }
        else
        {
            context->StopThread();
            DestroyWindow(hWnd);
        }
        break;

    default:
        return(DefWindowProc(hWnd, msg, wParam, lParam));
    }

    return 0;
}

void    ReadOptions()
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, "software\\dgnuff\\sspf", 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return;
    }

    char value[VALUE_LEN];
    DWORD size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "SecPerImage", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        int32 const spi = (int32) Util::Atoi(value);
        if (spi >= 5 && spi <= 3600)
        {
            secPerImage = spi;
        }
    }

    size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "EffectVolatility", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        effectVolatility = Util::Clamp((uint32) Util::Atoi(value), 0, 75);
    }

    size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "PictureVolatility", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        pictureVolatility = Util::Clamp((uint32) Util::Atoi(value), 0, 75);
    }

    size = RESOURCE_LEN - 1;
    if (RegQueryValueEx(hKey, "Resource", NULL, NULL, (LPBYTE) resourceStr, &size) == ERROR_SUCCESS)
    {
        resourceStr[size] = 0;
    }

    int32 startTime = -1;
    int32 endTime = -1;
    size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "StartTime", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        startTime = (int32) Util::Atoi(value);
        if (startTime < 0 || startTime >= 24 * 60)
        {
            startTime = -1;
        }
    }

    size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "EndTime", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        endTime = (int32) Util::Atoi(value);
        if (endTime < 0 || endTime >= 24 * 60)
        {
            endTime = -1;
        }
    }

    if (startTime != -1 && endTime != -1)
    {
        Util::SetTimes(startTime, endTime);
    }

    size = VALUE_LEN - 1;
    if (RegQueryValueEx(hKey, "BlankDelay", NULL, NULL, (LPBYTE) value, &size) == ERROR_SUCCESS)
    {
        value[size] = 0;
        int const delay = (int32) Util::Atoi(value);
        if (delay > 0)
        {
            Util::SetDelay(delay);
        }
    }

    RegCloseKey(hKey);
}

void    WriteOptions()
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, "software\\dgnuff\\sspf", 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return;
    }

    char value[16];
    snprintf(value, 16, "%d", secPerImage);
    DWORD size = (DWORD) strlen(value) + 1;
    RegSetValueEx(hKey, "SecPerImage", 0, REG_SZ, (BYTE *) value, size);

    snprintf(value, 16, "%d", effectVolatility);
    size = (DWORD) strlen(value) + 1;
    RegSetValueEx(hKey, "EffectVolatility", 0, REG_SZ, (BYTE *) value, size);

    snprintf(value, 16, "%d", pictureVolatility);
    size = (DWORD) strlen(value) + 1;
    RegSetValueEx(hKey, "PffectVolatility", 0, REG_SZ, (BYTE *) value, size);

    size = (DWORD) strlen(resourceStr) + 1;
    RegSetValueEx(hKey, "Resource", 0, REG_SZ, (BYTE *) resourceStr, size);

    snprintf(value, 16, "%d", Util::GetStartTime());
    size = (DWORD) strlen(value) + 1;
    RegSetValueEx(hKey, "StartTime", 0, REG_SZ, (BYTE *) value, size);

    snprintf(value, 16, "%d", Util::GetEndTime());
    size = (DWORD) strlen(value) + 1;
    RegSetValueEx(hKey, "EndTime", 0, REG_SZ, (BYTE *) value, size);

    RegCloseKey(hKey);
}

char const  *ProcessArg(function<void(char const *)> lambda, char *arg, char const *cmdLine, bool optional)
{
    if (arg[2] == ':' && arg[3])
    {
        lambda(&arg[3]);
    }
    else if (arg[2])
    {
        lambda(&arg[2]);
    }
    else if (!optional && *cmdLine)
    {
        cmdLine = cmdline(cmdLine, arg, RESOURCE_LEN);
        if (arg[0])
        {
            lambda(arg);
        }
    }
    return cmdLine;
}

bool    ProcessCommandLine(char const *cmdLine)
{
    int32 delay;
    int32 verbose;
#ifdef DEBUG
    float frameRate = 2.0f;
#endif
    function<void(char const *)> lambda;
    HWND prevHWnd;
    RECT prevRect;

    cmdLine = byp(cmdLine);
    if (*cmdLine == 0)
    {
        cmdLine = "-c";
    }

    ReadOptions();
    verbose = 0;

    while (*cmdLine)
    {
        char arg[RESOURCE_LEN];
        cmdLine = cmdline(cmdLine, arg, RESOURCE_LEN - 1);
        if ((arg[0] & 0xfd) == 0x2d)
        {
            // -s - screensaver normal operation
            // -p - screensaver preview mode
            // -c - screensaver config mode
            // -w:n - windowed mode [n windows]
            // -d:n - seconds to display each image
            // -e:File - error file name
            // -b:t1-t2 - switch off between t1 and t2
            // -b:file - set additional switch off dll
            // -b:port:time - set ad hoc UDP blank port
            // -q:m-n - set buffered image queuesize
            // -f:p - set picture shuffle rate
            // -f:e - set effect shuffle rate
            // -y:n - debug only set single full screen monitor
            // -z:n - debug only set transition speed multiplier
            // -r debug only draw border rectangle
            switch (arg[1] | 0x20)  // spcfmwdetrqbYZ
            {
            case 's':
                // /s means screensaver mode.  That controls what will stop us.
                screenSaver = true;
                break;

            case 'p':
                // /p is preview mode
                prevHWnd = 0;
                lambda = [&prevHWnd](char const *str)
                {
                    prevHWnd = (HWND) Util::Atoi(str);
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine);

                if (prevHWnd == 0)
                {
                    return false;
                }
                else if (GetClientRect(prevHWnd, &prevRect) == 0)
                {
                    return false;
                }
                else
                {
                    previewW = prevRect.right - prevRect.left;
                    previewH = prevRect.bottom - prevRect.top;
                    previewHWnd = prevHWnd;
                    previewMode = true;
                }
                break;

            case 'c':
                configMode = true;
                break;

            case 'w':
                lambda = [](char const *str)
                {
                    uint32 x, y;
                    if (sscanf(str, "%d%*c%d", &x, &y) == 2 && x >= 1 && x < 65536 && y >= 1 && y < 65536)
                    {
                        uint32 const aspect = x << 16 | y;
                        windowParams.push_back(aspect);
                        numWindows = (int) windowParams.size();
                        fullScreen = false;
                    }
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine);
                break;

            case 'd':
                delay = 0;
                lambda = [&delay](char const *str)
                {
                    delay = (int32) Util::Atoi(str);
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine);

#ifdef DEBUG
                if (delay >= 2 && delay <= 3600)
#else
                if (delay >= 5 && delay <= 3600)
#endif
                {
                    secPerImage = delay;
                }
                break;

            case 'e':
                // Do nothing lambda, just to consume the arg.  It's already been processed
                // in LogFile::LogFile()
                lambda = [](char const *str)
                {
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine);
                break;

            case 'b':
                cmdLine = ProcessArg(&Util::BlankOpt, arg, cmdLine);
                break;

            case 'q':
                cmdLine = ProcessArg(&Util::ParseQueueSize, arg, cmdLine);
                break;

            case 'f':
                
                break;

            case 'v':
                verbose++;
                break;

//#ifdef DEBUG
            case 'y':
                fullScreen = true;

                int fsm;
                fsm = 0;

                lambda = [&fsm](char const *str)
                {
                    fsm = (int32) Util::Atoi(str);
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine, true);
                if (fsm > 0)
                {
                    fullScreenMonitors.insert(fsm - 1);
                }
                break;

#if 0
            case 'z':
                lambda = [&frameRate](char const *str)
                {
                    frameRate = (float) atof(str);
                };

                cmdLine = ProcessArg(lambda, arg, cmdLine);
                break;
#endif
//#endif
            case 'r':
                VEffect::s_borderRect = true;
                break;

            default:
                break;
            }
        }
        else
        {
            if (Loader::AddResources(arg) > 0)
            {
                resourceStr[0] = 0;
            }
        }
    }
    if (resourceStr[0] != 0)
    {
        Loader::AddResources(resourceStr);
    }

#ifdef DEBUG
    Context::SetFrameRate(frameRate);
#endif

    return true;
}

bool    RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX winClass;

    winClass.lpszClassName = windowClass;
    winClass.cbSize = sizeof(WNDCLASSEX);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = WindowProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    winClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName = NULL;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 5 * sizeof(int32) + sizeof(Context *);

    return RegisterClassEx(&winClass) != 0;
}

HWND    SetupWindow(HINSTANCE hInstance, int32 cmdShow, DWORD style, int32 *coords)
{
    HWND parent = previewMode ? previewHWnd : NULL;
    HWND hWnd;

    hWnd = CreateWindowEx(NULL, windowClass, "Digital Picture Frame", style,
                    coords[0], coords[1], coords[2], coords[3],
                    parent, NULL, hInstance, (LPVOID) coords);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, cmdShow);
        UpdateWindow(hWnd);

        HWND hCurWnd = GetForegroundWindow();
        DWORD dwMyID = GetCurrentThreadId();
        DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
        AttachThreadInput(dwCurID, dwMyID, TRUE);
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SetForegroundWindow(hWnd);
        AttachThreadInput(dwCurID, dwMyID, FALSE);
        SetFocus(hWnd);
        SetActiveWindow(hWnd);
    }
    return hWnd;
}

bool    SetupWindows(HINSTANCE hInstance, int32 cmdShow)
{
    int32 coords[4];
    HWND hWnd = NULL;
    bool succeeded;

    succeeded = true;
    if (previewMode)
    {
        coords[0] = CW_USEDEFAULT;
        coords[1] = CW_USEDEFAULT;
        coords[2] = previewW;
        coords[3] = previewH;
        hWnd = SetupWindow(hInstance, cmdShow, WS_CHILD | WS_VISIBLE, coords);
        if (hWnd != NULL)
        {
            g_windows.push_back(hWnd);
        }
        else
        {
            succeeded = false;
        }
    }
    else if (fullScreen)
    {
        DISPLAY_DEVICE dd;
        DEVMODE dm;
        DWORD monitorNum = 0;

        succeeded = false;
        dd.cb = sizeof(DISPLAY_DEVICE);
        dm.dmSize = sizeof(DEVMODE);
        dm.dmDriverExtra = 0;
        for (DWORD devNum = 0; EnumDisplayDevices(NULL, devNum, &dd, EDD_GET_DEVICE_INTERFACE_NAME); devNum++)
        {
            if (EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
            {
                if (fullScreenMonitors.empty() || fullScreenMonitors.find(monitorNum) != fullScreenMonitors.end())
                {
                    coords[0] = dm.dmPosition.x;
                    coords[1] = dm.dmPosition.y;
                    coords[2] = dm.dmPelsWidth;
                    coords[3] = dm.dmPelsHeight;
                    hWnd = SetupWindow(hInstance, cmdShow, WS_POPUP | WS_SYSMENU | WS_VISIBLE, coords);
                    if (hWnd != NULL)
                    {
                        g_windows.push_back(hWnd);
                        succeeded = true;
                    }
                }
                monitorNum++;
            }
        }
    }
    else
    {
        for (int32 i = 0; i < numWindows; i++)
        {
            coords[0] = CW_USEDEFAULT;
            coords[1] = CW_USEDEFAULT;
            if (i < windowParams.size())
            {
                uint32 const aspect = windowParams[i];
                coords[2] = aspect >> 16;;
                coords[3] = aspect & 0xffff;
            }
            else
            {
                coords[2] = XSIZE;
                coords[3] = YSIZE;
            }
            hWnd = SetupWindow(hInstance, cmdShow, WS_OVERLAPPEDWINDOW | WS_VISIBLE, coords);
            if (hWnd != NULL)
            {
                g_windows.push_back(hWnd);
            }
            else
            {
                succeeded = false;
            }
        }
    }

    Loader::SetHWnd(hWnd);

    numWindows = (int32) g_windows.size();
    windowCount = 0;

    if (!succeeded)
    {
        for (auto hw : g_windows)
        {
            DestroyWindow(hw);
        }
    }

    return succeeded;
}

void    SetupQueue()
{
    uint32 const queueMin = max(Util::GetQueueMin(), (uint32) numWindows);
    uint32 const queueMax = max(Util::GetQueueMax(), (uint32) (numWindows * 2));
    Loader::SetQueueSize(queueMin, queueMax);
}

int32   curMS()
{
    FILETIME now;
    GetSystemTimeAsFileTime(&now);
    int64 const ticks = (int64) now.dwHighDateTime << 32 | now.dwLowDateTime;
    int32 const ms = (int32) (ticks / 10000);
    return ms;
}

static DWORD WINAPI HeartBeatThread(LPVOID unused)
{
    WaitForSingleObject(hHeartBeatEvent, INFINITE);

    for (auto hWnd : g_windows)
    {
        PostMessage(hWnd, WM_PREPARE, 0, 0);
        WaitForSingleObject(hHeartBeatEvent, INFINITE);
        PostMessage(hWnd, WM_TRANSITION, 0, 0);
        WaitForSingleObject(hHeartBeatEvent, INFINITE);
    }

    int32 const transtionDelay = (int32) ((5.0f * TICKS_PER_SEC) / Context::GetFrameRate());
    int32 const updateRate = (previewMode ? 10 : secPerImage) * TICKS_PER_SEC + transtionDelay;
    int32 ticksRemaining;
    for (;;)
    {
        for (auto hWnd : g_windows)
        {
            int32 const prepareStart = curMS();
            PostMessage(hWnd, WM_PREPARE, 0, 0);
            WaitForSingleObject(hHeartBeatEvent, INFINITE);
            int32 const prepareTicks = curMS() - prepareStart;
            ticksRemaining = max(updateRate - prepareTicks, 0);
            // TODO fix this, we don't need the while loop
            while (ticksRemaining > 0)
            {
                int32 const ticksToSleep = min(ticksRemaining, 30000);
                Sleep(ticksToSleep);
                ticksRemaining -= ticksToSleep;
            }
            PostMessage(hWnd, WM_TRANSITION, 0, 0);
            WaitForSingleObject(hHeartBeatEvent, INFINITE);
        }
    }
}

static bool     StartThread()
{
    hHeartBeatEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hHeartBeatEvent == NULL)
    {
        return false;
    }
    hHeartBeatThread = CreateThread(NULL, 0, HeartBeatThread, NULL, 0, NULL);
    if (hHeartBeatThread == NULL)
    {
        return false;
    }
    return true;
}

INT_PTR CALLBACK ConfigFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int32 editValue;
    int32 h;
    int32 m;
    int32 start;
    int32 end;
    char buff[32];

    switch (msg)
    {
    case WM_INITDIALOG:
        snprintf(buff, 32, "%d", secPerImage);
        SetDlgItemText(hWnd, EDITD, buff);

        snprintf(buff, 32, "%d", effectVolatility);
        SetDlgItemText(hWnd, EDITF, buff);

        snprintf(buff, 32, "%d", pictureVolatility);
        SetDlgItemText(hWnd, EDITP, buff);

        //CheckDlgButton(hWnd, CHECKS, shuffle ? 1 : 0);

        SetDlgItemText(hWnd, EDITR, resourceStr);

        snprintf(buff, 32, "%02d:%02d", Util::GetStartTime() / 60, Util::GetStartTime() % 60);
        SetDlgItemText(hWnd, EDITS, buff);

        snprintf(buff, 32, "%02d:%02d", Util::GetEndTime() / 60, Util::GetEndTime() % 60);
        SetDlgItemText(hWnd, EDITE, buff);

        SetFocus(GetDlgItem(hWnd, EDITR));
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hWnd, EDITR, resourceStr, RESOURCE_LEN - 1);
            resourceStr[RESOURCE_LEN - 1] = 0;

            //shuffle = IsDlgButtonChecked(hWnd, CHECKS) != 0;

            GetDlgItemText(hWnd, EDITD, buff, 32);
            buff[31] = 0;
            editValue = (int32) Util::Atoi(buff);
            if (editValue >= 5 && editValue <= 3600)
            {
                secPerImage = editValue;
            }

            GetDlgItemText(hWnd, EDITF, buff, 32);
            buff[31] = 0;
            editValue = (int32) Util::Atoi(buff);
            if (editValue >= 0 && editValue <= 75)
            {
                effectVolatility = editValue;
            }

            GetDlgItemText(hWnd, EDITP, buff, 32);
            buff[31] = 0;
            editValue = (int32) Util::Atoi(buff);
            if (editValue >= 0 && editValue <= 75)
            {
                pictureVolatility = editValue;
            }

            GetDlgItemText(hWnd, EDITS, buff, 32);
            buff[31] = 0;
            if (sscanf(buff, "%d:%d", &h, &m) == 2 && h >= 0 && h < 24 && m >= 0 && m < 60)
            {
                start = h * 60 + m;
            }
            else
            {
                start = -1;
            }

            GetDlgItemText(hWnd, EDITE, buff, 32);
            buff[31] = 0;
            if (sscanf(buff, "%d:%d", &h, &m) == 2 && h >= 0 && h < 24 && m >= 0 && m < 60)
            {
                end = h * 60 + m;
            }
            else
            {
                end = -1;
            }

            if (start != -1 && end != -1)
            {
                Util::SetTimes(start, end);
            }

            EndDialog(hWnd, 1);
            return TRUE;

        case IDCANCEL:
            EndDialog(hWnd, 0);
            return TRUE;

        default:
            break;
        }
        break;
    }
    return FALSE;
}

int32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int32 cmdShow)
{
    srand32(0);

    Loader::InitFetchers();
    Context::InitCallbacks();

    if (!ProcessCommandLine(cmdLine))
    {
        logFile.Report("ProcessCommandLine() error");
        logFile.Flush();
        return E_FAIL;
    }

    if (configMode)
    {
        if (DialogBox(NULL, MAKEINTRESOURCE(IDD_CONFIG), NULL, ConfigFunc) != 0)
        {
            WriteOptions();
        }
        logFile.Report("Got into config mode somehow");
        logFile.Flush();
        return 0;
    }

    if (Loader::FinalizeResources(pictureVolatility) == 0)
    {
        logFile.Report("Loader::FinalizeResources() error");
        logFile.Flush();
        return E_FAIL;
    }

    Loader::InitEffects(previewMode, effectVolatility);

    if (!RegisterWindowClass(hInstance))
    {
        logFile.Report("RegisterWindowClass() error");
        logFile.Flush();
        return E_FAIL;
    }

    if (!StartThread())
    {
        logFile.Report("StartThread() error");
        logFile.Flush();
        return E_FAIL;
    }

    if (!SetupWindows(hInstance, cmdShow))
    {
        logFile.Report("SetupWindows() error");
        logFile.Flush();
        return E_FAIL;
    }

    SetupQueue();

    if (fullScreen && !previewMode)
    {
        ShowCursor(0);
    }

    Loader::StartThread();
    Util::StartThread();

    MSG msg;
    memset(&msg, 0, sizeof(msg));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (fullScreen && !previewMode)
    {
        ShowCursor(1);
    }

    UnregisterClass(windowClass, hInstance);
    Loader::ShutdownFetchers();
    logFile.Flush();

    return (int32) msg.wParam;
}
