// Horizontal sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect28 : public VEffect
{
public:
    SVEffect28(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect28();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_selection;
    uint32          m_permute[8];

    static Vertex   s_localVertices[4 * 8];
    static uint16   s_localIndices[6 * 8];
};

Vertex  SVEffect28::s_localVertices[4 * 8];
uint16  SVEffect28::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect28, sveffect28_0, 0, 0);
dec(SVEffect28, sveffect28_1, 1, 0);
dec(SVEffect28, sveffect28_2, 0, 1);
dec(SVEffect28, sveffect28_3, 1, 1);
dec(SVEffect28, sveffect28_4, 0, 2);
dec(SVEffect28, sveffect28_5, 1, 2);
#endif

SVEffect28::SVEffect28(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_selection = s;
    m_tag = tag;
}

SVEffect28::~SVEffect28()
{
}

void    SVEffect28::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 1, 8);
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].w = 0.0f;
    }

    m_numVerts = 4 * 8;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8);

    m_numIndex = 6 * 8;

    switch (m_selection)
    {
    case 0:
        for (int32 i = 0; i < 8; i++)
        {
            m_permute[i] = i;
        }
        break;

    case 1:
        for (int32 i = 0; i < 8; i++)
        {
            m_permute[i] = 7 - i;
        }
        break;

        // 2 is random, so we generate a new one for each transition in SVEffect28::GenerateTrans()

    default:
        break;
    }
}

void    SVEffect28::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    transforms[0].postTrans.x = rand32() & 1 ? 4.0f / 256.0f : -4.0f / 256.0f;

    if (m_selection == 2)
    {
        Util::GeneratePermutation(m_permute, 8);
    }
}

void    SVEffect28::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        for (int32 i = 0; i < 8; i++)
        {
            int32 const i1 = m_permute[i];
            frames[i1] = frameNum;
            frameNum = max(frameNum - 16.0f, 0.0f);
        }
    }
}

void    SVEffect28::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16;
    }
}
