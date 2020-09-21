// Horizontal split two piece slide of old image out and new one in.

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class DVEffect3 : public VEffect
{
public:
    DVEffect3(char const *tag, int32 m, int32 t);
    virtual             ~DVEffect3();
    virtual void        Init();
    virtual void        GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void        GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32               m_mode;
    int32               m_type;

    Vertex              m_localVertices[2][4 * 2];

    static D3D11_RECT   s_rects[2];
    static uint16       s_localIndices[6 * 2];
};

D3D11_RECT  DVEffect3::s_rects[2];
uint16      DVEffect3::s_localIndices[6 * 2];

#ifdef NDEBUG
dec(DVEffect3, dveffect3_0, 0, 0);
dec(DVEffect3, dveffect3_1, 1, 0);
dec(DVEffect3, dveffect3_2, 2, 0);
dec(DVEffect3, dveffect3_3, 3, 0);
dec(DVEffect3, dveffect3_4, 0, 1);
dec(DVEffect3, dveffect3_5, 1, 1);
dec(DVEffect3, dveffect3_6, 2, 1);
dec(DVEffect3, dveffect3_7, 3, 1);
#endif

DVEffect3::DVEffect3(char const *tag, int32 m, int32 t)
{
    m_mode = m;
    m_type = t;
    m_passes = 2;
    m_passes_p2 = 2;
    m_usages = USE_SCISSORS_P1 | USE_SCISSORS_P2 | IS_DOUBLE;
    m_tag = tag;
}

DVEffect3::~DVEffect3()
{
}

void    DVEffect3::Init()
{
    m_vertices = m_localVertices[0];
    m_vertices_p2 = m_localVertices[1];

    m_numVerts = 4 * 2;
    m_numVerts_p2 = 4 * 2;

    m_indices = s_localIndices;
    m_indices_p2 = s_localIndices;
    GenerateIndices(s_localIndices, 2);

    m_numIndex = 6 * 2;
    m_numIndex_p2 = 6 * 2;
}

void     DVEffect3::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    float yFactors[2];
    GenerateRectangles(m_localVertices[0], 1, 2, s_rects, w, h, NULL, yFactors);
    memcpy(m_localVertices[1], m_localVertices[0], 4 * 2 * sizeof(Vertex));

    if (m_type)
    {
        for (int32 i = 0; i < 8; i++)
        {
            m_localVertices[0][i].w = 0.0f;
            m_localVertices[1][i].w = 0.0f;
        }
        switch (m_mode)
        {
        case 0:
            m_localVertices[0][0].w = 1.0f;
            m_localVertices[0][1].w = 1.0f;
            m_localVertices[0][4].w = 1.0f;
            m_localVertices[0][5].w = 1.0f;
            m_localVertices[1][2].w = 2.0f;
            m_localVertices[1][3].w = 2.0f;
            m_localVertices[1][6].w = 2.0f;
            m_localVertices[1][7].w = 2.0f;
            break;

        case 1:
            m_localVertices[0][2].w = 1.0f;
            m_localVertices[0][3].w = 1.0f;
            m_localVertices[0][6].w = 1.0f;
            m_localVertices[0][7].w = 1.0f;
            m_localVertices[1][0].w = 2.0f;
            m_localVertices[1][1].w = 2.0f;
            m_localVertices[1][4].w = 2.0f;
            m_localVertices[1][5].w = 2.0f;
            break;

        case 2:
            m_localVertices[0][1].w = 1.0f;
            m_localVertices[0][3].w = 1.0f;
            m_localVertices[0][4].w = 1.0f;
            m_localVertices[0][6].w = 1.0f;
            m_localVertices[1][0].w = 2.0f;
            m_localVertices[1][2].w = 2.0f;
            m_localVertices[1][5].w = 2.0f;
            m_localVertices[1][7].w = 2.0f;
            break;

        case 3:
            m_localVertices[0][0].w = 1.0f;
            m_localVertices[0][2].w = 1.0f;
            m_localVertices[0][5].w = 1.0f;
            m_localVertices[0][7].w = 1.0f;
            m_localVertices[1][1].w = 2.0f;
            m_localVertices[1][3].w = 2.0f;
            m_localVertices[1][4].w = 2.0f;
            m_localVertices[1][6].w = 2.0f;
            break;

        default:
            break;
        }
    }
    else
    {
        for (int32 i = 0; i < 8; i++)
        {
            m_localVertices[0][i].w = 1.0f;
            m_localVertices[1][i].w = 2.0f;
        }
    }

    m_transforms = transforms;
    memset(&transforms[0], 0, 6 * sizeof(AATrans));

    switch (m_mode)
    {
    default:
    case 0:
        transforms[1].postTrans.y = yFactors[0] / 256.0f;
        transforms[4].postTrans.y = yFactors[1] / 256.0f;
        break;

    case 1:
        transforms[1].postTrans.y = -yFactors[0] / 256.0f;
        transforms[4].postTrans.y = -yFactors[1] / 256.0f;
        break;

    case 2:
        transforms[1].postTrans.x = 2.0f / 256.0f;
        transforms[4].postTrans.x = -2.0f / 256.0f;
        break;

    case 3:
        transforms[1].postTrans.x = -2.0f / 256.0f;
        transforms[4].postTrans.x = 2.0f / 256.0f;
        break;
    }

    transforms[2].postTrans.x = -transforms[1].postTrans.x;
    transforms[5].postTrans.x = -transforms[4].postTrans.x;
    transforms[2].postTrans.y = -transforms[1].postTrans.y;
    transforms[5].postTrans.y = -transforms[4].postTrans.y;
}

void    DVEffect3::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (baseTrans != NULL)
    {
        *baseTrans = pass * 3;
    }
    if (startIndex != NULL)
    {
        *startIndex = pass * 6;
    }
    if (scissor != NULL && pass < 2)
    {
        *scissor = &s_rects[pass];
    }
}
