/*
 * image.h - declarations for routines in image.dll
 *
 * (C) Copyright 1999, David Goodenough. All rights reserved
 */

#pragma once

#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OK
#define OK      0
#endif
#ifndef ERROR
#define ERROR   (-1)
#endif

#define IMG_RGB             0
#define IMG_BGR             1
#define IMG_CFILL           2
#define IMG_BUFFERED        4
#define IMG_LINEAR          8
#define IMG_ALPHA           16
#define IMG_RGBA            (IMG_RGB | IMG_ALPHA)
#define IMG_BGRA            (IMG_BGR | IMG_ALPHA)

// All functions return OK or ERROR

int __cdecl read_bpg(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags, ...);
int __cdecl read_jpg(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags, ...);
int __cdecl read_png(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags, ...);
int __cdecl read_bmp(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags, ...);
int __cdecl read_gif(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags);
int __cdecl read_tif(char const *the_file, unsigned char **obuf, int *wp, int *hp, int flags);
int __cdecl write_bpg(char const *the_file, unsigned char **ibuf, int w, int h, int q, int s);
int __cdecl write_jpg(char const *the_file, unsigned char **ibuf, int w, int h, int q);
int __cdecl write_png(char const *the_file, unsigned char **ibuf, int w, int h);

#define FILTER_LANCZOS      0
#define FILTER_HAMMING      1
#define FILTER_BOX          2
#define FILTER_BILINEAR     3
#define FILTER_GAUSSIAN     4
#define FILTER_BLACKMAN     5

int __cdecl img_resize(unsigned char **dest, unsigned char **src, int dw, int dh, int sw, int sh, unsigned int filter, unsigned int fill);

#ifdef __cplusplus
}
#endif

/*
 * pragma to link in the library
 */
#ifndef _M_IX86
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "imaged.lib")
#else
#pragma comment(lib, "image.lib")
#endif
#endif
