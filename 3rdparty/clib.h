/*
 * clib.h - declarations for code in clib.lib
 *
 * (C) Copyright 1997, David Goodenough. All rights reserved
 */

#pragma once

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif
#ifndef OK
#define OK      0
#endif
#ifndef ERROR
#define ERROR   (-1)
#endif

#ifndef __cplusplus
#ifdef  _MAKE_CLIB_
#include        <winsock2.h>
#endif
#endif
#include        <windows.h>

#ifdef __cplusplus
#include <string>

extern "C" {
#endif

// much stuff gets tossed when windows wasn't preincluded
struct _scnfile_
{
    struct _scnfile_ *_next;
    char _filename[260];
#ifdef _WINDOWS_
    WIN32_FIND_DATA _finddata;
#endif
};

struct _scnfile_ * scnwld(char const *wildcard, int flags);
void scnwldcb(char const *wildcard, int flags, int (*func)(char const *));
struct _scnfile_ * scnfile(struct _scnfile_ *files, char *buffer, void *finddata);

char *cmdline(char *cmd, char *arg, int size);
void cmdargs(char *cmd, void (* option)(char *), void (* param)(char *));

void addenv(char *env, char *str);
char *findenv(char *env, char *str);
char *enumenv(char *env, int which);
char *tokenize(char *str, char *token, int size, int delim);
char *skiptoken(char *str, int delim, int count);
#ifdef _WINDOWS_
void setcliptext(char *s, HWND w);
char *getcliptext(HWND w, char *buff, int size);
#endif
void *beginthread(void(__cdecl *proc)(void *), void *param);
char *findinpath(char *file, char *path, char *buff);
void regupd(int (* infunc)(char *, int));
int get_version();
#ifdef _WINDOWS_
void xlatvk(HWND h, unsigned m, WPARAM w, LPARAM l);
void pcopy(void *ofp, void *ifp, HWND hwnd, HANDLE haccel);
#endif
char *filefromfile(char *file, int n);
int driveready(int drive);
void logprintf(void (*pfunc)(char *), char *fmt, ...);
char *strrstr(char *haystack, char const *needle);

#ifdef _WINDOWS_
#define     XKEY_SHIFT          1
#define     XKEY_CTRL           2
#define     XKEY_ALT            4
#define     WM_VKCHAR           0x0109

int wsastartup(WORD wVersionRequested);
SOCKET socks_open(unsigned long socks_addr, unsigned short socks_port,
                                        unsigned long targ_addr,
                                        unsigned short targ_port, int ver);
#endif

unsigned int crc16_init();
unsigned int crc16_byte(unsigned int crc, int ch);
unsigned int crc16_buff(unsigned int crc, void *buffer, int size);
unsigned int crc16_post(unsigned int crc);

unsigned int crcccitt_init();
unsigned int crcccitt_byte(unsigned int crc, int ch);
unsigned int crcccitt_buff(unsigned int crc, void *buffer, int size);
unsigned int crcccitt_post(unsigned int crc);

unsigned int crc32_init();
unsigned int crc32_byte(unsigned int crc, int ch);
unsigned int crc32_buff(unsigned int crc, void *buffer, int size);
unsigned int crc32_post(unsigned int crc);

char *byp(char *str);
char *unbyp(char *str);
char *bypa(char *str);
char *unbypa(char *str);                     // untested
char *bypn(char *str);                       // untested
char *unbypn(char *str);                     // untested
char *bypan(char *str);                      // untested
char *unbypan(char *str);                    // untested
char *strpos(char *str, int ch, int count);
void bzero(void *buff, int count);
int atox(char *str);
char *strip(char *str);
char *neaten(char *str);
char *spccr(char *str);
char *rmspc(char *str);
char *detab(char *str, int width);
char *chrcat(char *str, int ch);
int chrcnt(char *str, int ch);
char *lower(char *str);
char *upper(char *str);
char *strend(char *str);
char *strfmt(char *buff, int size, int val);
unsigned int clocktick();
int dot(int ch);
int dotpc(int ch);
int dow(int day, int month, int year);
int doy(int day, int month, int year);
int isleap(int year);
int moonp(int day, int month, int year);     // untested
int easter(int year);                        // untested
int sgn(int val);
int prefix(char const *main, char const *pref);
int prefixi(char const *main, char const *pref);
int suffix(char const *main, char const *suff);
int suffixi(char const *main, char const *suff);
char *stristr(char *haystack, char const *needle);
char *strichr(char *haystack, int needle);
char *strristr(char *haystack, char const *needle);
char *strrichr(char *haystack, int needle);
int sextw(int word);
int sextb(int byte);
int zextw(int word);
int zextb(int byte);
unsigned int gcd(unsigned int a, unsigned int b);
int putd(int x, void *fp);
int getd(void *fp);
int mkdirr(char const *path);
char *crypt(char *str);
void hexdump(char *buff, int size, int addr, void (* output)(int ch));
void permute(int count, void (* callback)(int *data));
void line(int x1, int y1, int x2, int y2, void (* callback)(int x, int y));
void circle(int x, int y, int r, void (* callback)(int x, int y));
void rgb_to_hsv(int r, int g, int b, int *hp, int *sp, int *vp);
int isbetween(int value, int low, int high, int modulus);
void ipandport(char *data, unsigned long *paddr, unsigned short *pport);
#ifdef _WINDOWS_
unsigned int st2jsec(SYSTEMTIME *st);
void jsec2st(SYSTEMTIME *st, unsigned int jsec);
#endif

unsigned __int64 muldiv64(unsigned __int64 a, unsigned __int64 b, unsigned __int64 c);
unsigned int getTimeMS();
unsigned int getTimeUS();

void screen_off();
void screen_on();

#define mk_uint64(lo, hi) ((unsigned __int64) (hi) << (unsigned __int64) 32) | \\
                                        (unsigned __int64) (lo))

/*
 * debug malloc/free
 */
#ifdef  DEBUG
#ifdef  _WIN64
typedef unsigned __int64    size_t_;
#else
typedef unsigned int        size_t_;
#endif
void mem_init(int mode, void (*output)(int, char *));
void mem_end();
void *mem_malloc(size_t_ size, char *file, int line);
void *mem_realloc(void *ptr, size_t_ size, char *file, int line);
void mem_free(void *ptr, char *file, int line);
char *mem_strdup(char *str, char *file, int line);
#define malloc_(s) mem_malloc((s), __FILE__, __LINE__)
#define realloc_(p, s) mem_realloc((p), (s), __FILE__, __LINE__)
#define free_(p) mem_free((p), __FILE__, __LINE__)
#define strdup_(s) mem_strdup((s), __FILE__, __LINE__)
#else
#define mem_init(x, y)  /* */
#define mem_end()       /* */
#define malloc_(s)      malloc(s)
#define realloc_(p, s)  realloc((p), (s))
#define free_(p)        free(p)
#define strdup_(s)      strdup(s)
#endif

#ifdef DEBUG_MALLOC
void md_init(int mode, void(*output)(int, char *));
void md_end();
void md_malloc(void *data, size_t size, char *file, int line);
void md_free(void *ptr, char *file, int line);
void md_realloc(void *oldptr, void *newptr, size_t size, char *file, int line);
void md_strdup(char *str, char *file, int line);
#else
#define md_init(mode, output)                           /* */
#define md_end()                                        /* */
#define md_malloc(data, size, file, line)               /* */
#define md_free(ptr, file, line)                        /* */
#define md_realloc(oldptr, newptr, size, file, line)    /* */
#define md_strdup(str, file, line)                      /* */
#endif

/*
 * debug fopen/fclose
 */
#ifdef _FILE_DEFINED
#ifdef  DEBUG
void fil_init(int mode, void (*output)(int, char *));
void fil_end();
FILE *fil_fopen(char *file, char *mode, char *source, int line);
int fil_fclose(FILE *file, char *source, int line);
#define fopen_(f, m)        fil_fopen((f), (m), __FILE__, __LINE__)
#define fclose_(f)          fil_fclose((f), __FILE__, __LINE__)
#else
#define fil_init(x, y)      /* */
#define fil_end()           /* */
#define fopen_(f, m)        fopen((f), (m))
#define fclose_(f)          fclose(f)
#endif
#endif

/*
 * debug socket/accept/closesocket
 */
#ifdef _WINSOCKAPI_
#ifdef  DEBUG
void sock_init(int mode, void (*output)(int, char *));
void sock_end();
SOCKET sock_socket(int af, int type, int protocol, char *source, int line);
SOCKET sock_accept(SOCKET sock, void *a, void *l, char *source, int line);
int sock_closesocket(SOCKET socket, char *source, int line);
#define socket_(a, t, p)    sock_socket((a), (t), (p), __FILE__, __LINE__)
#define accept_(s, a, l)    sock_accept((SOCKET) (s), (a), (l), __FILE__, __LINE__)
#define closesocket_(s)     sock_closesocket((int) (s), __FILE__, __LINE__)
#else
#define sock_init(x, y)     /* */
#define sock_end()          /* */
#define socket_(a, t, p)    socket((a), (t), (p))
#define accept_(s, a, l)    accept((s), (a), (l))
#define closesocket_(s)     closesocket(s)
#endif
#endif

/*
 * linked list stuff, also in debug versions
 */
void *lapp(void *list, void *item);
void *ldel(void *list, void *item);
int lcount(void *list);
void *lrev(void *list);
void *lgetitem(void *list, int item);
void *lsort(void *list, int (*cmp)(void *, void *));
void *lsortins(void *list, void *item, int (*cmp)(void *, void *));
void *lconcat(void *list1, void *list2);
void *lshuffle(void *list);
void *lnext(void *list);
void *lnext_deb(void *list);
void *lfree(void *list);
void *lfree_deb(void *list);
#ifdef DEBUG
#define lnext_ lnext_deb
#define lfree_ lfree_deb
#else
#define lnext_ lnext
#define lfree_ lfree
#endif

 /*
 * sha1 hashing stuff
 */
#define         SHA_DIGESTSIZE          20
void shaHash(unsigned char *result, unsigned char *buffer, int count);
void shaInitial();
void shaUpdate(unsigned char *buffer, int count);
void shaFinal(unsigned char *result);
void shaFile(unsigned char *result, void *fp);

/*
* sha256 hashing stuff
*/
#define         SHA256_DIGESTSIZE       32
void sha256Hash(unsigned char *result, unsigned char *buffer, int count);
void sha256Initial();
void sha256Update(unsigned char *buffer, int count);
void sha256Final(unsigned char *result);
void sha256File(unsigned char *result, void *fp);

/*
* sha512 hashing stuff
*/
#define         SHA512_DIGESTSIZE       64
void sha512Hash(unsigned char *result, unsigned char *buffer, int count);
void sha512Initial();
void sha512Update(unsigned char *buffer, int count);
void sha512Final(unsigned char *result);
void sha512File(unsigned char *result, void *fp);

/*
* blake2 hashing stuff
*/
#define         BLAKE2_DIGESTSIZE       64
void blake2Hash(unsigned char *result, int outCount, unsigned char *buffer, int count);
void blake2Initial(int outCount);
void blake2Update(unsigned char *buffer, int count);
void blake2Final(unsigned char *result);
void blake2File(unsigned char *result, int outCount, void *fp);

/*
 * md5 hashing stuff
 */
#define         MD5_DIGESTSIZE           16
void md5Hash(unsigned char *result, unsigned char *buffer, unsigned int count);
void md5Initial();
void md5Update(unsigned char *buffer, unsigned int count);
void md5Final(unsigned char *result);
void md5File(unsigned char *result, void *fp);

/*
 * safer symmetric encryption stuff
 */
#define SAFER_BLOCKSIZE     8       /* 64 bits */
#define SAFER_KEYSIZE       16      /* 128 bits */
#define SAFER_KEYLEN        (SAFER_BLOCKSIZE * (1 + 2 * 10)) /* internal key length */
void saferExpandKey(unsigned char *key, unsigned char *userKey);
void saferEncryptBlock(unsigned char *data, unsigned char *key);
void saferDecryptBlock(unsigned char *data, unsigned char *key);

// Read and write config data
#define CONFIG_PORTABLE     1
#define CONFIG_APPDATA      2
#define CONFIG_BESTGUESS    3

int ConfigInit(int preferredMode);
void ConfigSelectSection(char const *section);
void ConfigReadString(char const *key, char *buffer, unsigned int size);
int ConfigReadInt(char const *key, int def);
void ConfigWriteString(char const *key, char const *buffer);
void ConfigWriteInt(char const *key, int value);
void ConfigDelete(char const *key);
void ConfigClearSection(int full);
void ConfigFlush();
void ConfigEnd();

/*
 * number of ticks reported by clocktick() per day
 */
#define         TICKS_PER_DAY           (24l * 60l * 60l * 1000l)

/*
 * dialog style window (i.e. no size / maximize capability)
 */
#define         WS_DIALOG               (WS_OVERLAPPED | WS_CAPTION | \
                                        WS_SYSMENU | WS_MINIMIZEBOX)

/*
 * delimiter value for tokenize to make it do quoted files right
 */
#define         TOKEN_FILENAME          0x0101

/*
 * day and month strings
 */
extern char *shortMons[];
extern char *shortDays[];
extern char *longMons[];
extern char *longDays[];

/*
 * hex chars
 */
extern char hexchars[];

/*
 * stuff for scnwld
 */
#define         SCN_ARCHIVE             1
#define         SCN_HIDDEN              2
#define         SCN_READONLY            4
#define         SCN_SYSTEM              8
#define         SCN_DIRECTORY           16
#define         SCN_RECURSIVE           32

/*
 * random number generation - see also class Random below
 */
void srand32(unsigned int seed);
unsigned int rand32();
unsigned __int64 rand64();
unsigned int rnd32(unsigned int n);
unsigned __int64 rnd64(unsigned __int64 n);
float rndf32();

/*
 * for dialog static items
 */
#define         IDSTATIC                (-1)

/*
 * macros to stringify a value, and stack the deck for pragma messages
 */
#define         _T_T_(X)                #X
#define         _S_S_(X)                _T_T_(X)
#define         __LOC__                 __FILE__ "(" _S_S_(__LINE__) ") : "

#if     defined(NONOISE)
#define         TODO(Z)                 comment(user, Z)
#define         NOTE(Z)                 comment(user, Z)
#elif   defined(ERRNOISE)
#define         TODO(Z)                 message(__LOC__ "warning C4998: " Z)
#define         NOTE(Z)                 message(__LOC__ "warning C4997: " Z)
#else
#define         TODO(Z)                 message(__LOC__ "TODO: " Z)
#define         NOTE(Z)                 message(__LOC__ "NOTE: " Z)
#endif
#define         WARN(Z)                 message(__LOC__ "warning C4999: " Z)
#define         ERR(Z)                  message(__LOC__ "error C2999: " Z)

/*
 * Lets make DLL's easy
 */
#define         DLLENTRY                __declspec(dllexport)

/*
 * Easy count of array size
 */
#define         NUM_ELEMENT(x)          (sizeof(x) / sizeof(x[0]))

/*
 * Offset of a member of a structure
 */
#define         MEMBER_OFFSET(struc, member)     ((int) (INT_PTR) (&(((struc *)(0))->member)))

/*
 * windowclass safe version of GetSysColorBrush
 */

#define         GetSysColorBgrBrush(x)  ((HBRUSH) ((x) + 1))

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

char const *byp(char const *str);
char const *unbyp(char const *str);
char const *bypa(char const *str);
char const *unbypa(char const *str);                     // untested
char const *bypn(char const *str);                       // untested
char const *unbypn(char const *str);                     // untested
char const *bypan(char const *str);                      // untested
char const *unbypan(char const *str);                    // untested
char const *strichr(char const *main, int search);
char const *stristr(char const *main, char const *search);
char const *strristr(char const *main, char const *search);
char const *strrichr(char const *main, int search);
char const *strrstr(char const *main, char const *search);
char const *strend(char const *str);
char const *strpos(char const *str, int ch, int count);
char const *cmdline(char const *cmd, char *arg, int size);
void cmdargs(char const *cmd, void(*option)(char *), void(*param)(char *));
void scnwld(char const *wildcard, int flags, int (*func)(char const *));

void ConfigSelectSection(std::string const &section);
std::string ConfigReadString(std::string const &key, std::string const &def);
int ConfigReadInt(std::string const &key, int def);
void ConfigWriteString(std::string const &key, std::string const &buffer);
void ConfigWriteInt(std::string const &key, int value);
void ConfigDelete(std::string const &key);
void ConfigClearSection(bool full = false);

class Random
{
public:
    Random();
    Random(unsigned int seed);
    Random(unsigned int *seeddata, unsigned int seedsize);

    void Srand(unsigned int seed);
    void Srand(unsigned int *seeddata, unsigned int seedsize);
    unsigned int Rand();
    unsigned __int64 Rand64();
    unsigned int Rnd(unsigned int n);
    unsigned __int64 Rnd64(unsigned __int64 n);
    float Rndf();

private:
    unsigned __int64 m_seed[3];
    unsigned int m_rotate;

    void    SetSeed(unsigned int *seeddata, unsigned int seedsize);
    void    SetRandomSeed();
};

#include <functional>

template <typename T>
class RAII
{
public:
    RAII() = delete;
    RAII(RAII const &) = delete;
    RAII& operator=(RAII const &) = delete;
    RAII(RAII &&) = delete;
    RAII& operator=(RAII&&) = delete;

    RAII(T object, std::function<void(T)> destructor) :
    object_(object),
    destructor_(destructor),
    valid_(true)
    {
    }

    ~RAII()
    {
        if (valid_)
        {
            destructor_(object_);
        }
    }

    T Release()
    {
        valid_ = false;
        return object_;
    }

    void Destroy()
    {
        if (valid_)
        {
            destructor_(object_);
        }
        valid_ = false;
    }

private:
    T object_;
    std::function<void(T)> destructor_;
    bool valid_;
};

#endif

#ifdef __cplusplus

// Compile time julian date and time onversion
//int constexpr JCompileTime()
//{
//    char const ma = __DATE__[1] << 4 & 0x0f | __DATE__[2] & 0x0f;
//    return 0;
//}

#endif

/*
 * pragma to link in the library
 */
#ifdef _WIN64
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "clibd.lib")
#else
#pragma comment(lib, "clib.lib")
#endif
#endif
