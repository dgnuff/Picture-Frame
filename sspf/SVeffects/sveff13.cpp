// Eight piece curtained slide from side edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect13 : public VEffect
{
public:
    SVEffect13(char const *tag, int32 o, int32 d, int32 s);
    virtual             ~SVEffect13();
    virtual void        Init();
    virtual void        GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    void                GenerateFrames(float frameNum, float *frames) const;
    virtual void        GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32               m_direction;
    int32               m_sequence;
    uint32              m_permutation[8];

    static Vertex       s_localVertices[4 * 8];
    static D3D11_RECT   s_rects[8];
    static uint16       s_localIndices[6 * 8];
};

Vertex      SVEffect13::s_localVertices[4 * 8];
D3D11_RECT  SVEffect13::s_rects[8];
uint16      SVEffect13::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect13, sveffect13_0, 0, 0, 0);
dec(SVEffect13, sveffect13_1, 1, 0, 0);
dec(SVEffect13, sveffect13_2, 0, 1, 0);
dec(SVEffect13, sveffect13_3, 1, 1, 0);
dec(SVEffect13, sveffect13_4, 0, 0, 1);
dec(SVEffect13, sveffect13_5, 1, 0, 1);
dec(SVEffect13, sveffect13_6, 0, 1, 1);
dec(SVEffect13, sveffect13_7, 1, 1, 1);
#endif

SVEffect13::SVEffect13(char const *tag, int32 o, int32 d, int32 s)
{
    m_order = o;
    m_passes = 8;
    m_direction = d;
    m_sequence = s;
    m_usages = USE_SCISSORS_P1;
    m_tag = tag;
}

SVEffect13::~SVEffect13()
{
}

void    SVEffect13::Init()
{
    m_vertices = s_localVertices;

    m_numVerts = 4 * 8;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8);

    m_numIndex = 6 * 8;
}

void     SVEffect13::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    float xFactors[8];
    GenerateRectangles(s_localVertices, 8, 1, s_rects, w, h, xFactors, NULL);
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].w = (float) (i / 4);
    }

    m_transforms = transforms;
    memset(&transforms[0], 0, 8 * sizeof(AATrans));
    float const xBasis = m_direction ? -0.25f / 250.0f : 0.25f / 250.0f;

    for (int32 i = 0; i < 8; i++)
    {
        transforms[i].postTrans.x = xFactors[i] * xBasis;
    }

    if (m_sequence)
    {
        switch (rnd32(3))
        {
        case 0:
        default:
            for (int32 i = 0; i < 8; i++)
            {
                m_permutation[i] = i;
            }
            break;

        case 1:
            for (int32 i = 0; i < 8; i++)
            {
                m_permutation[i] = 7 - i;
            }
            break;

        case 2:
            Util::GeneratePermutation(m_permutation, 8);
            break;
        }
    }
}

void    SVEffect13::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_sequence)
        {
            for (int32 i = 0; i < 8; i++)
            {
                int32 const i1 = m_permutation[i];
                frames[i1] = frameNum * 2;
                frameNum = max(frameNum - 16.0f, 0.0f);
            }
        }
        else
        {
            for (int32 i = 0; i < 8; i++)
            {
                frames[i] = frameNum;
            }
        }
    }
}

void    SVEffect13::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (startIndex != NULL)
    {
        *startIndex = 6 * pass;
    }
    if (scissor != NULL)
    {
        *scissor = &s_rects[pass];
    }
}
