// Shears the whole old image out and the new one in from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::min;
using std::max;

class DVEffect6 : public VEffect
{
public:
    DVEffect6(char const *tag, int32 w, int32 e);
    virtual         ~DVEffect6();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames, float *frames_p2) const;

private:
    int32           m_which;
    int32           m_edge;

    static Vertex   s_localVertices[2][4][4];
};

Vertex  DVEffect6::s_localVertices[2][4][4];

#ifdef _MSC_VER
dec(DVEffect6, dveffect6_0, 0, 0);
dec(DVEffect6, dveffect6_1, 1, 0);
dec(DVEffect6, dveffect6_2, 0, 1);
dec(DVEffect6, dveffect6_3, 1, 1);
dec(DVEffect6, dveffect6_4, 0, 2);
dec(DVEffect6, dveffect6_5, 1, 2);
dec(DVEffect6, dveffect6_6, 0, 3);
dec(DVEffect6, dveffect6_7, 1, 3);
#endif

DVEffect6::DVEffect6(char const *tag, int32 w, int32 e)
{
    m_which = w;
    m_edge = e;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect6::~DVEffect6()
{
}

void    DVEffect6::Init()
{
    m_vertices = s_localVertices[0][m_edge];
    m_vertices_p2 = s_localVertices[1][m_edge];
    memcpy(s_localVertices[0][m_edge], s_vertices, 4 * sizeof(Vertex));
    memcpy(s_localVertices[1][m_edge], s_vertices, 4 * sizeof(Vertex));
    m_numVerts = 4;
    m_numVerts_p2 = 4;

    if (m_edge & 2)
    {
        s_localVertices[0][m_edge][1].z = 1.0f;
        s_localVertices[0][m_edge][3].z = 1.0f;
        s_localVertices[1][m_edge][1].z = 1.0f;
        s_localVertices[1][m_edge][3].z = 1.0f;
    }
    else
    {
        s_localVertices[0][m_edge][2].z = 1.0f;
        s_localVertices[0][m_edge][3].z = 1.0f;
        s_localVertices[1][m_edge][2].z = 1.0f;
        s_localVertices[1][m_edge][3].z = 1.0f;
    }

    for (int32 i = 0; i < 4; i++)
    {
        s_localVertices[1][m_edge][i].w = 1.0f;
    }


    m_indices = s_indices;
    m_numIndex = 6;

    m_indices_p2 = s_indices;
    m_numIndex_p2 = 6;
}

void    DVEffect6::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    if (m_edge & 2)
    {
        transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 128.0f : -2.0f / 128.0f;
        transforms[1].postTrans.y = -transforms[0].postTrans.y;
    }
    else
    {
        transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 128.0f : -2.0f / 128.0f;
        transforms[1].postTrans.x = -transforms[0].postTrans.x;
    }
}

void    DVEffect6::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames != NULL && frames_p2 != NULL)
    {
        if (m_which)
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
