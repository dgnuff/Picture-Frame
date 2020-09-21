// Unused and available

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

#if 0

class DVEffect16 : public VEffect
{
public:
    DVEffect16(char const *tag, int32 e);
    virtual         ~DVEffect16();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    Vertex          m_localVertices[2][4];
};

#ifdef NDEBUG
//dec(DVEffect16, dveffect16_0, 0);
//dec(DVEffect16, dveffect16_1, 1);
//dec(DVEffect16, dveffect16_2, 2);
//dec(DVEffect16, dveffect16_3, 3);
#endif

DVEffect16::DVEffect16(char const *tag, int32 e)
{
    m_edge = e;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect16::~DVEffect16()
{
}

void    DVEffect16::Init()
{
    m_vertices = m_localVertices[0];
    memcpy(m_localVertices[0], s_vertices, 4 * sizeof(Vertex));
    m_numVerts = 4;

    m_vertices_p2 = m_localVertices[1];
    memcpy(m_localVertices[1], s_vertices, 4 * sizeof(Vertex));
    m_numVerts_p2 = 4;

    switch (m_edge)
    {
    case 0:
        m_localVertices[0][1].w = 1.0f;
        m_localVertices[0][3].w = 1.0f;
        m_localVertices[1][0].w = 2.0f;
        m_localVertices[1][2].w = 2.0f;
        break;

    case 1:
        m_localVertices[0][0].w = 1.0f;
        m_localVertices[0][2].w = 1.0f;
        m_localVertices[1][1].w = 2.0f;
        m_localVertices[1][3].w = 2.0f;
        break;

    case 2:
        m_localVertices[0][0].w = 1.0f;
        m_localVertices[0][1].w = 1.0f;
        m_localVertices[1][2].w = 2.0f;
        m_localVertices[1][3].w = 2.0f;
        break;

    case 3:
        m_localVertices[0][2].w = 1.0f;
        m_localVertices[0][3].w = 1.0f;
        m_localVertices[1][0].w = 2.0f;
        m_localVertices[1][1].w = 2.0f;
        break;

    default:
        break;
    }

    m_indices = s_indices;
    m_numIndex = 6;

    m_indices_p2 = s_indices;
    m_numIndex_p2 = 6;
}

void    DVEffect16::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 3 * sizeof(AATrans));

    if (m_edge & 2)
    {
        transforms[1].postTrans.y = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[2].postTrans.y = -transforms[1].postTrans.y;
    }
    else
    {
        transforms[1].postTrans.x = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[2].postTrans.x = -transforms[1].postTrans.x;
    }
}

void    DVEffect16::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
}

#endif
