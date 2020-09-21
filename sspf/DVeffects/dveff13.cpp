// Shaped split wipe the whole old image out and the new one in from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class DVEffect13 : public VEffect
{
public:
    DVEffect13(char const *tag, int32 e, int32 s);
    virtual         ~DVEffect13();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 width, uint32 height);
    virtual bool    GenerateAlpha(uint8 **data, uint32 width, uint32 height);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_split;
    int32           m_flip;

    static Vertex   s_localVertices[4];
};

Vertex  DVEffect13::s_localVertices[4];

#ifdef NDEBUG
dec(DVEffect13, dveffect13_0,  0, 0);
dec(DVEffect13, dveffect13_1,  1, 0);
dec(DVEffect13, dveffect13_2,  2, 0);
dec(DVEffect13, dveffect13_3,  3, 0);
dec(DVEffect13, dveffect13_4,  0, 1);
dec(DVEffect13, dveffect13_5,  1, 1);
dec(DVEffect13, dveffect13_6,  2, 2);
dec(DVEffect13, dveffect13_7,  3, 2);
dec(DVEffect13, dveffect13_8,  0, 3);
dec(DVEffect13, dveffect13_9,  1, 3);
dec(DVEffect13, dveffect13_10, 2, 4);
dec(DVEffect13, dveffect13_11, 3, 4);
#endif

DVEffect13::DVEffect13(char const *tag, int32 e, int32 s)
{
    m_edge = e;
    m_split = s;
    m_passes = 2;
    m_passes_p2 = 2;
    m_usages = USE_STENCIL_P1 | USE_STENCIL_P2 | IS_DOUBLE;
    m_db_p2 = 0.5f;
    m_tag = tag;
}

DVEffect13::~DVEffect13()
{
}

void    DVEffect13::Init()
{
    m_vertices = s_vertices;
    m_numVerts = 4;

    m_vertices_p2 = s_localVertices;
    memcpy(s_localVertices, s_vertices, 4 * sizeof(Vertex));
    for (int32 i = 0; i < 4; i++)
    {
        s_localVertices[i].w = 1.0f;
    }

    m_numVerts_p2 = 4;

    m_indices = s_indices;
    m_numIndex = 6;

    m_indices_p2 = s_indices;
    m_numIndex_p2 = 6;
}

void    DVEffect13::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_flip = rand32() & 1;
    m_transforms = transforms;
    memset(&transforms[0], 0, 3 * sizeof(AATrans));
    if (m_edge & 2)
    {
        transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[1].postTrans.y = -transforms[0].postTrans.y;
        transforms[2].postTrans.y = transforms[0].postTrans.y;
    }
    else
    {
        transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[1].postTrans.x = -transforms[0].postTrans.x;
        transforms[2].postTrans.x = transforms[0].postTrans.x;
    }
}

bool    DVEffect13::GenerateAlpha(uint8 **data, uint32 width, uint32 height)
{
    VEffect::GenerateShapeAlpha(data, width, height, m_split);
    return true;
}

void    DVEffect13::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (baseTrans != NULL)
    {
        *baseTrans = pass;
    }
    if (alpha != NULL)
    {
        *alpha = (uint8) ((m_flip ? 1 - pass : pass) * 255);
    }
}
