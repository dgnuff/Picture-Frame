#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <clib.h>
#include "MonitorSwitch.h"

static  unsigned int    taskbarmsg;
static  HMENU           hPopup;
static  HWND            hWindow;
static  HICON           hIcon;
static  char const      className[] = "_Monitor_Switch_";
static  char const      windowTitle[] = "Monitor Switch";
static SOCKET           listen_sock;
static unsigned short   listen_port;
static int              ticks_remaining = 0;

class WSAwrapper
{
public:
    WSAwrapper()
    {
        int result;
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

static void BlankLocal(int delay)
{
    Sleep(5000);
    screen_off();
    Sleep(delay * 1000);
    screen_on();
}

static unsigned long    ResolveName(char *hostname)
{
    struct addrinfo *result;
    struct addrinfo *ptr;
    struct sockaddr_in  *sockaddr_ipv4;
    unsigned long addr;

    if (getaddrinfo(hostname, NULL, NULL, &result) != 0)
    {
        return INADDR_NONE;
    }

    addr = INADDR_NONE;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        if (ptr->ai_family == AF_INET)
        {
            sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
            addr = sockaddr_ipv4->sin_addr.s_addr;
            break;
        }
    }

    freeaddrinfo(result);

    return addr;
}

static bool BlankRemote(char const *params)
{
    char hostname[256];
    char portstr[64];
    char delay[64];

    int n = sscanf(params, "%250[^:]:%60[^:]:%60[^:]", hostname, portstr, delay);

    if (n == 2)
    {
        delay[0] = 'a';
        delay[1] = 0;
    }

    if (n != 2 && n != 3)
    {
        return false;
    }

    if (!wsawrapper.ok())
    {
        return true;
    }

    unsigned long address = ResolveName(hostname);
    if (address == INADDR_NONE)
    {
        return true;
    }

    unsigned short port = (unsigned short) atoi(portstr);

    if (port <= 1024)
    {
        return true;
    }

    SOCKET hsocket = INVALID_SOCKET;

    hsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hsocket == INVALID_SOCKET)
    {
        return true;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = address;
    saddr.sin_port = htons(port);
    memset(saddr.sin_zero, 0, 8);

    sendto(hsocket, delay, (int) strlen(delay) + 1, 0, (struct sockaddr *) &saddr, sizeof(saddr));

    closesocket(hsocket);

    return true;
}

void AddTrayIcon()
{
    NOTIFYICONDATA tnd;

    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = hWindow;
    tnd.uID = TRAY_ID;
    tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnd.uCallbackMessage = WM_TRAY;
    tnd.hIcon = hIcon;
    strncpy(tnd.szTip, windowTitle, sizeof(tnd.szTip) - 1);
    tnd.szTip[sizeof(tnd.szTip) - 1] = 0;
    Shell_NotifyIcon(NIM_ADD, &tnd);
}

void DeleteTrayIcon()
{
    NOTIFYICONDATA tnd;

    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = hWindow;
    tnd.uID = TRAY_ID;
    tnd.uFlags = NIF_MESSAGE | NIF_ICON;
    tnd.uCallbackMessage = WM_TRAY;
    tnd.hIcon = hIcon;
    *tnd.szTip = '\0';
    Shell_NotifyIcon(NIM_DELETE, &tnd);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (taskbarmsg != 0 && msg == taskbarmsg)
    {
        AddTrayIcon();
        return(0);
    }

    switch (msg)
    {
    case WM_CREATE:
        hWindow = hwnd;

        /* Network service. */
        listen_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (listen_sock == INVALID_SOCKET)
        {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }

        if (WSAAsyncSelect(listen_sock, hwnd, WM_WSAASYNC, FD_WRITE | FD_ACCEPT | FD_READ | FD_CLOSE) == SOCKET_ERROR)
        {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }

        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(listen_port);

        if (bind(listen_sock, (SOCKADDR *) &addr, sizeof(addr)) == SOCKET_ERROR)
        {
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        }

        taskbarmsg = RegisterWindowMessage("TaskbarCreated");
        AddTrayIcon();

        SetTimer(hwnd, IDT_TIMER, 1000, NULL);

        break;

    case WM_TIMER:
        if (ticks_remaining && --ticks_remaining == 0)
        {
            screen_on();
        }
        break;

    case WM_WSAASYNC:
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_READ:
            int errCode;
            errCode = WSAGETSELECTERROR(lParam);
            if (errCode)
            {
                return FALSE;
            }

            char data[64];
            int bytes_recv;
            bytes_recv = recv(wParam, (char *) data, 64, 0);

            if (bytes_recv == SOCKET_ERROR)
            {
                return FALSE;
            }
            if (bytes_recv > 0)
            {
                int delay = atoi(data);
                if (delay < 1 || delay > 120)
                {
                    delay = 15;
                }
                screen_off();
                ticks_remaining = delay;
            }

            return 0;

        default:
            break;
        }
        break;

    case WM_TRAY:
        switch (lParam)
        {
        case WM_RBUTTONDOWN:
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            int i;
            i = TrackPopupMenu(hPopup, 0, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
            if (i)
            {
                PostMessage(hwnd, WM_COMMAND, i, 0);
            }
            break;

        case WM_LBUTTONDBLCLK:
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_RESTORE);
            break;
        }
        break;

    case WM_CLOSE:
        closesocket(listen_sock);
        KillTimer(hwnd, IDT_TIMER);
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        DeleteTrayIcon();
        PostQuitMessage(0);
        break;

    case WM_SYSCOMMAND:
        switch (LOWORD(wParam))
        {
        case SC_MINIMIZE:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_RESTORE:
            ShowWindow(hwnd, SW_RESTORE);
            break;

        default:
            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static bool ServerMode(HINSTANCE hInstance, char *str)
{
    if ((str[0] | 0x20) == 's' && str[1] == ':')
    {
        int port = atoi(&str[2]);
        if (port > 1024 && port <= 65535)
        {
            if (!wsawrapper.ok())
            {
                return true;
            }

            listen_port = (unsigned short) port;

            HMENU hMenu = LoadMenu(hInstance, "ContextMenu");
            hPopup = GetSubMenu(hMenu, 0);
            SetMenuDefaultItem(hPopup, IDM_RESTORE, FALSE);
            hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

            HWND hwnd;
            WNDCLASSEX wc;
            MSG Msg;

            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = 0;
            wc.lpfnWndProc = WndProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = hInstance;
            wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
            wc.lpszMenuName = NULL;
            wc.lpszClassName = className;
            wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

            if (!RegisterClassEx(&wc))
            {
                MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
                return true;
            }

            hwnd = CreateWindowEx(0, className, windowTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, NULL, NULL, hInstance, NULL);

            if (hwnd == NULL)
            {
                MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
                return true;
            }

            ShowWindow(hwnd, SW_HIDE);
            UpdateWindow(hwnd);

            while (GetMessage(&Msg, NULL, 0, 0) > 0)
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }

            return true;
        }
    }
    return false;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char command[256];
    cmdline(lpCmdLine, command, 256);

    if (command[0] == 0)
    {
        BlankLocal(15);
        return 0;
    }

    if (!ServerMode(hInstance, command) && !BlankRemote(command))
    {
        int delay = atoi(command);
        if (delay >= 1 && delay <= 120)
        {
            BlankLocal(delay);
        }
    }

    return 0;
}
