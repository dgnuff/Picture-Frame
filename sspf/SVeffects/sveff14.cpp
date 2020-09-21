// Eight piece curtained slide from top/bottom edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect14 : public VEffect
{
public:
    SVEffect14(char const *tag, int32 o, int32 d, int32 s);
    virtual             ~SVEffect14();
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

Vertex      SVEffect14::s_localVertices[4 * 8];
D3D11_RECT  SVEffect14::s_rects[8];
uint16      SVEffect14::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect14, sveffect14_0, 0, 0, 0);
dec(SVEffect14, sveffect14_1, 1, 0, 0);
dec(SVEffect14, sveffect14_2, 0, 1, 0);
dec(SVEffect14, sveffect14_3, 1, 1, 0);
dec(SVEffect14, sveffect14_4, 0, 0, 1);
dec(SVEffect14, sveffect14_5, 1, 0, 1);
dec(SVEffect14, sveffect14_6, 0, 1, 1);
dec(SVEffect14, sveffect14_7, 1, 1, 1);
#endif

SVEffect14::SVEffect14(char const *tag, int32 o, int32 d, int32 s)
{
    m_order = o;
    m_passes = 8;
    m_direction = d;
    m_sequence = s;
    m_usages = USE_SCISSORS_P1;
    m_tag = tag;
}

SVEffect14::~SVEffect14()
{
}

void    SVEffect14::Init()
{
    m_vertices = s_localVertices;

    m_numVerts = 4 * 8;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8);

    m_numIndex = 6 * 8;
}

void     SVEffect14::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    float yFactors[8];
    GenerateRectangles(s_localVertices, 1, 8, s_rects, w, h, NULL, yFactors);
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].w = (float) (i / 4);
    }

    m_transforms = transforms;
    memset(&transforms[0], 0, 8 * sizeof(AATrans));
    float const yBasis = m_direction ? -0.25f / 250.0f : 0.25f / 250.0f;

    for (int32 i = 0; i < 8; i++)
    {
        transforms[i].postTrans.y = yFactors[i] * yBasis;
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

void    SVEffect14::GenerateFrames(float frameNum, float *frames) const
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

void    SVEffect14::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (startIndex != NULL)
    {
        *startIndex = pass * 6;
    }
    if (scissor != NULL && pass < 8)
    {
        *scissor = &s_rects[pass];
    }
}
