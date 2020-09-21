// Wipes and rotates of the whole image from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect1 : public VEffect
{
public:
    SVEffect1(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect1();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);

private:
    int32           m_edge;
};

#ifdef NDEBUG
dec(SVEffect1, sveffect1_0,  0, 0);
dec(SVEffect1, sveffect1_1,  1, 0);
dec(SVEffect1, sveffect1_2,  0, 1);
dec(SVEffect1, sveffect1_3,  1, 1);
dec(SVEffect1, sveffect1_4,  0, 2);
dec(SVEffect1, sveffect1_5,  1, 2);
dec(SVEffect1, sveffect1_6,  0, 3);
dec(SVEffect1, sveffect1_7,  1, 3);
#endif

SVEffect1::SVEffect1(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect1::~SVEffect1()
{
}

void    SVEffect1::Init()
{
    m_vertices = s_vertices;
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect1::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    if (rand32() & 1)
    {
        transforms[0].rotation.x = m_edge & 2 ? m_edge & 1 ? 1.0f : -1.0f : 0.0f;
        transforms[0].rotation.y = m_edge & 2 ? 0.0f : m_edge & 1 ? 1.0f : -1.0f;
        transforms[0].rotation.z = 0.0f;
        transforms[0].rotation.w = m_edge & 2 ? (float) M_PI_2_f / 256.0f : -(float) M_PI_2_f / 256.0f;
        transforms[0].preTrans.x = transforms[0].rotation.y;
        transforms[0].preTrans.y = transforms[0].rotation.x;
        transforms[0].preTrans.w = 1.0f;
    }
    else
    {
        if (m_edge & 2)
        {
            transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        }
        else
        {
            transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        }
    }
}
