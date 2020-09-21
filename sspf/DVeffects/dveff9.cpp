// Horizontal interleaved sliding bars

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class DVEffect9 : public VEffect
{
public:
    DVEffect9(char const *tag, int32 e);
    virtual         ~DVEffect9();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames, float *frames_p2) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;

    static Vertex   s_localVertices[2][4 * 8];
    static uint16   s_localIndices[6 * 8];
};

Vertex  DVEffect9::s_localVertices[2][4 * 8];
uint16  DVEffect9::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(DVEffect9, dveffect9_0,  0);
dec(DVEffect9, dveffect9_1,  1);
dec(DVEffect9, dveffect9_2, -1);
#endif

DVEffect9::DVEffect9(char const *tag, int32 e)
{
    m_edge = e;
    m_count = 2;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect9::~DVEffect9()
{
}

void    DVEffect9::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];
    GenerateRectangles(s_localVertices[0], 1, 8);
    memcpy(s_localVertices[1], s_localVertices[0], 4 * 8 * sizeof(Vertex));
    for (int32 i = 0; i < 32; i++)
    {
        s_localVertices[0][i].z = (float) (i / 8);
        s_localVertices[0][i].w = i & 4 ? 0.0f : 1.0f;
        s_localVertices[1][i].z = (float) (i / 8);
        s_localVertices[1][i].w = i & 4 ? 1.0f : 0.0f;
    }

    m_numVerts = 4 * 8;
    m_numVerts_p2 = 4 * 8;

    GenerateIndices(s_localIndices, 8);

    m_indices = s_localIndices;
    m_numIndex = 6 * 8;

    m_indices_p2 = s_localIndices;
    m_numIndex_p2 = 6 * 8;
}

void    DVEffect9::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    transforms[0].postTrans.x = rand32() & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
    transforms[1].postTrans.x = -transforms[0].postTrans.x;
}

void    DVEffect9::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames != NULL && frames_p2 != NULL)
    {
        for (int32 i = 0; i < 4; i++)
        {
            int32 const fi = m_edge ? 3 - i : i;
            float const f = Util::Clamp(m_edge == -1 ? frameNum : frameNum * 2.0f, 0.0f, 256.0f);
            frames[fi] = f;
            frames_p2[fi] = 256.0f - f;
            if (m_edge != -1)
            {
                frameNum -= 32;
            }
        }
    }
}

void    DVEffect9::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
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
