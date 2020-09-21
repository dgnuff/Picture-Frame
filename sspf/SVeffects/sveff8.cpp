// Horizontal interleaved sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect8 : public VEffect
{
public:
    SVEffect8(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect8();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    uint32          m_permutation[4];

    static Vertex   s_localVertices[4 * 8];
    static uint16   s_localIndices[6 * 8];
};

Vertex  SVEffect8::s_localVertices[4 * 8];
uint16  SVEffect8::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect8, sveffect8_0, 0, 0);
dec(SVEffect8, sveffect8_1, 1, 0);
dec(SVEffect8, sveffect8_2, 0, 1);
dec(SVEffect8, sveffect8_3, 1, 1);
dec(SVEffect8, sveffect8_4, 0, 2);
dec(SVEffect8, sveffect8_5, 1, 2);
dec(SVEffect8, sveffect8_6, 0, 3);
dec(SVEffect8, sveffect8_7, 1, 3);
#endif

SVEffect8::SVEffect8(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect8::~SVEffect8()
{
}

void    SVEffect8::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 1, 8);
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[i].z = (float) (i / 8);
        s_localVertices[i].w = i & 4 ? 0.0f : 1.0f;
    }

    m_numVerts = 4 * 8;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8);

    m_numIndex = 6 * 8;
}

void    SVEffect8::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    float const speed = m_edge == 0 ? 2.0f : 4.0f;
    transforms[0].postTrans.x = rand32() & 1 ? speed / 256.0f : -speed / 256.0f;
    transforms[1].postTrans.x = -transforms[0].postTrans.x;

    for (uint32 i = 0; i < 4; i++)
    {
        m_permutation[i] = m_edge == 2 ? i : 3 - i;
    }
    if (m_edge == 3)
    {
        Util::GeneratePermutation(m_permutation, 4);
    }
}

void    SVEffect8::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        for (int32 i = 0; i < 4; i++)
        {
            uint32 const p = m_permutation[i];
            frames[p] = frameNum;
            if (m_edge != 0)
            {
                frameNum = max(frameNum - 32.0f, 0.0f);
            }
        }
    }
}

void    SVEffect8::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16;
    }
}
