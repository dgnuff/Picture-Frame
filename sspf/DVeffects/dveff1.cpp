// Wipes / compresses the whole old image out and the new one in from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class DVEffect1 : public VEffect
{
public:
    DVEffect1(char const *tag, int32 e, int32 t);
    virtual         ~DVEffect1();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_type;

    Vertex          m_localVertices[2][4];
};

#ifdef NDEBUG
dec(DVEffect1, dveffect1_0, 0, 0);
dec(DVEffect1, dveffect1_1, 1, 0);
dec(DVEffect1, dveffect1_2, 2, 0);
dec(DVEffect1, dveffect1_3, 3, 0);
dec(DVEffect1, dveffect1_4, 0, 1);
dec(DVEffect1, dveffect1_5, 1, 1);
dec(DVEffect1, dveffect1_6, 2, 1);
dec(DVEffect1, dveffect1_7, 3, 1);
#endif

DVEffect1::DVEffect1(char const *tag, int32 e, int32 t)
{
    m_edge = e;
    m_usages = IS_DOUBLE;
    m_type = t;
    m_tag = tag;
}

DVEffect1::~DVEffect1()
{
}

void    DVEffect1::Init()
{
    m_vertices = m_localVertices[0];
    memcpy(m_localVertices[0], s_vertices, 4 * sizeof(Vertex));
    m_numVerts = 4;

    m_vertices_p2 = m_localVertices[1];
    memcpy(m_localVertices[1], s_vertices, 4 * sizeof(Vertex));
    m_numVerts_p2 = 4;

    if (m_type)
    {
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
    }
    else
    {
        for (int32 i = 0; i < 4; i++)
        {
            m_localVertices[0][i].w = 1.0f;
            m_localVertices[1][i].w = 2.0f;
        }
    }

    m_indices = s_indices;
    m_numIndex = 6;

    m_indices_p2 = s_indices;
    m_numIndex_p2 = 6;
}

void    DVEffect1::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
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

void    DVEffect1::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
}
