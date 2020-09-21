/*
 * imageget.cpp
 *
 * Image scale utility
 */

#include "fetchlib.h"
#include "imageget.h"
#include <vector>
#include <algorithm>

using std::vector;
using std::max;
using std::min;

int32 i = VOS__WINDOWS32;

HWND main_window = NULL;

char outfile[MAX_PATH];

char IMGName[] = "Image subsample";
char IMGClass[] = "IMAGE_GET";

HICON hicon;

HDC ddc;
HBITMAP dbm;
HBITMAP odbm;
HPEN colorPens[4];
HPEN greenPen;
HPEN bluePen;
int penIndex = 0;

#define boxPen          colorPens[penIndex]
#define redPen          colorPens[0]
#define yellowPen       colorPens[1]
#define whitePen        colorPens[2]
#define blackPen        colorPens[3]

HCURSOR arrowCursor;
HCURSOR weCursor;
HCURSOR nsCursor;
HCURSOR nwseCursor;
HCURSOR neswCursor;

int32 windoww;
int32 windowh;

int32 imgw;
int32 imgh;

float aspectRatio;

int32 xoffset;
int32 yoffset;
float scale;

#define         MM_NONE         0
#define         MM_MARKED       1
#define         MM_DRAGHORIZ    2
#define         MM_DRAGVERT     4
#define         MM_MOVELEFT     8
#define         MM_MOVETOP      16
#define         MM_MOVERIGHT    32
#define         MM_MOVEBOTTOM   64

int32 markMode;
int32 cursorMode;

RECT brect;     // Bounding rectangle of the source image in client window coordinates
RECT wrect;     // Bounding rectangle of selected region in client window coordinates
RECT orect;     // Bounding rectangle of original region in client window coordinates
FRECT irect;    // Bounding rectangle of selected region in source image coordinates

bool moving;
bool saved;

int32 mouseDownX;
int32 mouseDownY;
int32 dragOffsetX;
int32 dragOffsetY;

// source image buffer
uint8 imagebuf[SIZE][SIZE * 3];
uint8 *image[SIZE];

// display image buffer
uint8 dimagebuf[SIZE][SIZE * 3];
uint8 *dimage[SIZE];

vector<char *> fileList;
char *curfile;
int32 currentfile;
int32 prevCurr;
int32 moveDir;
int32 fileCount;

#define MAX_FETCHER     8
Fetcher *fetchers[MAX_FETCHER];
int32 numFetcher;

uint8 JPEG_SIG[] = { 0xFF, 0xD8, 0xFF };
uint8 JPEG_BYTE4[] = { 0xDB, 0xE0, 0xE1, 0xE2, 0xE3, 0xE8 };
uint8 PNG_SIG[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

static void logMsg(int32 level, char *msg)
{
    if (level == 0)
    {
        return;
    }
    FILE *fp = fopen("resource.log", "a");
    if (fp != NULL)
    {
        fprintf(fp, "%s\n", msg);
        fclose(fp);
    }
}

static void AddFile(char *file)
{
    char *newFile = strdup(file);
    if (newFile != NULL)
    {
        fileList.push_back(newFile);
    }
}

static void AddLine(char *buff)
{
    int32 i;
    Fetcher *fetcher;
    Resource *resource;
    char line[RESOURCE_LEN];

    if (buff[0] != '@')
    {
        AddFile(buff);
    }
    else
    {
        buff++;
        for (i = 0; i < numFetcher; i++)
        {
            fetcher = fetchers[i];
            if (prefixi(buff, fetcher->Name()) && memcmp(&buff[strlen(fetcher->Name())], "://", 3) == 0)
            {
                if ((resource = fetcher->Prepare(buff)) != NULL)
                {
                    while (fetcher->Gets(resource, line, RESOURCE_LEN) != EOF)
                    {
                        AddLine(line);
                    }
                    fetcher->Finish(resource);
                    free(resource);
                }
                break;
            }
        }
        if (i == numFetcher)
        {
            fetcher = fetchers[0];
            if ((resource = fetcher->Prepare(buff)) != NULL)
            {
                while (fetcher->Gets(resource, line, RESOURCE_LEN) != EOF)
                {
                    AddLine(line);
                }
                fetcher->Finish(resource);
                free(resource);
            }
        }
    }
}

static int32 Compare(void *a, void *b)
{
    struct _scnfile_ *c;
    struct _scnfile_ *d;

    c = (struct _scnfile_ *) a;
    d = (struct _scnfile_ *) b;
    return strcmp(c->_filename, d->_filename);
}

static void Scanfiles(char *filename)
{
    int32 i;
    DWORD attribs;
    struct _scnfile_ *work;

    for (i = 0; filename[i]; i++)
    {
        if (filename[i] == '/')
        {
            filename[i] = '\\';
        }
    }

    if (i == 0)
    {
        strcpy(filename, ".\\");
        i = 2;
    }
    else if (filename[i - 1] == '\\' && i == 3)
        ;
    else
    {
        if (filename[i - 1] == '\\')
        {
            filename[--i] = 0;
            attribs = FILE_ATTRIBUTE_DIRECTORY;
        }
        else
        {
            attribs = GetFileAttributes(filename);
        }
        if (attribs == 0xffffffff || !(attribs & FILE_ATTRIBUTE_DIRECTORY))
        {
            while (i && filename[i - 1] != ':' && filename[i - 1] != '\\')
            {
                i--;
            }
        }
        else
        {
            filename[i++] = '\\';
        }
    }

    strcpy(&filename[i], "*.jpg");
    work = scnwld(filename, 0);
    strcpy(&filename[i], "*.png");
    work = (_scnfile_ *) lconcat(work, scnwld(filename, 0));

    if (work != NULL)
    {
        work = (_scnfile_ *) lsort(work, Compare);
        while (work != NULL)
        {
            AddFile(work->_filename);
            work = work->_next;
        }
        lfree(work);
    }
}

void SetupScale()
{
    int32 outdim;

    brect.left = 0;
    brect.top = 0;
    brect.right = windoww;
    brect.bottom = windowh;

    if (imgw * windowh > windoww * imgh)
    {
        scale = (float) windoww / (float) imgw;
        xoffset = 0;
        outdim = (int32) ((float) imgh * scale);
        yoffset = (windowh - outdim) / 2;
        brect.top += yoffset;
        brect.bottom -= yoffset;
    }
    else if (imgw * windowh < windoww * imgh)
    {
        scale = (float) windowh / (float) imgh;
        yoffset = 0;
        outdim = (int32) ((float) imgw * scale);
        xoffset = (windoww - outdim) / 2;
        brect.left += xoffset;
        brect.right -= xoffset;
    }
    else
    {
        scale = (float) windoww / (float) imgw;
        xoffset = 0;
        yoffset = 0;
    }
}

bool CloseTo(int32 a, LONG b)
{
    return a >= b - 2 && a <= b + 2;
}

void AdjustRects(bool preferHorizontal, bool lockLeft, bool lockTop, bool recursionok)
{
    wrect.left = wrect.left < brect.left ? brect.left : wrect.left > brect.right ? brect.right : wrect.left;
    wrect.right = wrect.right < brect.left ? brect.left : wrect.right > brect.right ? brect.right : wrect.right;
    wrect.top = wrect.top < brect.top ? brect.top : wrect.top > brect.bottom ? brect.bottom : wrect.top;
    wrect.bottom = wrect.bottom < brect.top ? brect.top : wrect.bottom > brect.bottom ? brect.bottom : wrect.bottom;

    if (wrect.left > wrect.right)
    {
        int32 const t = wrect.left;
        wrect.left = wrect.right;
        wrect.right = t;
    }
    if (wrect.top > wrect.bottom)
    {
        int32 const t = wrect.top;
        wrect.top = wrect.bottom;
        wrect.bottom = t;
    }

    irect.left = (float) (wrect.left - xoffset) / scale;
    irect.right = (float) (wrect.right - xoffset) / scale;
    irect.top = (float) (wrect.top - yoffset) / scale;
    irect.bottom = (float) (wrect.bottom - yoffset) / scale;

    float const imgWidth = irect.right - irect.left;
    float const imgHeight = irect.bottom - irect.top;
    float const imgAspect = imgWidth / imgHeight;

    if (preferHorizontal)
    {
        float const desiredHeight = imgWidth / aspectRatio;
        if (lockTop)
        {
            irect.bottom = irect.top + desiredHeight;
            if (irect.bottom > (float) imgh)
            {
                irect.bottom = (float) imgh;
                irect.top = (float) imgh - desiredHeight;
                if (irect.top < 0.0f)
                {
                    irect.top = 0.0f;
                    AdjustRects(false, lockLeft, true, false);
                }
            }
        }
        else
        {
            irect.top = irect.bottom - desiredHeight;
            if (irect.top < 0.0f)
            {
                irect.top = 0.0f;
                irect.bottom = desiredHeight;
                if (irect.bottom > (float) imgh)
                {
                    irect.bottom = (float) imgh;
                    AdjustRects(false, lockLeft, false, false);
                }
            }
        }
    }
    else
    {
        float const desiredWidth = imgHeight * aspectRatio;
        if (lockLeft)
        {
            irect.right = irect.left + desiredWidth;
            if (irect.right > (float) imgw)
            {
                irect.right = (float) imgw;
                irect.left = (float) imgw - desiredWidth;
                if (irect.left < 0.0f)
                {
                    irect.left = 0.0f;
                    AdjustRects(true, true, lockTop, false);
                }
            }
        }
        else
        {
            irect.left = irect.right - desiredWidth;
            if (irect.left < 0.0f)
            {
                irect.left = 0.0f;
                irect.right = desiredWidth;
                if (irect.right > (float) imgw)
                {
                    irect.right = (float) imgw;
                    AdjustRects(true, false, lockTop, false);
                }
            }
        }
    }

    float const iw = (float) imgw;
    float const ih = (float) imgh;

    irect.left = irect.left < 0.0f ? 0.0f : irect.left > iw ? iw : irect.left;
    irect.right = irect.right < 0.0f ? 0.0f : irect.right > iw ? iw : irect.right;
    irect.top = irect.top < 0.0f ? 0.0f : irect.top > ih ? ih : irect.top;
    irect.bottom = irect.bottom < 0.0f ? 0.0f : irect.bottom > ih ? ih : irect.bottom;
    if (irect.left > irect.right)
    {
        float const t = irect.left;
        irect.left = irect.right;
        irect.right = t;
    }
    if (irect.top > irect.bottom)
    {
        float const t = irect.top;
        irect.top = irect.bottom;
        irect.bottom = t;
    }

    wrect.left = (int32) (irect.left * scale + 0.5f) + xoffset;
    wrect.right = (int32) (irect.right * scale + 0.5f) + xoffset;
    wrect.top = (int32) (irect.top * scale + 0.5f) + yoffset;
    wrect.bottom = (int32) (irect.bottom * scale + 0.5f) + yoffset;
}

void    MatchWidth()
{
    if (orect.left == -1 || orect.top == -1 || orect.right == -1 || orect.bottom == -1)
    {
        return;
    }
    wrect.left = orect.left;
    wrect.right = orect.right;
    int32 const center = (orect.top + orect.bottom) / 2;
    int32 const height = (int32) ((float) (wrect.right - wrect.left) / aspectRatio);
    wrect.top = center - height / 2;
    wrect.bottom = wrect.top + height;
    AdjustRects(true, true, true, true);
}

void    MatchHeight()
{
    if (orect.left == -1 || orect.top == -1 || orect.right == -1 || orect.bottom == -1)
    {
        return;
    }
    wrect.top = orect.top;
    wrect.bottom = orect.bottom;
    int32 const center = (orect.left + orect.right) / 2;
    int32 const width = (int32) ((float) (wrect.bottom - wrect.top) * aspectRatio);
    wrect.left = center - width / 2;
    wrect.right = wrect.left + width;
    AdjustRects(false, true, true, true);
}

static uint8 *fetchData;
static int32 fetchAvail;

LRESULT CALLBACK WindowProc(HWND hwnd, unsigned msg, WPARAM wparam, LPARAM lparam)
{
    int32 i;
    int32 j;
    int32 w;
    int32 h;
    int32 x;
    int32 y;
    int32 mx;
    int32 my;
    int32 dx;
    int32 dy;
    bool prefHoriz;
    char *cp;
    char *dp;
    char *ep;
    Fetcher *fetcher;
    Resource *resource;
    FILE *fp;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    POINT pt;
    BITMAPINFO bmi;
    char title[256];
    char localFile[RESOURCE_LEN];

    switch (msg)
    {
    case WM_CREATE:
        main_window = hwnd;

        for (i = 0; i < SIZE; i++)
        {
            image[i] = &imagebuf[i][0];
            dimage[i] = &dimagebuf[i][0];
        }

        GetClientRect(hwnd, &rect);
        windoww = rect.right - rect.left;
        windowh = rect.bottom - rect.top;

        hdc = GetDC(hwnd);
        ddc = CreateCompatibleDC(hdc);
        dbm = CreateCompatibleBitmap(hdc, windoww, windowh);
        odbm = (HBITMAP) SelectObject(ddc, dbm);
        ReleaseDC(hwnd, hdc);

        redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        yellowPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
        whitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        bluePen = CreatePen(PS_SOLID, 1, RGB(0, 128, 255));

        markMode = MM_NONE;
        saved = false;
        fileCount = 0;

        break;

    case WM_DESTROY:
        if (redPen != NULL)
        {
            DeleteObject(redPen);
        }
        if (yellowPen != NULL)
        {
            DeleteObject(yellowPen);
        }
        if (whitePen != NULL)
        {
            DeleteObject(whitePen);
        }
        if (blackPen != NULL)
        {
            DeleteObject(blackPen);
        }
        if (greenPen != NULL)
        {
            DeleteObject(greenPen);
        }
        if (bluePen != NULL)
        {
            DeleteObject(bluePen);
        }
        if (odbm != NULL)
        {
            SelectObject(ddc, odbm);
        }
        DeleteDC(ddc);
        if (dbm != NULL)
        {
            DeleteObject(dbm);
        }

        PostQuitMessage(0);
        break;

    case WM_ENTERSIZEMOVE:
        moving = true;
        break;

    case WM_SIZE:
        if (moving)
        {
            break;
        }
        // fall through

    case WM_EXITSIZEMOVE:
        moving = false;
        hdc = GetDC(hwnd);
        SelectObject(ddc, odbm);
        DeleteObject(dbm);
        GetClientRect(hwnd, &rect);
        windoww = rect.right - rect.left;
        windowh = rect.bottom - rect.top;
        dbm = CreateCompatibleBitmap(hdc, windoww, windowh);
        odbm = (HBITMAP) SelectObject(ddc, dbm);
        ReleaseDC(hwnd, hdc);
        PostMessage(hwnd, WM_RESIZEIMAGE, 0, 0);
        break;

    case WM_UPDATE:
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;

    case WM_ERASEBKGND:
        // do nothing, this avoids flash on a redraw
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, 0, 0, windoww, windowh, ddc, 0, 0, SRCCOPY);
        if (markMode != MM_NONE)
        {
            if (orect.left != -1 && orect.top != -1 && orect.right != -1 && orect.bottom != -1)
            {
                SelectObject(hdc, bluePen);
                MoveToEx(hdc, orect.left, orect.top, NULL);
                LineTo(hdc, orect.left, orect.bottom);
                LineTo(hdc, orect.right, orect.bottom);
                LineTo(hdc, orect.right, orect.top);
                LineTo(hdc, orect.left, orect.top);
            }

            SelectObject(hdc, saved ? greenPen : boxPen);
            MoveToEx(hdc, wrect.left, wrect.top, NULL);
            LineTo(hdc, wrect.left, wrect.bottom);
            LineTo(hdc, wrect.right, wrect.bottom);
            LineTo(hdc, wrect.right, wrect.top);
            LineTo(hdc, wrect.left, wrect.top);
        }
        EndPaint(hwnd, &ps);
        break;

    case WM_SYSCOMMAND:
        if (wparam == SC_CLOSE)
        {
            DestroyWindow(hwnd);
            return(0);
        }
        return(DefWindowProc(hwnd, msg, wparam, lparam));

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case MENU_PREV:
            PostMessage(hwnd, WM_PREV, 1, 0);
            break;

        case MENU_NEXT:
            PostMessage(hwnd, WM_NEXT, 1, 0);
            break;

        case MENU_SAVE:
            if ((fp = fopen(outfile, "a")) != NULL)
            {
                strncpy(localFile, curfile, RESOURCE_LEN);
                localFile[RESOURCE_LEN - 1] = 0;
                if ((cp = strchr(localFile, ';')) != NULL)
                {
                    *cp = 0;
                }
                cp = prefix(localFile, ".\\") ? &localFile[2] : localFile;
                if (markMode == MM_MARKED)
                {
                    fprintf(fp, "%s;%d,%d,%d,%d\n", cp, (int32) (irect.left + 0.5f), (int32) (irect.top + 0.5f),
                                    (int32) (irect.right - irect.left + 0.5f), (int32) (irect.bottom - irect.top + 0.5f));
                }
                else
                {
                    fprintf(fp, "%s\n", cp);
                }
                fclose(fp);
                saved = true;
                PostMessage(hwnd, WM_UPDATE, 0, 0);
            }
            break;

        case MENU_EXIT:
            PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
            break;

        }
        break;

    case WM_CHAR:
        if (curfile)
        {
            switch (wparam)
            {
            case 'p':
                PostMessage(hwnd, WM_PREV, fileCount != 0 ? fileCount : 1, 0);
                break;
            case 'P':
                PostMessage(hwnd, WM_PREV, 10, 0);
                break;
            case 'n':
            case ' ':
                PostMessage(hwnd, WM_NEXT, fileCount != 0 ? fileCount : 1, 0);
                break;
            case 'N':
                PostMessage(hwnd, WM_NEXT, 10, 0);
                break;
            case 's':
                PostMessage(hwnd, WM_COMMAND, MENU_SAVE, 0);
                break;
            case 'w':
                MatchWidth();
                PostMessage(main_window, WM_UPDATE, 0, 0);
                break;
            case 'h':
                MatchHeight();
                PostMessage(main_window, WM_UPDATE, 0, 0);
                break;
            case 'c':
                penIndex = penIndex + 1 & 3;
                PostMessage(main_window, WM_UPDATE, 0, 0);
                break;
            default:
                break;
            }

            if (isdigit((int) wparam))
            {
                fileCount = fileCount * 10 + (int) wparam - '0';
            }
            else
            {
                fileCount = 0;
            }
        }
        break;


    case WM_PREV:
        while (wparam--)
        {
            if (currentfile <= 1)
            {
                break;
            }
            prevCurr = currentfile--;
        }
        moveDir = -1;
        curfile = fileList[currentfile - 1];
        markMode = MM_NONE;
        PostMessage(hwnd, WM_OPENFILE, 0, 0);
        break;

    case WM_NEXT:
        while (wparam--)
        {
            if (currentfile >= fileList.size())
            {
                break;
            }
            prevCurr = currentfile++;
        }
        moveDir = 1;
        curfile = fileList[currentfile - 1];
        markMode = MM_NONE;
        PostMessage(hwnd, WM_OPENFILE, 0, 0);
        break;

    case WM_LBUTTONDOWN:
        mx = (int32) (int16) LOWORD(lparam);
        my = (int32) (int16) HIWORD(lparam);
        mouseDownX = mx;
        mouseDownY = my;
        dragOffsetX = 0;
        dragOffsetY = 0;
        saved = false;
        if (markMode == MM_NONE)
        {
            wrect.left = mx;
            wrect.right = mx;
            wrect.top = my;
            wrect.bottom = my;
            AdjustRects(true, true, true, true);
            markMode = MM_MOVERIGHT | MM_MOVEBOTTOM;
            SetCapture(hwnd);
        }
        else if (cursorMode != 0)
        {
            markMode = cursorMode;
            if (wparam & MK_SHIFT)
            {
                markMode = 0;
                if (cursorMode & (MM_MOVELEFT | MM_MOVERIGHT))
                {
                    markMode |= MM_DRAGHORIZ;
                }
                else if (cursorMode & (MM_MOVETOP | MM_MOVEBOTTOM))
                {
                    markMode |= MM_DRAGVERT;
                }
            }
            dragOffsetX = markMode & (MM_MOVELEFT | MM_DRAGHORIZ) ? mx - wrect.left : mx - wrect.right;
            dragOffsetY = markMode & (MM_MOVETOP | MM_DRAGVERT) ? my - wrect.top : my - wrect.bottom;
            SetCapture(hwnd);
        }
        else if (mx >= wrect.left && mx <= wrect.right && my >= wrect.top && my <= wrect.bottom)
        {
            markMode = MM_DRAGHORIZ | MM_DRAGVERT;
            dragOffsetX = mx - wrect.left;
            dragOffsetY = my - wrect.top;
            SetCapture(hwnd);
        }
        break;

    case WM_LBUTTONUP:
        markMode = (wrect.left != wrect.right && wrect.top != wrect.bottom) ? MM_MARKED : MM_NONE;
        ReleaseCapture();
        break;

    case WM_RBUTTONDOWN:
        if (markMode == MM_MARKED)
        {
            markMode = MM_NONE;
        }
        saved = false;
        break;

    case WM_MOUSEMOVE:
        mx = (int32) (int16) LOWORD(lparam);
        my = (int32) (int16) HIWORD(lparam);
        switch (markMode)
        {
        case MM_NONE:
            break;

        case MM_MARKED:
            cursorMode = 0;
            if (CloseTo(mx, wrect.right) && my >= wrect.top && my <= wrect.bottom)
            {
                cursorMode |= MM_MOVERIGHT;
            }
            else if (CloseTo(mx, wrect.left) && my >= wrect.top && my <= wrect.bottom)
            {
                cursorMode |= MM_MOVELEFT;
            }
            if (CloseTo(my, wrect.bottom) && mx >= wrect.left && mx <= wrect.right)
            {
                cursorMode |= MM_MOVEBOTTOM;
            }
            else if (CloseTo(my, wrect.top) && mx >= wrect.left && mx <= wrect.right)
            {
                cursorMode |= MM_MOVETOP;
            }
            switch (cursorMode)
            {
            case 0:
                SetCursor(arrowCursor);
                break;

            case MM_MOVELEFT:
            case MM_MOVERIGHT:
                SetCursor(weCursor);
                break;

            case MM_MOVETOP:
            case MM_MOVEBOTTOM:
                SetCursor(nsCursor);
                break;

            case MM_MOVELEFT | MM_MOVETOP:
            case MM_MOVERIGHT | MM_MOVEBOTTOM:
                SetCursor(nwseCursor);
                break;

            case MM_MOVELEFT | MM_MOVEBOTTOM:
            case MM_MOVERIGHT | MM_MOVETOP:
                SetCursor(neswCursor);
                break;
            }
            break;

        case MM_DRAGHORIZ:
        case MM_DRAGVERT:
        case MM_DRAGHORIZ | MM_DRAGVERT:
            dx = mx - mouseDownX;
            dy = my - mouseDownY;
            mouseDownX = mx;
            mouseDownY = my;
            if (dx < 0)
            {
                dx = max(dx, (int32) (brect.left - wrect.left));
            }
            else
            {
                dx = min(dx, (int32) (brect.right - wrect.right));
            }
            if (dy < 0)
            {
                dy = max(dy, (int32) (brect.top - wrect.top));
            }
            else
            {
                dy = min(dy, (int32) (brect.bottom - wrect.bottom));
            }
            if (markMode & MM_DRAGHORIZ)
            {
                wrect.left += dx;
                wrect.right += dx;
            }
            if (markMode & MM_DRAGVERT)
            {
                wrect.top += dy;
                wrect.bottom += dy;
            }
            // This shuld be a no-op.
            AdjustRects(true, true, true, true);
            PostMessage(main_window, WM_UPDATE, 0, 0);
            break;

        default:
            dx = mx - mouseDownX;
            dy = my - mouseDownY;

            if ((markMode & (MM_MOVELEFT | MM_MOVERIGHT)) != 0 && (markMode & (MM_MOVETOP | MM_MOVEBOTTOM)) != 0)
            {
                if ((float) abs(dx) > (float) abs(dy) * aspectRatio)
                {
                    dy = 0;
                }
                else
                {
                    dx = 0;
                }
            }
            if (markMode & MM_MOVELEFT)
            {
                wrect.left += dx;
            }
            else if (markMode & MM_MOVERIGHT)
            {
                wrect.right += dx;
            }
            if (markMode & MM_MOVETOP)
            {
                wrect.top += dy;
            }
            else if (markMode & MM_MOVEBOTTOM)
            {
                wrect.bottom += dy;
            }
            if ((markMode & (MM_MOVELEFT | MM_MOVERIGHT)) != 0 && (markMode & (MM_MOVETOP | MM_MOVEBOTTOM)) == 0)
            {
                prefHoriz = true;
            }
            else if ((markMode & (MM_MOVELEFT | MM_MOVERIGHT)) == 0 && (markMode & (MM_MOVETOP | MM_MOVEBOTTOM)) != 0)
            {
                prefHoriz = false;
            }
            else
            {
                prefHoriz = abs(dx) > abs(dy);
            }

            AdjustRects(prefHoriz, (markMode & MM_MOVELEFT) == 0, (markMode & MM_MOVETOP) == 0, true);
            if (markMode & MM_MOVELEFT)
            {
                mx = wrect.left + dragOffsetX;
            }
            if (markMode & MM_MOVERIGHT)
            {
                mx = wrect.right + dragOffsetX;
            }
            if (markMode & MM_MOVETOP)
            {
                my = wrect.top + dragOffsetY;
            }
            if (markMode & MM_MOVEBOTTOM)
            {
                my = wrect.bottom + dragOffsetY;
            }
            mouseDownX = mx;
            mouseDownY = my;
            pt.x = mx;
            pt.y = my;
            ClientToScreen(hwnd, &pt);
            SetCursorPos(pt.x, pt.y);

            PostMessage(main_window, WM_UPDATE, 0, 0);
            break;
        }
        break;

    case WM_OPENFILE:
        for (;;)
        {
            strncpy(localFile, curfile, RESOURCE_LEN);
            localFile[RESOURCE_LEN - 1] = 0;
            if ((cp = strchr(localFile, ';')) != NULL)
            {
                *cp++ = 0;
            }

            fetcher = NULL;
            resource = NULL;

            for (i = 0; i < numFetcher; i++)
            {
                fetcher = fetchers[i];
                if (prefixi(localFile, fetcher->Name()) && memcmp(&localFile[strlen(fetcher->Name())], "://", 3) == 0)
                {
                    if ((resource = fetcher->Prepare(localFile)) == NULL)
                    {
                        fetcher = NULL;
                        fetchData = NULL;
                        fetchAvail = 0;
                    }
                    else if ((fetchAvail = fetcher->Alldata(resource, &fetchData)) <= 0)
                    {
                        fetcher->Finish(resource);
                        free(resource);
                        resource = NULL;
                        fetcher = NULL;
                        fetchData = NULL;
                    }
                    break;
                }
            }
            if (i == numFetcher)
            {
                fetcher = fetchers[0];
                if ((resource = fetcher->Prepare(localFile)) == NULL)
                {
                    fetcher = NULL;
                    fetchData = NULL;
                    fetchAvail = 0;
                }
                else if ((fetchAvail = fetcher->Alldata(resource, &fetchData)) <= 0)
                {
                    fetcher->Finish(resource);
                    free(resource);
                    resource = NULL;
                    fetcher = NULL;
                    fetchData = NULL;
                }
            }

            w = SIZE;
            h = SIZE;
            memset(imagebuf, 0, SIZE * SIZE * 3);

            i = ERROR;
            if (fetchData != NULL)
            {
                if (fetchAvail >= 4 && memcmp(fetchData, JPEG_SIG, 3) == 0)
                {
                    for (j = 0; j < sizeof(JPEG_BYTE4); j++)
                    {
                        if (fetchData[3] == JPEG_BYTE4[j])
                        {
                            i = read_jpg((char *) fetchData, image, &w, &h, IMG_BUFFERED | IMG_BGR, fetchAvail);
                            break;
                        }
                    }
                }
                else if (fetchAvail >= 8 && memcmp(fetchData, PNG_SIG, 8) == 0)
                {
                    i = read_png((char *) fetchData, image, &w, &h, IMG_BUFFERED | IMG_BGR, fetchAvail);
                }
            }
            // Do we ever do these now?
            else if (suffixi(localFile, ".jpg"))
            {
                i = read_jpg(localFile, image, &w, &h, IMG_BGR);
            }
            else if (suffixi(localFile, ".png"))
            {
                i = read_png(localFile, image, &w, &h, IMG_BGR);
            }
            if (fetcher != NULL && resource != NULL)
            {
                fetcher->Finish(resource);
                free(resource);
            }
            if (i == OK)
            {
                break;
            }
            fileList.erase(fileList.begin() + currentfile - 1);
            if (fileList.size() == 0)
            {
                break;
            }
            if (moveDir == 1)
            {
                if (currentfile > (int32) fileList.size())
                {
                    currentfile--;
                }
            }
            else
            {
                if (currentfile > 1)
                {
                    currentfile--;
                }
            }
            curfile = fileList[currentfile - 1];
        }

        if (i != OK)
        {
            PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
            break;
        }

        imgw = w;
        imgh = h;

        cp = strrchr(localFile, '/');
        dp = strrchr(localFile, '\\');
        ep = strrchr(localFile, ':');

        if (dp == NULL || ep != NULL && ep > dp)
        {
            dp = ep;
        }
        if (cp == NULL || dp != NULL && dp > cp)
        {
            cp = dp;
        }
        if (cp == NULL)
        {
            cp = localFile;
        }
        else
        {
            cp++;
        }
        snprintf(title, 256, "%s  %s (%d/%d)", IMGName, cp, currentfile, (int32) fileList.size());
        SetWindowText(hwnd, title);
        PostMessage(hwnd, WM_RESIZEIMAGE, 0, 0);
        break;

    case WM_RESIZEIMAGE:
        img_resize(dimage, image, windoww, windowh, imgw, imgh, FILTER_BILINEAR, 0xff000000);

        SetupScale();

        memset(&bmi.bmiHeader, 0, sizeof(bmi.bmiHeader));

        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = windoww;
        bmi.bmiHeader.biHeight = windowh;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        SelectObject(ddc, odbm);
        for (i = 0; i < windowh; i++)
        {
            SetDIBits(ddc, dbm, windowh - i - 1, 1, dimage[i], &bmi, 0);
        }
        SelectObject(ddc, dbm);

        cp = strchr(curfile, ';');
        if (cp != NULL && sscanf(++cp, "%d,%d,%d,%d", &x, &y, &w, &h) == 4)
        {
            wrect.left = (int32) ((float) x * scale + 0.5f) + xoffset;
            wrect.right = (int32) ((float) (x + w) * scale + 0.5f) + xoffset;
            wrect.top = (int32) ((float) y * scale + 0.5f) + yoffset;
            wrect.bottom = (int32) ((float) (y + h) * scale + 0.5f) + yoffset;
            orect.left = wrect.left;
            orect.right = wrect.right;
            orect.top = wrect.top;
            orect.bottom = wrect.bottom;

            AdjustRects(true, true, true, true);
            markMode = MM_MARKED;
        }
        else
        {
            orect.left = -1;
            orect.right = -1;
            orect.top = -1;
            orect.bottom = -1;
        }
        saved = false;

        PostMessage(main_window, WM_UPDATE, 0, 0);
        break;

    default:
        return(DefWindowProc(hwnd, msg, wparam, lparam));
    }
    return(0);
}

int32 setupinstance(HINSTANCE inst, LPSTR cmd, int32 show)
{
    int32 x;
    int32 y;
    Fetcher *fetcher;
    WNDCLASS wc;
    HWND hwnd;
    char filename[RESOURCE_LEN];

    fetcher = fileFactory();
    if (fetcher->Init() == OK)
    {
        fetchers[numFetcher++] = fetcher;
    }
    hicon = LoadIcon(inst, "IMGIcon");

    aspectRatio = 16.0f / 9.0f;

    strncpy(filename, ".", RESOURCE_LEN);

    while (*(cmd = byp(cmd)) != 0)
    {
        if ((*cmd & 0xfd) == 0x2d)
        {
            switch (cmd[1] | 0x20)
            {
            case 'r':
                if (cmd[2] == ':')
                {
                    cmd++;
                }
                if (*(cmd = byp(&cmd[2])) == 0)
                {
                    break;
                }
                if (sscanf(cmd, "%d%*c%d", &x, &y) == 2 &&
                    x >= 1 && x <= SIZE &&
                    y >= 1 && y <= SIZE)
                {
                    aspectRatio = (float) x / (float) y;
                }
                cmd = unbyp(cmd);
                break;

            case 'o':
                if (cmd[2] == ':')
                {
                    cmd++;
                }
                if (*(cmd = byp(&cmd[2])) == 0)
                {
                    break;
                }
                cmd = cmdline(cmd, outfile, MAX_PATH - 2);
                break;

            default:
                cmd = unbyp(cmd);
                break;
            }
        }
        else
        {
            cmd = cmdline(cmd, filename, RESOURCE_LEN - 2);
        }
    }

    if (outfile[0] == 0)
    {
        return FALSE;
    }

    if (filename[0] == '@')
    {
        AddLine(filename);
    }
    else
    {
        Scanfiles(filename);
    }
    if (fileList.empty())
    {
        return FALSE;
    }
    currentfile = 1;
    prevCurr = 1;
    moveDir = 1;
    curfile = fileList[0];

    arrowCursor = LoadCursor(NULL, IDC_ARROW);
    weCursor = LoadCursor(NULL, IDC_SIZEWE);
    nsCursor = LoadCursor(NULL, IDC_SIZENS);
    nwseCursor = LoadCursor(NULL, IDC_SIZENWSE);
    neswCursor = LoadCursor(NULL, IDC_SIZENESW);

    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = inst;
    wc.hIcon = hicon;
    wc.hCursor = arrowCursor;
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = "IMGMenu";
    wc.lpszClassName = IMGClass;
    if (RegisterClass(&wc) == 0)
    {
        return FALSE;
    }

    hwnd = CreateWindow(IMGClass, IMGName,
                                        WS_OVERLAPPEDWINDOW,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        NULL, NULL, inst, NULL);

    if (hwnd == NULL)
    {
        return FALSE;
    }

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    PostMessage(hwnd, WM_OPENFILE, 0, 0);

    return(TRUE);
}

int32 WINAPI WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd, int32 show)
{
    MSG msg;

    if (setupinstance(inst, cmd, show) == 0)
    {
        return(FALSE);
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return((int32) msg.wParam);
}
