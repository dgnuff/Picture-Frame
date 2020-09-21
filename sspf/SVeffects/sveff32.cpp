// Grow / shrink from center

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect32 : public VEffect
{
public:
    SVEffect32(char const *tag, int32 o, int32 r);
    virtual         ~SVEffect32();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_rotate;

    static Vertex   s_localVertices[4];
};

Vertex  SVEffect32::s_localVertices[4];

#ifdef NDEBUG
dec(SVEffect32, sveffect32_0, 0, 0);
dec(SVEffect32, sveffect32_1, 1, 0);
dec(SVEffect32, sveffect32_2, 0, 1);
dec(SVEffect32, sveffect32_3, 1, 1);
#endif

SVEffect32::SVEffect32(char const *tag, int32 o, int32 r)
{
    m_order = o;
    m_rotate = r;
    m_tag = tag;
}

SVEffect32::~SVEffect32()
{
}

void    SVEffect32::Init()
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

void    SVEffect32::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));

    if (m_rotate)
    {
        const float r = (rndf32() * (M_PI_f + M_PI_2_f) + M_PI_2_f) / 256.0f;
        transforms[0].rotation.z = 1.0f;
        transforms[0].rotation.w = rand32() & 1 ? r : -r;
    }
}
