/*
 * imageget.h
 */

#pragma once

#include <windows.h>

#ifndef RC_INVOKED
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <clib.h>
#include <image.h>
#endif

#include "types.h"

#define         MENU_EXIT               100
#define         MENU_PREV               101
#define         MENU_NEXT               102
#define         MENU_SAVE               103

#define         WM_UPDATE               (WM_USER + 0)
#define         WM_OPENFILE             (WM_USER + 1)
#define         WM_RESIZEIMAGE          (WM_USER + 2)
#define         WM_PREV                 (WM_USER + 3)
#define         WM_NEXT                 (WM_USER + 4)

#define         SIZE                    16384

typedef struct frect
{
    float left;
    float top;
    float right;
    float bottom;
} FRECT;

typedef char            int8;
typedef unsigned char   uint8;
typedef short           int16;
typedef unsigned short  uint16;
typedef int             int32;
typedef unsigned int    uint32;
