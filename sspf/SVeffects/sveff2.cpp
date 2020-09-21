// Spin the whole image in or out

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect2 : public VEffect
{
public:
    SVEffect2(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect2();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

public:
    int32           m_shrink;

    static Vertex   s_localVertices[4];
};

 Vertex  SVEffect2::s_localVertices[4];

#ifdef NDEBUG
dec(SVEffect2, sveffect2_0, 0, 0);
dec(SVEffect2, sveffect2_1, 1, 0);
dec(SVEffect2, sveffect2_2, 0, 1);
dec(SVEffect2, sveffect2_3, 1, 1);
#endif

SVEffect2::SVEffect2(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_shrink = s;
    m_count = 2;
    m_tag = tag;
}

SVEffect2::~SVEffect2()
{
}

void    SVEffect2::Init()
{
    if (m_shrink)
    {
        m_vertices = s_localVertices;
        memcpy(s_localVertices, s_vertices, 4 * sizeof(Vertex));
        for (int32 i = 0; i < 4; i++)
        {
            s_localVertices[i].ctrl = -1.0f;
        }
    }
    else
    {
        m_vertices = s_vertices;
    }
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect2::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    if (m_shrink)
    {
        GenerateRandomTrans(&transforms[0], w, h, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        GenerateRandomTrans(&transforms[0], w, h, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.6f);
    }
}
