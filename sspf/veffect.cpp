#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;
using std::min;
using std::vector;

Vertex const    VEffect::s_vertices[4] =
{
    { -1.0f,  1.0f,    0.0f, 0.0f,    0.0f, 0.0f,   0.0f, 0.0f, },      // 0   1
    {  1.0f,  1.0f,    0.0f, 0.0f,    1.0f, 0.0f,   0.0f, 0.0f, },      // 2   3
    { -1.0f, -1.0f,    0.0f, 0.0f,    0.0f, 1.0f,   0.0f, 0.0f, },
    {  1.0f, -1.0f,    0.0f, 0.0f,    1.0f, 1.0f,   0.0f, 0.0f, },
};

Vertex const    VEffect::s_vertices_p2[4] =
{
    { -1.0f,  1.0f,    0.0f, (float) MAX_TRANSFORM, 0.0f, 0.0f,    0.0f, 0.0f, },
    {  1.0f,  1.0f,    0.0f, (float) MAX_TRANSFORM, 1.0f, 0.0f,    0.0f, 0.0f, },
    { -1.0f, -1.0f,    0.0f, (float) MAX_TRANSFORM, 0.0f, 1.0f,    0.0f, 0.0f, },
    {  1.0f, -1.0f,    0.0f, (float) MAX_TRANSFORM, 1.0f, 1.0f,    0.0f, 0.0f, },
};

uint16 const    VEffect::s_indices[6] = { 0, 1, 2, 1, 2, 3 };
bool            VEffect::s_borderRect = false;

safevector<VEffect *> veffects;

VEffect::VEffect() :
m_passes(1),
m_passes_p2(1),
m_count(1),
m_order(0),
m_usages(0),
m_db_p2(0.0f),
m_vertices(nullptr),
m_numVerts(0),
m_vertices_p2(s_vertices_p2),
m_numVerts_p2(4),
m_indices(nullptr),
m_numIndex(0),
m_indices_p2(s_indices),
m_numIndex_p2(6),
m_tag(""),
m_transforms(nullptr)
{
    veffects.push_back(this);
}

VEffect::~VEffect()
{
}

int32   VEffect::GetOrder() const
{
    return m_order;
}

int32   VEffect::GetPasses() const
{
    return m_passes;
}

int32   VEffect::GetPasses_p2() const
{
    return m_passes_p2;
}

int32   VEffect::GetCount() const
{
    return m_count;
}

uint32  VEffect::GetUsages() const
{
    return m_usages;
}

float   VEffect::GetDB_p2() const
{
    return m_db_p2;
}

char const  *VEffect::GetTag() const
{
    return m_tag;
}

void     VEffect::GetVertexData(Vertex *vertices, uint16 *indices) const
{
    if (vertices != NULL)
    {
        memcpy(vertices, m_vertices, m_numVerts * sizeof(Vertex));
        for (uint32 i = 0; i < m_numVerts; i++)
        {
            vertices[i].w *= (1.0f / 1024.0f);
        }
    }

    if (indices != NULL)
    {
        memcpy(indices, m_indices, m_numIndex * sizeof(uint16));
    }
}

void     VEffect::GetVertexData_p2(Vertex *vertices, uint16 *indices) const
{
    if (vertices != NULL)
    {
        memcpy(vertices, m_vertices_p2, m_numVerts_p2 * sizeof(Vertex));
        for (uint32 i = 0; i < m_numVerts_p2; i++)
        {
            vertices[i].w *= (1.0f / 1024.0f);
        }
    }

    if (indices != NULL)
    {
        memcpy(indices, m_indices_p2, m_numIndex_p2 * sizeof(uint16));
    }
}

bool    VEffect::GenerateAlpha(uint8 **data, uint32 width, uint32 height)
{
    return false;
}

void    VEffect::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        frames[0] = frameNum;
    }
}

void    VEffect::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
}

void    VEffect::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    GenerateFrames(frameNum, frames);
    if (frames_p2 != NULL)
    {
        frames_p2[0] = 256.0f - frameNum;
    }
}

Vertex const    *VEffect::GetVertices()
{
    return s_vertices;
}

uint16 const    *VEffect::GetIndices()
{
    return s_indices;
}

void    VEffect::InitVEffects(bool previewMode)
{
    if (previewMode)
    {
        veffects.clear();
        return;
    }

    int32 const numfx = (int32) veffects.size();
    for (int32 i = 0; i < numfx; i++)
    {
        VEffect *veffect = veffects[i];
        veffect->Init();
        int32 const count = Util::Clamp(veffect->GetCount(), 1, 8);
        for (int32 j = 1; j < count; j++)
        {
            veffects.push_back(veffect);
        }
    }
}

void    VEffect::GenerateVertex(Vertex *vertices, int32 i, float x, float y, int32 frame, int32 matrix, float d, float c)
{
    vertices[i].x = x * 2.0f - 1.0f;
    vertices[i].y = y * 2.0f - 1.0f;
    vertices[i].z = (float) frame;
    vertices[i].w = (float) matrix;
    vertices[i].tu = x;
    vertices[i].tv = 1.0f - y;
    vertices[i].db = d;
    vertices[i].ctrl = c;
}

void    VEffect::GenerateRectangles(Vertex *vertices, uint32 xcount, uint32 ycount, D3D11_RECT *rects, uint32 width, uint32 height, float *xFactors, float *yFactors)
{
    static bool vdata[4][2] =
    {
        { false, true  },
        { true,  true  },
        { false, false },
        { true,  false },
    };

    uint32 m = 0;
    uint32 v = 0;
    uint32 r = 0;

    uint32 yi1;
    uint32 yi2;
    float yf1;
    float yf2;

    ScissorPoint(yf1, yi1, 0, ycount, height);
    // DirectX places (0, 0) top left, which results in Scissors rectangles needing to be
    // upside down with respect to N.D.C.  Invert the y coord here.
    yi1 = height - yi1;

    for (uint32 y = 0; y < ycount; y++)
    {
        ScissorPoint(yf2, yi2, y + 1, ycount, height);
        yi2 = height - yi2;
        if (yFactors != NULL)
        {
            yFactors[y] = (yf2 - yf1) * (float) ycount;
        }

        uint32 xi1;
        uint32 xi2;
        float xf1;
        float xf2;

        ScissorPoint(xf1, xi1, 0, xcount, width);

        for (uint32 x = 0; x < xcount; x++)
        {
            ScissorPoint(xf2, xi2, x + 1, xcount, width);
            if (xFactors != NULL)
            {
                xFactors[x] = (xf2 - xf1) * (float) xcount;
            }

            for (uint32 p = 0; p < 4; p++)
            {
                float const x = vdata[p][0] ? xf1 : xf2;
                float const y = vdata[p][1] ? yf1 : yf2;
                GenerateVertex(vertices, v++, x, y, 0, m);
            }
            m++;

            if (rects != NULL)
            {
                rects[r].left = xi1;
                rects[r].right = xi2;
                rects[r].top = yi2;
                rects[r].bottom = yi1;
                r++;
            }

            xi1 = xi2;
            xf1 = xf2;
        }
        yi1 = yi2;
        yf1 = yf2;
    }
}

void    VEffect::GenerateIndices(uint16 *indices, uint32 size)
{
    for (uint32 i = 0; i < size; i++)
    {
        uint32 const o = i * 4;
        uint32 const t = i * 6;
        for (uint32 j = 0; j < 6; j++)
        {
            indices[t + j] = s_indices[j] + o;
        }
    }
}

void    VEffect::GenerateRandomTrans(AATrans *transform, uint32 scrx, uint32 scry, float preX, float preY, float postX, float postY, float spread, float scale) const
{
    if (transform == NULL)
    {
        return;
    }

    // Evenly distributed unit vector using Marsaglia's method from http://mathworld.wolfram.com/SpherePointPicking.html
    // Rather than pick points in (-1 .. 1), (-1, 1) and reject those for which x^2 + y^2 >= 1, start by using our
    // standard code to generate a random point on a circle, and take it from there.
    float r = sqrt(rndf32());
    float a = rndf32() * M_2PI_f;
    float x = r * cos(a);
    float y = r * sin(a);
    float const sumsq = x * x + y * y;

    // Note that Marsaglia specifically prohibits sumsq == 1.0f, while this could allow such a value to leak out due to
    // rounding error in our fp math.  Studying the result, I don't think this will be catastrophic, it'll just result in
    // x == 0.0f, y == 0.0f, z == -1.0f which seems like a perfectly reasonable value.
    transform->rotation.x = 2.0f * x * sqrtf(1.0f - sumsq);
    transform->rotation.y = 2.0f * y * sqrtf(1.0f - sumsq);
    transform->rotation.z = 1.0f - 2.0f * sumsq;
    transform->rotation.w = (M_PI_2_f + rndf32() * M_PI_f * 2.0f) / 256.0f;

    transform->preTrans.x = -preX;
    transform->preTrans.y = -preY;
    transform->preTrans.z = 0.0f;
    transform->preTrans.w = 1.0f;

    if (spread == 0.0f && scale == 0.0f)
    {
        memset(&transform->postTrans, 0, sizeof(XMFLOAT4));
        return;
    }

    if (postX == 0.0f && postY == 0.0f)
    {
        r = sqrt(rndf32());
        a = rndf32() * M_2PI_f;
        postX = r * cos(a) * 0.5f;
        postY = r * sin(a) * 0.5f;
    }

    r = sqrt(rndf32());
    a = rndf32() * M_2PI_f;
    x = r * cos(a) * spread;
    y = r * sin(a) * spread;

    transform->postTrans.x = postX + x;
    transform->postTrans.y = postY + y;

    if (scale > 0.0f)
    {
        float const multX = MultFactor(preX, transform->postTrans.x);
        float const multY = MultFactor(preY, transform->postTrans.y);
        float const mult = min(multX, multY);
        transform->postTrans.x *= mult * scale;
        transform->postTrans.y *= mult * scale;
    }

    transform->postTrans.y *= (float) scrx / (float) scry;

    transform->postTrans.x /= 64.0f;
    transform->postTrans.y /= 64.0f;
    transform->postTrans.z = (rndf32() + 0.5f) / -256.0f;
    transform->postTrans.w = 0.0f;
}

void    VEffect::GenerateShapeAlpha(uint8 **data, uint32 width, uint32 height, uint32 split)
{
    auto Triangle = [](uint32 value, uint32 limit)
    {
        float const fv = (float) value;
        float const fl = (float) limit;
        float const fl2 = (float) limit / 2.0f;
        float const fl4 = (float) limit / 4.0f;

        if (fv < fl4)
        {
            return fv / fl4;
        }
        if (fv < fl4 * 3.0f)
        {
            return (fl2 - fv) / fl4;
        }
        return (fv - fl) / fl4;
    };

    bool const lean = (rand32() & 1) == 1;
    switch (split)
    {
    case 0:
        for (uint32 y = 0; y < height; y++)
        {
            uint32 const c = lean ? y : height - y - 1;
            for (uint32 x = 0; x < width; x++)
            {
                data[y][x] = x > c ? 255 : 0;
            }
        }
        break;

    case 1:
        for (uint32 y = 0; y < height; y++)
        {
            float const s = sinf((float) y / (float) height * M_2PI_f - M_PI_f);
            uint32 const c = (uint32) ((lean ? s : -s) * (float) (width / 3) + (float) (width / 2));
            for (uint32 x = 0; x < width; x++)
            {
                data[y][x] = x > c ? 255 : 0;
            }
        }
        break;

    case 2:
        for (uint32 x = 0; x < width; x++)
        {
            float const s = sinf((float) x / (float) width * M_2PI_f - M_PI_f);
            uint32 const c = (uint32) ((lean ? s : -s) * (float) (height / 3) + (float) (height / 2));
            for (uint32 y = 0; y < height; y++)
            {
                data[y][x] = y > c ? 255 : 0;
            }
        }
        break;

    case 3:
        for (uint32 y = 0; y < height; y++)
        {
            float const t = Triangle(y, height);
            uint32 const c = (uint32) ((lean ? t : -t) * (float) (width / 3) + (float) (width / 2));
            for (uint32 x = 0; x < width; x++)
            {
                data[y][x] = x > c ? 255 : 0;
            }
        }
        break;

    case 4:
        for (uint32 x = 0; x < width; x++)
        {
            float const t = Triangle(x, width);
            uint32 const c = (uint32) ((lean ? t : -t) * (float) (height / 3) + (float) (height / 2));
            for (uint32 y = 0; y < height; y++)
            {
                data[y][x] = y > c ? 255 : 0;
            }
        }
        break;

    default:
        break;
    }
}

void    VEffect::GenerateRectangleAlpha(uint8 **data, uint32 width, uint32 height, uint32 xcount, uint32 ycount)
{
    uint32 const xsize = width / xcount;
    uint32 const ysize = height / ycount;
    uint32 const totalcount = xcount * ycount;
    uint32 const step = 256 / totalcount;
    uint8 alpha = 0;
    for (uint32 y = 0; y < ycount; y++)
    {
        for (uint32 x = 0; x < xcount; x++)
        {
            uint32 const ybase = y * ysize;
            for (uint32 y1 = 0; y1 < ysize; y1++)
            {
                uint32 const xbase = x * xsize;
                for (uint32 x1 = 0; x1 < xsize; x1++)
                {
                    data[ybase + y1][xbase + x1] = alpha;
                }
            }
        }
        alpha += step;
    }
}

float   VEffect::MultFactor(float base, float offset) const
{
    if (offset < 0.0f)
    {
        float const distance = min(-1.0f - base, -0.5f);
        return distance / offset;
    }
    if (offset > 0.0f)
    {
        float const distance = max(1.0f - base, 0.5f);
        return distance / offset;
    }
    return 1.0f;
}

void    VEffect::ScissorPoint(float &fval, uint32 &ival, uint32 pos, uint32 count, uint32 size) const
{
    float const fraction = (float)pos / (float)count;
    ival = (uint32)(fraction * (float)size + 0.5f);
    fval = (float)ival / size;

#ifdef DEBUG
    // See the comment in Context::InitTransform() for why we scale scissors values in debug mode
    if (s_borderRect)
    {
        float const scissorsScale = 3.0f / 4.0f;

        float const fraction_d = fraction * scissorsScale + (1.0f - scissorsScale) / 2.0f;

        ival = (uint32)(fraction_d * (float)size + 0.5f);
        fval = ((float)ival / size - (1.0f - scissorsScale) / 2.0f) / scissorsScale;
    }
#endif
}
