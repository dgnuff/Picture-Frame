// Compression wipes of the whole image from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect44 : public VEffect
{
public:
    SVEffect44(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect44();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_edge;
    Vertex          m_vertexBuffer[4];
    static float    s_transformIndices[4][4];
};

#ifdef _MSC_VER
dec(SVEffect44, sveffect44_0,  0, 0);
dec(SVEffect44, sveffect44_1,  1, 0);
dec(SVEffect44, sveffect44_2,  0, 1);
dec(SVEffect44, sveffect44_3,  1, 1);
dec(SVEffect44, sveffect44_4,  0, 2);
dec(SVEffect44, sveffect44_5,  1, 2);
dec(SVEffect44, sveffect44_6,  0, 3);
dec(SVEffect44, sveffect44_7,  1, 3);
#endif

float   SVEffect44::s_transformIndices[4][4] =
{
    { 1.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 0.0f },
};

SVEffect44::SVEffect44(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect44::~SVEffect44()
{
}

void    SVEffect44::Init()
{
    m_vertices = m_vertexBuffer;
    memcpy(m_vertexBuffer, s_vertices, sizeof(Vertex) * 4);
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect44::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans) * 2);

    for (int i = 0; i < 4; i++)
    {
        m_vertexBuffer[i].w = s_transformIndices[m_edge][i];
    }

    if (m_edge & 2)
    {
        transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
    }
    else
    {
        transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
    }
}
