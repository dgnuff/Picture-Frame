#include "fetchlib.h"

struct FileResource : public Resource
{
    FILE *m_fp;
    uint8 *m_data;
    int32 m_size;
};

static int32 file_init()
{
    return OK;
}

static Resource *file_prepare(char const *name)
{
    char const *filename;
    FILE *fp;
    FileResource *fileResource;

    if (prefixi(filename = name, "file://"))
    {
        filename = &filename[7];
    }
    if ((fp = fopen(filename, "rb")) == NULL)
    {
        return NULL;
    }
    fileResource = (FileResource *) malloc(sizeof(FileResource));
    if (fileResource != NULL)
    {
        fileResource->m_fp = fp;
        fileResource->m_data = NULL;
        fileResource->m_size = 0;
    }
    else
    {
        fclose(fp);
    }
    return fileResource;
}

static int32 file_gets(Resource *resource, char *buff, int32 size)
{
    int32 i;
    int32 ch;
    FileResource *fileResource;
    FILE *fp;

    if (resource == NULL || buff == NULL)
    {
        return EOF;
    }
    fileResource = (FileResource *) resource;
    fp = fileResource->m_fp;
    for (i = 0; i < size - 1; i++)
    {
        if ((ch = getc(fp)) == EOF)
        {
            buff[i] = 0;
            if (i > 0 && buff[i - 1] == '\r')
            {
                buff[--i] = 0;
            }
            return(i == 0 ? EOF : i);
        }
        buff[i] = ch;
        if (ch == '\n')
        {
            if (i)
            {
                break;
            }
            i--;
        }
    }
    buff[i] = 0;
    if (i > 0 && buff[i - 1] == '\r')
    {
        buff[--i] = 0;
    }
    return i;
}

static int32 file_alldataWorker(Resource *resource, uint8 **buffPtr, bool acquire)
{
    uint32 size;
    FileResource *fileResource;
    FILE *fp;

    if (resource == NULL)
    {
        return 0;
    }
    fileResource = (FileResource *) resource;
    if (fileResource->m_data == NULL)
    {
        fp = fileResource->m_fp;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fileResource->m_data = (uint8 *) malloc(size);
        if (fileResource->m_data != NULL)
        {
            if ((fileResource->m_size = (int32) fread(fileResource->m_data, 1, size, fp)) < 0)
            {
                free(fileResource->m_data);
                fileResource->m_data = NULL;
                fileResource->m_size = 0;
            }
        }
    }

    size = fileResource->m_size;
    if (buffPtr != NULL)
    {
        *buffPtr = fileResource->m_data;
        if (acquire)
        {
            fileResource->m_data = NULL;
            fileResource->m_size = 0;
        }
    }
    return size;
}

static uint32 file_alldata(Resource *resource, uint8 **buffPtr)
{
    return file_alldataWorker(resource, buffPtr, false);
}

static uint32 file_alldataAcquire(Resource *resource, uint8 **buffPtr)
{
    return file_alldataWorker(resource, buffPtr, true);
}

static void file_finish(Resource *resource)
{
    if (resource == NULL)
    {
        return;
    }

    FileResource *fileResource = (FileResource *) resource;
    if (fileResource->m_fp != NULL)
    {
        fclose(fileResource->m_fp);
        fileResource->m_fp = NULL;
    }
    if (fileResource->m_data)
    {
        free(fileResource->m_data);
        fileResource->m_data = NULL;
    }
}

static void file_shutdown()
{
}

static Fetcher file_fetcher =
{
    "file",
    file_init,
    file_prepare,
    file_gets,
    file_alldata,
    file_alldataAcquire,
    file_finish,
    file_shutdown
};

Fetcher *fileFactory()
{
    return &file_fetcher;
}
