#pragma once

#include "types.h"
#include "resources.h"
#include "log.h"

using namespace DirectX;

#define XSIZE           (1024 + 16)
#define YSIZE           (576 + 9)

#define WM_NEWTITLE     (WM_USER + 0)
#define WM_FIRSTLOAD    (WM_USER + 1)
#define WM_PREPARE      (WM_USER + 2)
#define WM_TRANSITION   (WM_USER + 3)
#define WM_HEARTBEAT    (WM_USER + 4)
#define WM_GONEBLACK    (WM_USER + 5)
#define WM_SCREENON     (WM_USER + 6)
#define WM_POST_CREATE  (WM_USER + 7)
#define WM_SHUTDOWN     (WM_USER + 8)
#define WM_WRITEERRORS  (WM_USER + 9)

#define MAX_TRANSFORM   1024
#define MAX_VERTEX      4096
#define MAX_INDEX       8192
#define MAX_FRAME       256
#define MAX_EFFECT      4096

#define M_PI_f          3.14159265359f
#define M_PI_2_f        1.57079632679f
#define M_PI_4_f        0.78539816340f
#define M_2PI_f         6.28318530718f
#define M_3PI_2_f		4.71238898038f

char const *ProcessArg(std::function<void(char const *)> lambda, char *arg, char const *cmdLine, bool optional = false);

struct Vertex
{
    float   x, y, z, w;     // x, y are coord x, y.
                            // z is frame number index
                            // w is transform index
    float   tu, tv;         // texture coordinates
    float   db, ctrl;       // db is depth bias
                            // ctrl == 1: grow over sequence
                            // ctrl == 0: size invariant
                            // ctrl == -1: shrink over sequence
};

struct AATrans
{
    XMFLOAT4 rotation;
    XMFLOAT4 preTrans;      // w specifies multiplier when vertex restored after multiplication.  Generally 1.
    XMFLOAT4 postTrans;
};

struct FRECT
{
    float   left;
    float   top;
    float   right;
    float   bottom;
};

struct Matrices
{
    XMMATRIX viewProjectionMatrix;
};

struct Frames
{
    XMFLOAT4 frames[MAX_FRAME / 4];
};

struct DepthBias
{
    XMFLOAT4 depthBias;
};

struct Transforms
{
    XMFLOAT4 transforms[(MAX_TRANSFORM + 1) * 3];
};

struct IdentityTransform
{
    XMFLOAT4 identityTransform[3];
};

struct PSParams
{
    XMFLOAT4 oldSTFixup;
    XMFLOAT4 newSTFixup;
    XMFLOAT4 ctrlSTFixup;
    XMFLOAT4 frame;
    XMFLOAT4 control;
};

class ImageResource
{
public:
    ImageResource()
    {
    }

    ImageResource(std::string filename, std::map<uint32, std::string> aspects) :
    m_filename(filename),
    m_aspects(aspects)
    {
    }

    std::string m_filename;
    std::map<uint32, std::string> m_aspects;
};

class Texture
{
public:
    Texture() :
    m_resource(nullptr),
    m_data(nullptr),
    m_size(0),
    m_originalSize(0),
    m_tx(0),
    m_ty(0),
    m_x(0),
    m_y(0),
    m_w(0),
    m_h(0)
    {
    }

    Texture(ImageResource const *resource, uint8 *data, uint32 size, uint32 tx, uint32 ty) :
    m_resource(resource),
    m_data(data),
    m_size(size),
    m_originalSize(size),
    m_tx(tx),
    m_ty(ty),
    m_x(0),
    m_y(0),
    m_w(0),
    m_h(0)
    {
    }

    ImageResource const *m_resource;
    uint8               *m_data;
    uint32              m_size;
    uint32              m_originalSize;
    uint32              m_tx;
    uint32              m_ty;
    uint32              m_x;
    uint32              m_y;
    uint32              m_w;
    uint32              m_h;

private:
    static ImageResource s_imageResource;
};

#define dec(cname, name, ...)                           \
static char const *tag_##name = #name "(" #__VA_ARGS__ ")";   \
static cname name(tag_##name, __VA_ARGS__)
