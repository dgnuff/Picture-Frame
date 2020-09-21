// Grow / shrink from corners

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect30 : public VEffect
{
public:
    SVEffect30(char const *tag, int32 o, int32 c);
    virtual         ~SVEffect30();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_corner;

    static Vertex   s_localVertices[4];
};

Vertex  SVEffect30::s_localVertices[4];

#ifdef NDEBUG
dec(SVEffect30, sveffect30_0, 0, 0);
dec(SVEffect30, sveffect30_1, 1, 0);
dec(SVEffect30, sveffect30_2, 0, 1);
dec(SVEffect30, sveffect30_3, 1, 1);
dec(SVEffect30, sveffect30_4, 0, 2);
dec(SVEffect30, sveffect30_5, 1, 2);
dec(SVEffect30, sveffect30_6, 0, 3);
dec(SVEffect30, sveffect30_7, 1, 3);
#endif

SVEffect30::SVEffect30(char const *tag, int32 o, int32 c)
{
    m_order = o;
    m_corner = c;
    m_tag = tag;
}

SVEffect30::~SVEffect30()
{
}

void    SVEffect30::Init()
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

void    SVEffect30::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    transforms[0].preTrans.x = (float) ((m_corner & 1) * 2 - 1);
    transforms[0].preTrans.y = (float) ((m_corner & 2) - 1);
    transforms[0].preTrans.w = 1.0f;
}
