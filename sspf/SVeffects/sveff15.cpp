// 4 by 4 checkerboard slide/rotate from all edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect15 : public VEffect
{
public:
    SVEffect15(char const *tag, int32 o, int32 r, int32 e);
    virtual         ~SVEffect15();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_rotate;
    int32           m_edge;

    static Vertex   s_localVertices[4 * 16];
    static uint16   s_localIndices[6 * 16];
};

Vertex  SVEffect15::s_localVertices[4 * 16];
uint16  SVEffect15::s_localIndices[6 * 16];

#ifdef NDEBUG
dec(SVEffect15, sveffect15_0, 0, 0, 0);
dec(SVEffect15, sveffect15_1, 1, 0, 0);
dec(SVEffect15, sveffect15_2, 0, 1, 0);
dec(SVEffect15, sveffect15_3, 1, 1, 0);
dec(SVEffect15, sveffect15_4, 0, 0, 1);
dec(SVEffect15, sveffect15_5, 1, 0, 1);
dec(SVEffect15, sveffect15_6, 0, 1, 1);
dec(SVEffect15, sveffect15_7, 1, 1, 1);
#endif

SVEffect15::SVEffect15(char const *tag, int32 o, int32 r, int32 e)
{
    m_order = o;
    m_rotate = r;
    m_edge = e;
    m_tag = tag;
}

SVEffect15::~SVEffect15()
{
}

void    SVEffect15::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 4, 4);

    for (int32 i = 0; i < 64; i++)
    {
        if ((i ^ i >> 2) & 4)
        {
            s_localVertices[i].w = 1.0f;
            s_localVertices[i].db = -1.0f / 256.0f;  // Small depth bias, just enough to prevent z-fighting for the slide.
        }
        else
        {
            s_localVertices[i].w = 0.0f;
            s_localVertices[i].db = 0.0f;
        }
    }

    m_numVerts = 4 * 16;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 16);

    m_numIndex = 6 * 16;
}

void     SVEffect15::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    int32 const t0 = rand32() & 1;
    int32 const t1 = 1 - t0;
    if (m_rotate)
    {
        if (m_edge)
        {
            transforms[t0].rotation.x = 1.0f;
            transforms[t1].rotation.x = -1.0f;
            transforms[t0].rotation.w = -M_PI_2_f / 256.0f;
            transforms[t1].rotation.w = -M_PI_2_f / 256.0f;
            transforms[t0].preTrans.y = -1.0f;
            transforms[t1].preTrans.y = 1.0f;
            transforms[t0].preTrans.w = 1.0f;
            transforms[t1].preTrans.w = 1.0f;
        }
        else
        {
            transforms[t0].rotation.y = 1.0f;
            transforms[t1].rotation.y = -1.0f;
            transforms[t0].rotation.w = -M_PI_2_f / 256.0f;
            transforms[t1].rotation.w = -M_PI_2_f / 256.0f;
            transforms[t0].preTrans.x = 1.0f;
            transforms[t1].preTrans.x = -1.0f;
            transforms[t0].preTrans.w = 1.0f;
            transforms[t1].preTrans.w = 1.0f;
        }
    }
    else
    {
        if (m_edge)
        {
            transforms[t0].postTrans.y = -2.0f / 256.0f;
            transforms[t1].postTrans.y = 2.0f / 256.0f;
        }
        else
        {
            transforms[t0].postTrans.x = -2.0f / 256.0f;
            transforms[t1].postTrans.x = 2.0f / 256.0f;
        }
    }
}

void    SVEffect15::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 32;
    }
}
