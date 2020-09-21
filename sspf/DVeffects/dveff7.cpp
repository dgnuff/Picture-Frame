// V shaped shear the whole old image out and the new one in from left or right

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::min;
using std::max;

class DVEffect7 : public VEffect
{
public:
    DVEffect7(char const *tag, int32 p, int32 e);
    virtual         ~DVEffect7();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames, float *frames_p2) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_point;
    int32           m_edge;

    static Vertex   s_localVertices[2][4 * 2];
    static uint16   s_localIndices[6 * 2];
};

Vertex  DVEffect7::s_localVertices[2][4 * 2];
uint16  DVEffect7::s_localIndices[6 * 2];

#ifdef NDEBUG
dec(DVEffect7, dveffect7_0, 0, 0);
dec(DVEffect7, dveffect7_1, 1, 0);
dec(DVEffect7, dveffect7_2, 0, 1);
dec(DVEffect7, dveffect7_3, 1, 1);
#endif

DVEffect7::DVEffect7(char const *tag, int32 p, int32 e)
{
    m_point = p;
    m_edge = e;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect7::~DVEffect7()
{
}

void    DVEffect7::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];

    GenerateRectangles(s_localVertices[0], 1, 2);
    memcpy(s_localVertices[1], s_localVertices[0], 4 * 2 * sizeof(Vertex));

    for (int32 i = 0; i < 4; i++)
    {
        s_localVertices[0][i + 2].z = 1.0f;
        s_localVertices[0][i + 4].w = 0.0f;
        s_localVertices[1][i + 2].z = 1.0f;
        s_localVertices[1][i].w = 1.0f;
    }

    m_numVerts = 8;
    m_numVerts_p2 = 8;

    m_indices = s_localIndices;
    m_indices_p2 = s_localIndices;

    GenerateIndices(s_localIndices, 2);

    m_numIndex = 12;
    m_numIndex_p2 = 12;
}

void    DVEffect7::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 128.0f : -2.0f / 128.0f;
    transforms[1].postTrans.x = -transforms[0].postTrans.x;
}
void    DVEffect7::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames != NULL && frames_p2 != NULL)
    {
        if (m_point)
        {
            frames[0] = min(frameNum, 128.0f);
            frames[1] = max(frameNum - 128.0f, 0.0f);
            frames_p2[0] = 128.0f - min(frameNum, 128.0f);
            frames_p2[1] = 128.0f - max(frameNum - 128.0f, 0.0f);
        }
        else
        {
            frames[0] = max(frameNum - 128.0f, 0.0f);
            frames[1] = min(frameNum, 128.0f);
            frames_p2[0] = 128.0f - max(frameNum - 128.0f, 0.0f);
            frames_p2[1] = 128.0f - min(frameNum, 128.0f);
        }
    }
}

void    DVEffect7::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4;
    }
    if (numTri_p2 != NULL)
    {
        *numTri_p2 = 4;
    }
}
