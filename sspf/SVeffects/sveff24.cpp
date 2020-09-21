// Slide entire image in or out diagnoally from the corners

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect24 : public VEffect
{
public:
    SVEffect24(char const *tag, int32 o, int32 c);
    virtual         ~SVEffect24();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_corner;
};

#ifdef NDEBUG
dec(SVEffect24, sveffect24_0, 0, 0);
dec(SVEffect24, sveffect24_1, 1, 0);
dec(SVEffect24, sveffect24_2, 0, 1);
dec(SVEffect24, sveffect24_3, 1, 1);
dec(SVEffect24, sveffect24_4, 0, 2);
dec(SVEffect24, sveffect24_5, 1, 2);
dec(SVEffect24, sveffect24_6, 0, 3);
dec(SVEffect24, sveffect24_7, 1, 3);
#endif

SVEffect24::SVEffect24(char const *tag, int32 o, int32 c)
{
    m_order = o;
    m_corner = c;
    m_tag = tag;
}

SVEffect24::~SVEffect24()
{
}

void    SVEffect24::Init()
{
    m_vertices = s_vertices;
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect24::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    transforms[0].postTrans.x = m_corner & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
    transforms[0].postTrans.y = m_corner & 2 ? 2.0f / 256.0f : -2.0f / 256.0f;
}
