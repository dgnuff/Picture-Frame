// Grow / shrink from edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect31 : public VEffect
{
public:
    SVEffect31(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect31();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_edge;

    static Vertex   s_localVertices[4];
};

Vertex  SVEffect31::s_localVertices[4];

#ifdef NDEBUG
dec(SVEffect31, sveffect31_0, 0, 0);
dec(SVEffect31, sveffect31_1, 1, 0);
dec(SVEffect31, sveffect31_2, 0, 1);
dec(SVEffect31, sveffect31_3, 1, 1);
dec(SVEffect31, sveffect31_4, 0, 2);
dec(SVEffect31, sveffect31_5, 1, 2);
dec(SVEffect31, sveffect31_6, 0, 3);
dec(SVEffect31, sveffect31_7, 1, 3);
#endif

SVEffect31::SVEffect31(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect31::~SVEffect31()
{
}

void    SVEffect31::Init()
{
    m_vertices = s_localVertices;
    memcpy(s_localVertices, s_vertices, 4 * sizeof(Vertex));
    for (uint32 i = 0; i < 4; i++)
    {
        s_localVertices[i].ctrl = -1.0f;
    }

    m_numVerts = 4;

    m_indices = s_indices;

    m_numIndex = 6;
}

void    SVEffect31::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));

    float const direction = (float) ((m_edge & 2) - 1);
    if (m_edge & 1)
    {
        transforms[0].preTrans.x = direction;
    }
    else
    {
        transforms[0].preTrans.y = direction;
    }
    transforms[0].preTrans.w = 1.0f;
}
