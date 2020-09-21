// Rotate entire image about corners and centers of edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect40 : public VEffect
{
public:
    SVEffect40(char const *tag, int32 o, int32 c, int32 d);
    virtual         ~SVEffect40();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    float           m_rotationDir;
    int32           m_corner;
};

#ifdef NDEBUG
dec(SVEffect40, sveffect40_0, 0, 0,  1);
dec(SVEffect40, sveffect40_1, 1, 0,  1);
dec(SVEffect40, sveffect40_2, 0, 1,  1);
dec(SVEffect40, sveffect40_3, 1, 1,  1);
dec(SVEffect40, sveffect40_4, 0, 0, -1);
dec(SVEffect40, sveffect40_5, 1, 0, -1);
dec(SVEffect40, sveffect40_6, 0, 1, -1);
dec(SVEffect40, sveffect40_7, 1, 1, -1);
#endif

SVEffect40::SVEffect40(char const *tag, int32 o, int32 c, int32 d)
{
    m_order = o;
    m_corner = c;
    m_rotationDir = (float) d;
    m_tag = tag;
}

SVEffect40::~SVEffect40()
{
}

void    SVEffect40::Init()
{
    m_vertices = s_vertices;
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect40::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));

    int32 const edge = rand32() & 3;
    float x;
    float y;
    if (m_corner)
    {
        x = -1.0f + (float) (edge & 1) * 2.0f;
        y = -1.0f + (float) (edge & 2);
    }
    else if (edge & 1)
    {
        x = 0.0f;
        y = -1.0f + (float) (edge & 2);
    }
    else
    {
        x = -1.0f + (float) (edge & 2);
        y = 0.0f;
    }

    transforms[0].rotation.z = 1.0f;
    transforms[0].rotation.w = (float) M_PI_2_f * m_rotationDir / (m_corner ? 256.0f : 128.0f);
    transforms[0].preTrans.x = x;
    transforms[0].preTrans.y = y;
    transforms[0].preTrans.w = 1.0f;
}
