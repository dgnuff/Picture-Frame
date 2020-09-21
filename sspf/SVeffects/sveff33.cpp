// Two piece V slide from side edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect33 : public VEffect
{
public:
    SVEffect33(char const *tag, int32 o, int32 e, int32 s);
    virtual         ~SVEffect33();
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

Vertex  SVEffect33::s_localVertices[4 * 4];
uint16  SVEffect33::s_localIndices[2][3 * 3] =
{
    {
         0,  1,  2,
         4,  6,  7,
         9, 10, 15,
    },
    {
         8,  9, 11,
        13, 14, 15,
         0,  3,  6,
    },
};

#ifdef NDEBUG
dec(SVEffect33, sveffect33_0, 0, 0, 0);
dec(SVEffect33, sveffect33_1, 1, 0, 0);
dec(SVEffect33, sveffect33_2, 0, 1, 0);
dec(SVEffect33, sveffect33_3, 1, 1, 0);
dec(SVEffect33, sveffect33_4, 0, 0, 1);
dec(SVEffect33, sveffect33_5, 1, 0, 1);
dec(SVEffect33, sveffect33_6, 0, 1, 1);
dec(SVEffect33, sveffect33_7, 1, 1, 1);
#endif

SVEffect33::SVEffect33(char const *tag, int32 o, int32 e, int32 s)
{
    m_order = o;
    m_edge = e;
    m_separate = s;
    m_tag = tag;
}

SVEffect33::~SVEffect33()
{
}

void    SVEffect33::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 1, 2);
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

void     SVEffect33::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));

    uint32 const r = rand32();
    m_spread = r & 3;
    if (r & 4)
    {
        transforms[0].rotation.y = 1.0f;
        transforms[1].rotation.y = -1.0f;
        transforms[0].rotation.w = M_PI_2_f / 256.0f;
        transforms[1].rotation.w = M_PI_2_f / 256.0f;
        transforms[0].preTrans.x = -1.0f;
        transforms[1].preTrans.x = 1.0f;
        transforms[0].preTrans.w = 1.0f;
        transforms[1].preTrans.w = 1.0f;
    }
    else
    {
        transforms[0].postTrans.x = 2.0f / 256.0f;
        transforms[1].postTrans.x = -2.0f / 256.0f;
    }
}

void    SVEffect33::GenerateFrames(float frameNum, float *frames) const
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

void    SVEffect33::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 3;
    }
}
