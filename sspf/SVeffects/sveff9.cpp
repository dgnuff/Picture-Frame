// Vertical interleaved sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect9 : public VEffect
{
public:
    SVEffect9(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect9();
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

Vertex  SVEffect9::s_localVertices[4 * 8];
uint16  SVEffect9::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect9, sveffect9_0, 0, 0);
dec(SVEffect9, sveffect9_1, 1, 0);
dec(SVEffect9, sveffect9_2, 0, 1);
dec(SVEffect9, sveffect9_3, 1, 1);
dec(SVEffect9, sveffect9_4, 0, 2);
dec(SVEffect9, sveffect9_5, 1, 2);
dec(SVEffect9, sveffect9_6, 0, 3);
dec(SVEffect9, sveffect9_7, 1, 3);
#endif

SVEffect9::SVEffect9(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect9::~SVEffect9()
{
}

void    SVEffect9::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 8, 1);
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

void    SVEffect9::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    float const speed = m_edge == 0 ? 2.0f : 4.0f;
    transforms[0].postTrans.y = rand32() & 1 ? speed / 256.0f : -speed / 256.0f;
    transforms[1].postTrans.y = -transforms[0].postTrans.y;

    for (uint32 i = 0; i < 4; i++)
    {
        m_permutation[i] = m_edge == 2 ? i : 3 - i;
    }
    if (m_edge == 3)
    {
        Util::GeneratePermutation(m_permutation, 4);
    }
}

void    SVEffect9::GenerateFrames(float frameNum, float *frames) const
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

void    SVEffect9::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16;
    }
}
