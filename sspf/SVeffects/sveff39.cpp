// Shaped split wipe the whole image from opposite edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect39 : public VEffect
{
public:
    SVEffect39(char const *tag, int32 o, int32 e, int32 s);
    virtual         ~SVEffect39();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual bool    GenerateAlpha(uint8 **data, uint32 width, uint32 height);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_split;
    int32           m_flip;
};

#ifdef NDEBUG
dec(SVEffect39, sveffect39_0,  0, 0, 0);
dec(SVEffect39, sveffect39_1,  1, 0, 0);
dec(SVEffect39, sveffect39_2,  0, 2, 0);
dec(SVEffect39, sveffect39_3,  1, 2, 0);
dec(SVEffect39, sveffect39_4,  0, 0, 1);
dec(SVEffect39, sveffect39_5,  1, 0, 1);
dec(SVEffect39, sveffect39_6,  0, 2, 2);
dec(SVEffect39, sveffect39_7,  1, 2, 2);
dec(SVEffect39, sveffect39_8,  0, 0, 3);
dec(SVEffect39, sveffect39_9,  1, 0, 3);
dec(SVEffect39, sveffect39_10, 0, 2, 4);
dec(SVEffect39, sveffect39_11, 1, 2, 4);
#endif

SVEffect39::SVEffect39(char const *tag, int32 o, int32 e, int32 s)
{
    m_order = o;
    m_passes = 2;
    m_edge = e;
    m_split = s;
    m_usages = USE_STENCIL_P1;
    m_tag = tag;
}

SVEffect39::~SVEffect39()
{
}

void    SVEffect39::Init()
{
    m_vertices = s_vertices;
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect39::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    uint32 const r = rand32();
    m_edge = m_edge & 2 | r & 1;
    m_flip = r & 2;
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    if (m_edge & 2)
    {
        transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[1].postTrans.y = -transforms[0].postTrans.y;
    }
    else
    {
        transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 256.0f : -2.0f / 256.0f;
        transforms[1].postTrans.x = -transforms[0].postTrans.x;
    }
}

bool    SVEffect39::GenerateAlpha(uint8 **data, uint32 width, uint32 height)
{
    VEffect::GenerateShapeAlpha(data, width, height, m_split);
    return true;
}

void    SVEffect39::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
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
