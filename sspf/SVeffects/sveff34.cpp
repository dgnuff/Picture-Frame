// Two piece V slide from top / bottom edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect34 : public VEffect
{
public:
    SVEffect34(char const *tag, int32 o, int32 e, int32 s);
    virtual         ~SVEffect34();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_separate;
    int32           m_spread;

    static Vertex   s_localVertices[4 * 4];
    static uint16   s_localIndices[2][3 * 3];
};

Vertex  SVEffect34::s_localVertices[4 * 4];
uint16  SVEffect34::s_localIndices[2][3 * 3] =
{
    {
         1,  2,  3,
         4,  6,  7,
         9, 10, 12,
    },
    {
         0,  3,  6,
         8,  9, 11,
        12, 13, 14,
    },
};

#ifdef NDEBUG
dec(SVEffect34, sveffect34_0, 0, 0, 0);
dec(SVEffect34, sveffect34_1, 1, 0, 0);
dec(SVEffect34, sveffect34_2, 0, 1, 0);
dec(SVEffect34, sveffect34_3, 1, 1, 0);
dec(SVEffect34, sveffect34_4, 0, 0, 1);
dec(SVEffect34, sveffect34_5, 1, 0, 1);
dec(SVEffect34, sveffect34_6, 0, 1, 1);
dec(SVEffect34, sveffect34_7, 1, 1, 1);
#endif

SVEffect34::SVEffect34(char const *tag, int32 o, int32 e, int32 s)
{
    m_order = o;
    m_edge = e;
    m_separate = s;
    m_tag = tag;
}

SVEffect34::~SVEffect34()
{
}

void    SVEffect34::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 2, 1);
    memcpy(&s_localVertices[4 * 2], s_localVertices, 4 * 2 * sizeof(Vertex));

    for (int32 i = 0; i < 8; i++)
    {
        s_localVertices[i].w = 0.0f;
        s_localVertices[i + 8].z = 1.0f;
        s_localVertices[i + 8].w = 1.0f;
    }

    m_numVerts = 4 * 4;

    m_indices = s_localIndices[m_edge ? 1 : 0];

    m_numIndex = 3 * 3;
}

void     SVEffect34::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));

    uint32 const r = rand32();
    m_spread = r & 3;
    if (r & 4)
    {
        transforms[0].rotation.x = -1.0f;
        transforms[1].rotation.x = 1.0f;
        transforms[0].rotation.w = M_PI_2_f / 256.0f;
        transforms[1].rotation.w = M_PI_2_f / 256.0f;
        transforms[0].preTrans.y = -1.0f;
        transforms[1].preTrans.y = 1.0f;
        transforms[0].preTrans.w = 1.0f;
        transforms[1].preTrans.w = 1.0f;
    }
    else
    {
        transforms[0].postTrans.y = 2.0f / 256.0f;
        transforms[1].postTrans.y = -2.0f / 256.0f;
    }
}
void    SVEffect34::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_separate)
        {
            float const f1 = Util::Clamp(frameNum * 2.0f - (m_spread & 1 ? 128.0f : 256.0f), 0.0f, 256.0f);
            float const f2 = Util::Clamp(frameNum * 2.0f, 0.0f, 256.0f);
            if (m_spread & 2)
            {
                frames[0] = f1;
                frames[1] = f2;
            }
            else
            {
                frames[0] = f2;
                frames[1] = f1;
            }
        }
        else
        {
            frames[0] = frameNum;
            frames[1] = frameNum;
        }
    }
}

void    SVEffect34::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 3;
    }
}
