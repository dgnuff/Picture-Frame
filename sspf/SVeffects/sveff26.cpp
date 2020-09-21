// Horizontal split sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect26 : public VEffect
{
public:
    SVEffect26(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect26();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_selection;
    int32           m_edge;
    uint32          m_permute[2][8];
    D3D11_RECT      m_rects[2];

    static Vertex   s_localVertices[4 * 8 * 2];
    static uint16   s_localIndices[6 * 8 * 2];
};

Vertex  SVEffect26::s_localVertices[4 * 8 * 2];
uint16  SVEffect26::s_localIndices[6 * 8 * 2];

#ifdef NDEBUG
dec(SVEffect26, sveffect26_0, 0, 0);
dec(SVEffect26, sveffect26_1, 1, 0);
dec(SVEffect26, sveffect26_2, 0, 1);
dec(SVEffect26, sveffect26_3, 1, 1);
dec(SVEffect26, sveffect26_4, 0, 2);
dec(SVEffect26, sveffect26_5, 1, 2);
#endif

SVEffect26::SVEffect26(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_passes = 2;
    m_selection = s;
    m_tag = tag;
}

SVEffect26::~SVEffect26()
{
}

void    SVEffect26::Init()
{
    Vertex generateBuffer[4 * 8 * 2];
    GenerateRectangles(generateBuffer, 2, 8);
    for (int32 i = 0; i < 64; i++)
    {
        generateBuffer[i].z = (float) (i / 4);
        generateBuffer[i].w = (float) (i >> 2 & 1);
    }

    for (int32 i = 0; i < 8; i++)
    {
        memcpy(&s_localVertices[i * 4 +  0], &generateBuffer[i * 8 + 0], 4 * sizeof(Vertex));
        memcpy(&s_localVertices[i * 4 + 32], &generateBuffer[i * 8 + 4], 4 * sizeof(Vertex));
    }
    m_vertices = s_localVertices;

    m_numVerts = 4 * 8 * 2;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8 * 2);

    m_numIndex = 6 * 8 * 2;

    //GenerateScissors(m_rects, 2, 1);

    switch (m_selection)
    {
    case 0:
        for (int32 i = 0; i < 8; i++)
        {
            m_permute[0][i] = i;
            m_permute[1][i] = i;
        }
        break;

    case 1:
        for (int32 i = 0; i < 8; i++)
        {
            m_permute[0][i] = i;
            m_permute[1][i] = 7 - i;
        }
        break;

    // 2 is random, so we generate a new one for each transition in SVEffect26::GenerateTrans()

    default:
        break;
    }
}

void    SVEffect26::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    uint32 const r = rand32();
    m_edge = r & 1;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    transforms[0].postTrans.x = r & 2 ? 4.0f / 256.0f : -2.0f / 256.0f;
    transforms[1].postTrans.x = -transforms[0].postTrans.x;

    if (m_selection == 2)
    {
        Util::GeneratePermutation(&m_permute[0][0], 8);
        Util::GeneratePermutation(&m_permute[1][0], 8);
    }
}

void    SVEffect26::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        memset(frames, 0, sizeof(float) * 64);
        for (int32 i = 0; i < 8; i++)
        {
            int32 const i1 = m_edge ? i : 7 - i;
            int32 const index1 = m_permute[0][i1] * 2;
            frames[index1 + 0] = frameNum;
            int32 const index2 = m_permute[1][i1] * 2;
            frames[index2 + 1] = frameNum;
            frameNum = max(frameNum - 16.0f, 0.0f);
        }
    }
}

void    SVEffect26::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    //if (scissor != NULL)
    //{
    //    *scissor = &m_rects[pass];
    //}
    if (startIndex != NULL)
    {
        *startIndex = 16 * 3 * pass;
    }
    if (numTri != NULL)
    {
        *numTri = 16;
    }
}
