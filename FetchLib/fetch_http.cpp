#include        "fetchlib.h"
//#include        <curl/curl.h>

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

struct HttpResource : public Resource
{
    uint8 *m_data;
    uint32 m_size;
    uint32 m_pos;
};

char curlErrorMessage[CURL_ERROR_SIZE];

/* curl calls this routine to get more data */
static size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp)
{
    uint8 *newbuff;

    HttpResource *httpResource = (HttpResource *) userp;
    size *= nitems;

    size_t rembuff = httpResource->m_size - httpResource->m_pos;

    if (size > rembuff)
    {
        /* not enough space in buffer */
        newbuff = (uint8 *) realloc(httpResource->m_data, httpResource->m_size + (size - rembuff));
        if (newbuff == NULL)
        {
            size = rembuff;
        }
        else
        {
            /* realloc succeeded increase buffer size */
            httpResource->m_size += (uint32) (size - rembuff);
            httpResource->m_data = newbuff;
        }
    }

    if (httpResource->m_data != NULL)
    {
        memcpy(&httpResource->m_data[httpResource->m_pos], buffer, size);
        httpResource->m_pos += (uint32) size;
    }

    return size;
}

static int32 http_init()
{
    return wsawrapper.ok() ? OK : ERROR;
}

static Resource *http_prepare(char const *name)
{
    HttpResource *httpResource;

    if (!prefixi(name, "http://") && !prefixi(name, "https://"))
    {
        return NULL;
    }

    httpResource = (HttpResource *) malloc(sizeof(HttpResource));
    if (httpResource == NULL)
    {
        return NULL;
    }
    httpResource->m_data = NULL;
    httpResource->m_size = 0;
    httpResource->m_pos = 0;


    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        free(httpResource);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, name);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpResource);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorMessage);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 8);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1000);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        free(httpResource->m_data);
        free(httpResource);
        httpResource = NULL;
    }
    else
    {
        httpResource->m_size = httpResource->m_pos;
        httpResource->m_pos = 0;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    return httpResource;
}

static int32 http_getc(Resource *resource)
{
    HttpResource *httpResource;

    if (resource == NULL)
    {
        return EOF;
    }
    httpResource = (HttpResource *) resource;
    if (httpResource->m_data == NULL)
    {
        return EOF;
    }
    return httpResource->m_pos < httpResource->m_size ? httpResource->m_data[httpResource->m_pos++] : EOF;
}

static int32 http_gets(Resource *resource, char *buff, int32 size)
{
    if (resource == NULL || buff == NULL)
    {
        return EOF;
    }
    int32 ch;
    int32 i = 0;
    while ((ch = http_getc(resource)) != EOF)
    {
        if (i < size - 1)
        {
            buff[i] = ch;
        }
        if (ch != '\r' && ch != '\n' && i < size - 1)
        {
            i++;
        }
        if (ch == '\n')
        {
            break;
        }
    }
    buff[i] = 0;
    return ch == EOF && i == 0 ? EOF : i;
}

static int32 http_read(Resource *resource, uint8 *buff, int32 size)
{
    HttpResource *httpResource;

    httpResource = (HttpResource *) resource;
    if (resource == NULL || buff == NULL)
    {
        return EOF;
    }
    int32 ch;
    int32 i = 0;
    while (i < size && (ch = http_getc(resource)) != EOF)
    {
        buff[i++] = ch;
    }
    return ch == EOF && i == 0 ? EOF : i;
}

static uint32 http_alldataWorker(Resource *resource, uint8 **buffPtr, bool acquire)
{
    HttpResource *httpResource;

    if (resource == NULL)
    {
        return 0;
    }
    httpResource = (HttpResource *) resource;

    uint32 const size = httpResource->m_size;
    if (buffPtr != NULL)
    {
        *buffPtr = httpResource->m_data;
        if (acquire)
        {
            httpResource->m_data = NULL;
            httpResource->m_size = 0;
            httpResource->m_pos = 0;
        }
    }
    return size;
}

static uint32 http_alldata(Resource *resource, uint8 **buffPtr)
{
    return http_alldataWorker(resource, buffPtr, false);
}

static uint32 http_alldataAcquire(Resource *resource, uint8 **buffPtr)
{
    return http_alldataWorker(resource, buffPtr, true);
}

static void http_finish(Resource *resource)
{
    HttpResource *httpResource;

    httpResource = (HttpResource *) resource;
    if (resource == NULL)
    {
        return;
    }

    if (httpResource->m_data != NULL)
    {
        free(httpResource->m_data);
        httpResource->m_data = NULL;
    }
}

void http_shutdown()
{
}

static Fetcher http_fetcher =
{
    "http",
    http_init,
    http_prepare,
    http_gets,
    http_alldata,
    http_alldataAcquire,
    http_finish,
    http_shutdown
};

Fetcher *httpFactory()
{
    return &http_fetcher;
}

static Fetcher https_fetcher =
{
    "https",
    http_init,
    http_prepare,
    http_gets,
    http_alldata,
    http_alldataAcquire,
    http_finish,
    http_shutdown
};

Fetcher *httpsFactory()
{
    return &https_fetcher;
}
