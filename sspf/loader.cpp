#include "pch.h"

#include "sspf.h"
#include "fetchlib.h"
#include "peffect.h"
#include "veffect.h"
#include "util.h"
#include "loader.h"

using std::atomic;
using std::back_inserter;
using std::hash;
using std::map;
using std::max;
using std::queue;
using std::string;
using std::transform;
using std::unordered_map;
using std::vector;

namespace Loader
{

class Job
{
public:
    Job();
    Job(Context *context, uint32 x, uint32 y, bool dontWait, bool showBlack);
    virtual ~Job();

    Context *GetContext();
    uint32  GetX() const;
    uint32  GetY() const;
    bool    GetDontWait() const;
    bool    GetShowBlack() const;

private:
    Context *m_context;
    uint32  m_x;
    uint32  m_y;
    bool    m_dontWait;
    bool    m_showBlack;
};

atomic<bool>            n_terminateThread = false;
HANDLE                  n_hEvent = NULL;
HANDLE                  n_hThread = NULL;
HWND                    n_hWnd = NULL;

vector<ImageResource>   n_resources;
vector<uint32>          n_aspects;

unordered_map<string, map<uint32, string>>  n_resourceAssembly;

uint32                  n_queueMin;
uint32                  n_queueMax;
queue<Texture>          n_textureQueue;
threadSafeQueue<Job>    n_jobQueue;

vector<Fetcher *>       n_fetchers;

shuffle<uint32>         n_imageShuffle(0);
shuffle<uint32>         n_effectShuffle(0);

void                    (* n_setEffectCallback)(Context &context, PEffect *peffect, VEffect *veffect) = NULL;
void                    (* n_textureLoadCallback)(Context &context, Texture &texture) = NULL;

uint32                  n_ffColour = 0;
uint8                   n_dummyTexture[64 * 4];

void    SubmitJob(Context *context, uint32 x, uint32 y, bool dontWait, bool showBlack)
{
    Job const job(context, x, y, dontWait, showBlack);
    n_jobQueue.enqueue(job);
    SetEvent(n_hEvent);
}

void    InitFetchers()
{
    Fetcher *fetcher;

    fetcher = fileFactory();
    if (fetcher->Init() == OK)
    {
        n_fetchers.push_back(fetcher);
    }
}

void    ShutdownFetchers()
{
    for (auto fetcher : n_fetchers)
    {
        fetcher->Shutdown();
    }
}

Fetcher *FindFetcher(char const *resource, bool exact)
{
    for (auto fetcher : n_fetchers)
    {
        char const * const name = fetcher->Name();
        if (prefixi(resource, name) && prefixi(&resource[strlen(name)], "://"))
        {
            return fetcher;
        }
    }
    // This relies on the file fetcher being in slot zero.
    return exact ? NULL : n_fetchers[0];
}

void    SetHWnd(HWND hWnd)
{
    n_hWnd = hWnd;
}

void    LoadTexture()
{
    Fetcher *fetcher = NULL;
    Resource *resource = NULL;
    uint32 size;
    uint8 *data;

    uint32 retries = 0;
    bool ok = false;

    char resourceName[RESOURCE_LEN];
    int32 const resourceNum = n_imageShuffle.next_item();
    ImageResource const &imageResource = n_resources[resourceNum];
    strncpy(resourceName, imageResource.m_filename.c_str(), RESOURCE_LEN);
    resourceName[RESOURCE_LEN - 1] = 0;

    while (!ok && retries < 4)
    {
        fetcher = FindFetcher(resourceName, false);
        if (fetcher != NULL)
        {
            if ((resource = fetcher->Prepare(resourceName)) == NULL)
            {
                fetcher = NULL;
                logFile.Report("fetcher->Prepare(%s) failed", resourceName);
            }
            else if ((size = fetcher->AlldataAcquire(resource, &data)) == 0)
            {
                logFile.Report("fetcher->AlldataAcquire() for %s returned zero", resourceName);
            }
            else
            {
                ok = true;
            }
        }
        retries++;
    }

    if (!ok)
    {
        return;
    }

    int texx = 0;
    int texy = 0;
    uint8 *buffer = NULL;
    uint8 *image[1];

    // This is a little gratuitous.  I decode the image here, but then promptly throw the results away.  This is
    // done just to verify that we can actually decode a valid image from the buffer before we make a copy of the
    // buffer and put it in the queue.
    if (Util::GetJPGDimensions(data, size, &texx, &texy) && (buffer = (uint8 *) malloc(texx * texy * 3)) != NULL)
    {
        image[0] = buffer;
        ok = read_jpg((char *) data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR, size) == OK;
    }
    else if (Util::GetPNGDimensions(data, size, &texx, &texy) && (buffer = (uint8 *) malloc(texx * texy * 3)) != NULL)
    {
        image[0] = buffer;
        ok = read_png((char *) data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR, size) == OK;
    }
    else if (Util::GetBMPDimensions(data, size, &texx, &texy) && (buffer = (uint8 *) malloc(texx * texy * 3)) != NULL)
    {
        image[0] = buffer;
        ok = read_bmp((char *) data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR, size) == OK;
    }
    else if (Util::GetBPGDimensions(data, size, &texx, &texy) && (buffer = (uint8 *) malloc(texx * texy * 3)) != NULL)
    {
        image[0] = buffer;
        ok = read_bpg((char *) data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR, size) == OK;
    }
    else
    {
        ok = false;
    }

    if (buffer != NULL)
    {
        free(buffer);
    }

    if (ok)
    {
        Texture texture(&imageResource, data, size, texx, texy);
        n_textureQueue.push(texture);
    }

    if (fetcher != NULL)
    {
        if (resource != NULL)
        {
            fetcher->Finish(resource);
        }
        fetcher = NULL;
    }
    if (resource != NULL)
    {
        free(resource);
        resource = NULL;
    }
}

void    CreateDummyColor(uint32 r, uint32 g, uint32 b)
{
    for (uint32 i = 0; i < 64; i++)
    {
        n_dummyTexture[i * 4 + 0] = (uint8) r;
        n_dummyTexture[i * 4 + 1] = (uint8) g;
        n_dummyTexture[i * 4 + 2] = (uint8) b;
        n_dummyTexture[i * 4 + 3] = (uint8) (i == 0 ? 0 : 0xff);
    }
}

DWORD WINAPI    LoaderThread(LPVOID unused)
{
    bool dontWait = false;
    bool queueLoading = false;
    bool showBlack;

    for (;;)
    {
        if (!queueLoading && (uint32) n_textureQueue.size() < n_queueMin)
        {
            queueLoading = true;
        }
        if (queueLoading && (uint32) n_textureQueue.size() >= n_queueMax)
        {
            queueLoading = false;
        }

        if (dontWait)
        {
            dontWait = false;
        }
        else if (queueLoading)
        {
            LoadTexture();
            PostMessage(n_hWnd, WM_WRITEERRORS, 0, 0);
            if ((uint32) n_textureQueue.size() < n_queueMin)
            {
                continue;
            }
        }
        else
        {
            WaitForSingleObject(n_hEvent, 30000);
        }
        if (n_terminateThread)
        {
            break;
        }

        Job job;
        if (!n_jobQueue.get(job))
        {
            continue;
        }

        dontWait = job.GetDontWait();
        showBlack = job.GetShowBlack();

        Context *context = job.GetContext();
        if (context == NULL)
        {
            continue;
        }

        bool ok = false;
        Texture texture;  

        if (!showBlack && n_textureQueue.size() > 0)
        {
            texture = n_textureQueue.front();
            n_textureQueue.pop();

            int texx = texture.m_tx;
            int texy = texture.m_ty;

            uint8 *buffer = (uint8 *) malloc(texture.m_tx * texture.m_ty * 4);
            uint8 *image[1];

            image[0] = buffer;
            if (buffer == NULL)
            {
                // do nothing, this just elides any further work if we failed to allocate
            }
            else if (Util::IsJPG(texture.m_data, texture.m_size))
            {
                ok = read_jpg((char *) texture.m_data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR | IMG_ALPHA, texture.m_size) == OK;
            }
            else if (Util::IsPNG(texture.m_data, texture.m_size))
            {
                ok = read_png((char *) texture.m_data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR | IMG_ALPHA, texture.m_size) == OK;
            }
            else if (Util::IsBMP(texture.m_data, texture.m_size))
            {
                ok = read_bmp((char *) texture.m_data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR | IMG_ALPHA, texture.m_size) == OK;
            }
            else if (Util::IsBPG(texture.m_data, texture.m_size))
            {
                ok = read_bpg((char *) texture.m_data, image, &texx, &texy, IMG_BUFFERED | IMG_LINEAR | IMG_ALPHA, texture.m_size) == OK;
            }

            free(texture.m_data);

            if (ok)
            {
                texture.m_data = buffer;
                texture.m_size = (uint32) (texx * texy * 4);
                string textRect = "";
                uint32 const aspect = Util::Aspect(job.GetX(), job.GetY());
                if (texture.m_resource != nullptr)
                {
                    auto aspectIt = texture.m_resource->m_aspects.find(aspect);
                    if (aspectIt != texture.m_resource->m_aspects.end())
                    {
                        textRect = aspectIt->second;
                    }
                    else
                    {
                        float dist = 1.0e30f;
                        for (auto &candidate : texture.m_resource->m_aspects)
                        {
                            uint32 const candAspect = candidate.first;
                            float const candDist = Util::AspectDistance(aspect, candAspect);
                            if (candDist < dist)
                            {
                                textRect = candidate.second;
                                dist = candDist;
                            }
                        }
                    }
                }
                else
                {
                    logFile.Report("Error: texture resource was nullptr, punting");
                }
                uint32 x, y, w, h;
                if (textRect != "" && sscanf(textRect.c_str(), "%u,%u,%u,%u", &x, &y, &w, &h) == 4)
                {
                    texture.m_x = x;
                    texture.m_y = y;
                    texture.m_w = w;
                    texture.m_h = h;
                }
                else
                {
                    texture.m_x = 0;
                    texture.m_y = 0;
                    texture.m_w = texx;
                    texture.m_h = texy;
                }
            }
        }

        if (!ok)
        {
            // fill with a color - not much else to do
            if ((++n_ffColour & 7) == 0)
            {
                n_ffColour++; // step over black;
            }

            CreateDummyColor((n_ffColour & 1) && !showBlack ? 0xff : 0, (n_ffColour & 2) && !showBlack ? 0xff : 0, (n_ffColour & 4) && !showBlack ? 0xff : 0);
            texture.m_data = n_dummyTexture;
            texture.m_size = 64 * 4;
            texture.m_originalSize = 64 * 4;
            texture.m_tx = 8;
            texture.m_ty = 8;
            texture.m_x = 0;
            texture.m_x = 0;
            texture.m_w = 8;
            texture.m_h = 8;
            if (!showBlack)
            {
                logFile.Report("Something broke - passing single colored texture to context");
            }
        }

        uint32 effectnum = n_effectShuffle.next_item();
        if (effectnum & 0x80000000)
        {
            VEffect *veffect = veffects[effectnum & 0x0fffffff];
            (*n_setEffectCallback)(*context, NULL, veffect);
        }
        else
        {
            PEffect *peffect = peffects[effectnum];
            (* n_setEffectCallback)(*context, peffect, NULL);
        }

        (* n_textureLoadCallback)(*context, texture);
    }

    return 0;
}

bool    StartThread()
{
    SetQueueSize(n_queueMin, n_queueMax);
    n_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (n_hEvent == NULL)
    {
        return false;
    }
    n_hThread = CreateThread(NULL, 0, LoaderThread, NULL, 0, NULL);
    if (n_hThread == NULL)
    {
        return false;
    }
    return true;
}

void   StopThread()
{
    // Set the terminate flag, and then set the event.
    n_terminateThread = true;
    SetEvent(n_hEvent);
    // Give the thread 100ms to exit
    DWORD const result = WaitForSingleObject(n_hThread, 100);
    if (result != WAIT_OBJECT_0)
    {
        // Thread did not terminate in a timely manner.  It's marginally safer to suspend it than it is to
        // terminate it.  Stopping it one way or another is definitely desirable since it can touch some
        // of the DirectX resources owned by any Contexts that exist, and those are about to get released.
        // It's OK to leave it like this, since the process is about to go away.
        SuspendThread(n_hThread);
    }
}

void    SetCallbacks(void (* setEffectCallback)(Context &context, PEffect *peffect, VEffect *veffect),
                     void (* textureLoadCallback)(Context &context, Texture &texture))
{
    n_setEffectCallback = setEffectCallback;
    n_textureLoadCallback = textureLoadCallback;
}

void    SetQueueSize(uint32 queueMin, uint32 queueMax)
{
    n_queueMin = Util::Clamp(queueMin, 1, 32);
    n_queueMax = Util::Clamp(queueMax, n_queueMin, 128);
}

void    AddFile(string const &cfile, uint32 aspect)
{
    char fileb[RESOURCE_LEN];
    strncpy(fileb, cfile.c_str(), RESOURCE_LEN);
    fileb[RESOURCE_LEN - 1] = 0;

    char *file = byp(fileb);
    strip(file);
    if (file[0] == 0 || file[0] == '#' || file[0] == ';')
    {
        return;
    }

    string fileName;
    string bounds;
    char *semip = strrchr(file, ';');
    if (semip != nullptr)
    {
        *semip++ = 0;
        bounds = semip;
    }
    fileName = file;

    auto it = n_resourceAssembly.find(fileName);
    if (it == n_resourceAssembly.end())
    {
        map<uint32, string> aspects{ { aspect, bounds } };
        n_resourceAssembly[fileName] = std::move(aspects);
    }
    else
    {
        it->second[aspect] = bounds;
    }
}

void    Addecursive(char const *directory, char const *extent, uint32 aspect)
{
    string path = directory + string("\\*.") + extent;
    struct _scnfile_ *files = scnwld(path.c_str(), SCN_RECURSIVE);
    while (files != NULL)
    {
        char filename[1024];
        files = scnfile(files, filename, NULL);
        AddFile(filename, aspect);
    }
}

uint32  AddResourceWorker(string const &resname, uint32 aspect, string const &basepath, bool bpValid)
{
    string pathStart;
    char *linep;
    char line[RESOURCE_LEN];
    string newBasepath;
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    DWORD const attributes = GetFileAttributes(resname.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return (uint32) n_resourceAssembly.size();
    }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        string converted;
        auto fixSeparators = [](char ch)
        {
            return ch == '/' || ch == ':' ? '\\' : ch;
        };
        transform(resname.begin(), resname.end(), back_inserter(converted), fixSeparators);
        vector<string> components = Util::Explode(converted, '\\');
        if (components.size() == 0)
        {
            return (uint32) n_resourceAssembly.size();
        }
        string const &finalPart = components[components.size() - 1];
        uint32 const aspect = Util::FindAspect(finalPart.c_str());
        Addecursive(&resname[0], "jpg", aspect);
        Addecursive(&resname[0], "png", aspect);
        Addecursive(&resname[0], "bmp", aspect);
        Addecursive(&resname[0], "tif", aspect);
        return (uint32) n_resourceAssembly.size();
    }

    char const *resnameStr = resname.c_str();
    Fetcher *fetcher = FindFetcher(resnameStr, false);
    if (bpValid && FindFetcher(resnameStr, true) == NULL)
    {
        _splitpath(resnameStr, drive, dir, NULL, NULL);
        pathStart = drive[0] == 0 && dir[0] != '/' && dir[0] != '\\' ? basepath.c_str() : drive;
        newBasepath = pathStart + dir;
        bpValid = true;
    }
    else
    {
        bpValid = false;
    }
    if (fetcher != NULL)
    {
        Resource *resource;
        if ((resource = fetcher->Prepare(resnameStr)) != NULL)
        {
            while (fetcher->Gets(resource, line, RESOURCE_LEN) != EOF)
            {
                if (line[0] == 0 || line[0] == '#' || line[0] == ';')
                {
                    continue;
                }
                linep = line[0] == '@' ? &line[1] : line;
                if (bpValid && FindFetcher(linep, true) == NULL)
                {
                    _splitpath(linep, drive, dir, fname, ext);
                    pathStart = drive[0] == 0 && dir[0] != '/' && dir[0] != '\\' ? newBasepath.c_str() : drive;
                    snprintf(linep, MAX_PATH, "%s%s%s%s", pathStart.c_str(), dir, fname, ext);
                    linep[MAX_PATH - 1] = 0;
                }
                if (line[0] == '@')
                {
                    AddResourceWorker(linep, aspect, newBasepath, bpValid);
                }
                else
                {
                    AddFile(linep, aspect);
                }
            }
            fetcher->Finish(resource);
            free(resource);
        }
    }
    return (uint32) n_resourceAssembly.size();
}

uint32  AddResources(string const &resourceStr)
{
    uint32 count = 0;
    vector<string> resources = Util::Explode(resourceStr, ';');
    for (string const &resource : resources)
    {
        uint32 aspect;
        string resourceFile;
        aspect = Util::ParseAspect(resource, resourceFile);
        count += AddResourceWorker(resourceFile, aspect, "", true);
        n_aspects.push_back(aspect);
    }
    return count;
}

uint32  FinalizeResources(uint32 volatility)
{
    size_t count = n_aspects.size();
    uint32 num = 0;
    for (auto const &resource : n_resourceAssembly)
    {
        if (resource.second.size() == count)
        {
            n_resources.emplace_back(ImageResource(resource.first, resource.second));
            n_imageShuffle.add_item(num++);
        }
    }
    n_resourceAssembly.clear();

    if (volatility > 0)
    {
        n_imageShuffle.shuffle_payload();
    }
    n_imageShuffle.set_volatility((float) volatility / 100.0f);

    return (uint32) n_resources.size();
}

uint32  FindNearestAspect(uint32 aspect)
{
    uint32 best = aspect;
    float distance = 1.0e6;
    float const magnitude = (float) (aspect >> 16) / (float) (aspect & 0xffff);
    for (uint32 candidate : n_aspects)
    {
        float const candidateMagnitude = (float) (aspect >> 16) / (float) (aspect & 0xffff);
        float const candidateDistance = abs(magnitude - candidateMagnitude);
        if (candidateDistance < distance)
        {
            best = candidate;
            distance = candidateDistance;
        }
    }
    return best;
}

void    InitEffects(bool previewMode, uint32 volatility)
{
    PEffect::InitPEffects(previewMode);
    for (uint32 i = 0; i < peffects.size(); i++)
    {
        n_effectShuffle.add_item(i);
    }

    VEffect::InitVEffects(previewMode);
    for (uint32 i = 0; i < veffects.size(); i++)
    {
        n_effectShuffle.add_item(i | 0x80000000);
    }

    if (volatility > 0)
    {
        n_effectShuffle.shuffle_payload();
    }
    n_effectShuffle.set_volatility((float) volatility / 100.0f);
}

Job::Job() :
m_context(NULL),
m_x(0),
m_y(0),
m_dontWait(false),
m_showBlack(false)
{
}

Job::Job(Context *context, uint32 x, uint32 y, bool dontWait, bool showBlack) :
m_context(context),
m_x(x),
m_y(y),
m_dontWait(dontWait),
m_showBlack(showBlack)
{
}

Job::~Job()
{
}

Context *Job::GetContext()
{
    return m_context;
}

uint32  Job::GetX() const 
{
    return m_x;
}

uint32  Job::GetY() const 
{
    return m_y;
}

bool    Job::GetDontWait() const
{
    return m_dontWait;
}

bool    Job::GetShowBlack() const
{
    return m_showBlack;
}

};
