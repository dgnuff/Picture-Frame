// Horizontal sliding bars old image out and new image in

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class DVEffect11 : public VEffect
{
public:
    DVEffect11(char const *tag, int32 d, int32 s);
    virtual         ~DVEffect11();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames, float *frames_p2) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;
    int32           m_selection;
    uint32          m_permute[8];

    static Vertex   s_localVertices[2][4 * 8];
    static uint16   s_localIndices[6 * 8];
};

Vertex  DVEffect11::s_localVertices[2][4 * 8];
uint16  DVEffect11::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(DVEffect11, dveffect11_0, 0, 0);
dec(DVEffect11, dveffect11_1, 1, 0);
dec(DVEffect11, dveffect11_2, 0, 1);
dec(DVEffect11, dveffect11_3, 1, 1);
dec(DVEffect11, dveffect11_4, 0, 2);
dec(DVEffect11, dveffect11_5, 1, 2);
#endif

DVEffect11::DVEffect11(char const *tag, int32 d, int32 s)
{
    m_direction = d;
    m_selection = s;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect11::~DVEffect11()
{
}

void    DVEffect11::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];
    GenerateRectangles(s_localVertices[0], 1, 8);
    memcpy(s_localVertices[1], s_localVertices[0], 4 * 8 * sizeof(Vertex));
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[0][i].z = (float) (i / 4);
        s_localVertices[0][i].w = 0.0f;
        s_localVertices[1][i].z = (float) (i / 4);
        s_localVertices[1][i].w = 1.0f;
    }

    m_numVerts = 4 * 8;
    m_numVerts_p2 = 4 * 8;

    GenerateIndices(s_localIndices, 8);

    m_indices = s_localIndices;
    m_numIndex = 6 * 8;

    m_indices_p2 = s_localIndices;
    m_numIndex_p2 = 6 * 8;

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

        // 2 is random, so we generate a new one for each transition in DVEffect11::GenerateTrans()

    default:
        break;
    }
}

void    DVEffect11::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    transforms[0].postTrans.x = m_direction ? 4.0f / 256.0f : -4.0f / 256.0f;
    transforms[1].postTrans.x = -transforms[0].postTrans.x;

    if (m_selection == 2)
    {
        Util::GeneratePermutation(m_permute, 8);
    }
}

void    DVEffect11::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames != NULL && frames_p2 != NULL)
    {
        for (int32 i = 0; i < 8; i++)
        {
            int32 const i1 = m_permute[i];
            frames[i1] = max(frameNum, 0.0f);
            frames_p2[i1] = max(128.0f - frameNum, 0.0f);
            frameNum -= 16;
        }
    }
}

void    DVEffect11::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16;
    }
    if (numTri_p2 != NULL)
    {
        *numTri_p2 = 16;
    }
}
