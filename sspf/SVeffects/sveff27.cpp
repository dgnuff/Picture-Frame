// Vertical split sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect27 : public VEffect
{
public:
    SVEffect27(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect27();
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

Vertex  SVEffect27::s_localVertices[4 * 8 * 2];
uint16  SVEffect27::s_localIndices[6 * 8 * 2];

#ifdef NDEBUG
dec(SVEffect27, sveffect27_0, 0, 0);
dec(SVEffect27, sveffect27_1, 1, 0);
dec(SVEffect27, sveffect27_2, 0, 1);
dec(SVEffect27, sveffect27_3, 1, 1);
dec(SVEffect27, sveffect27_4, 0, 2);
dec(SVEffect27, sveffect27_5, 1, 2);
#endif

SVEffect27::SVEffect27(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_passes = 2;
    m_selection = s;
    m_tag = tag;
}

SVEffect27::~SVEffect27()
{
}

void    SVEffect27::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 8, 2);
    for (int32 i = 0; i < 64; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].w = (float) (i >> 5 & 1);
    }

    m_numVerts = 4 * 8 * 2;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8 * 2);

    m_numIndex = 6 * 8 * 2;

    //GenerateScissors(m_rects, 1, 2);

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

        // 2 is random, so we generate a new one for each transition in SVEffect27::GenerateTrans()

    default:
        break;
    }
}

void    SVEffect27::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    uint32 const r = rand32();
    m_edge = r & 1;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    transforms[0].postTrans.y = r & 2 ? 4.0f / 256.0f : -2.0f / 256.0f;
    transforms[1].postTrans.y = -transforms[0].postTrans.y;

    if (m_selection == 2)
    {
        Util::GeneratePermutation(&m_permute[0][0], 8);
        Util::GeneratePermutation(&m_permute[1][0], 8);
    }
}

void    SVEffect27::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        memset(frames, 0, sizeof(float) * 64);
        for (int32 i = 0; i < 8; i++)
        {
            int32 const i1 = m_edge ? i : 7 - i;
            int32 const index1 = m_permute[0][i1];
            frames[index1 + 0] = frameNum;
            int32 const index2 = m_permute[1][i1];
            frames[index2 + 8] = frameNum;

            frameNum = max(frameNum - 16.0f, 0.0f);
        }
    }
}

void    SVEffect27::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
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
