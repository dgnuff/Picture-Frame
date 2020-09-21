// Two piece slide or rotate from side edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect6 : public VEffect
{
public:
    SVEffect6(char const *tag, int32 o, int32 r, int32 s);
    virtual         ~SVEffect6();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_rotate;
    int32           m_separate;
    int32           m_dir;

    static Vertex   s_localVertices[2][4 * 2];
    static uint16   s_localIndices[6 * 2];
};

Vertex  SVEffect6::s_localVertices[2][4 * 2];
uint16  SVEffect6::s_localIndices[6 * 2];

#ifdef NDEBUG
dec(SVEffect6, sveffect6_0, 0, 0, 0);
dec(SVEffect6, sveffect6_1, 1, 0, 0);
dec(SVEffect6, sveffect6_2, 0, 1, 0);
dec(SVEffect6, sveffect6_3, 1, 1, 0);
dec(SVEffect6, sveffect6_4, 0, 0, 1);
dec(SVEffect6, sveffect6_5, 1, 0, 1);
dec(SVEffect6, sveffect6_6, 0, 1, 1);
dec(SVEffect6, sveffect6_7, 1, 1, 1);
#endif

SVEffect6::SVEffect6(char const *tag, int32 o, int32 r, int32 s)
{
    m_order = o;
    m_rotate = r;
    m_separate = s;
    m_tag = tag;
}

SVEffect6::~SVEffect6()
{
}

void    SVEffect6::Init()
{
    m_vertices = s_localVertices[m_separate ? 1 : 0];
    GenerateRectangles(s_localVertices[m_separate ? 1 : 0], 2, 1);
    if (m_separate)
    {
        for (int32 i = 4; i < 8; i++)
        {
            s_localVertices[1][i].z = 1.0f;
        }
    }

    m_numVerts = 4 * 2;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 2);

    m_numIndex = 6 * 2;
}

void     SVEffect6::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    if (m_rotate)
    {
        transforms[0].rotation.y = 1.0f;
        transforms[1].rotation.y = -1.0f;
        transforms[0].rotation.w = -M_PI_2_f / 256.0f;
        transforms[1].rotation.w = -M_PI_2_f / 256.0f;
        transforms[0].preTrans.x = 1.0f;
        transforms[1].preTrans.x = -1.0f;
        transforms[0].preTrans.w = 1.0f;
        transforms[1].preTrans.w = 1.0f;
    }
    else
    {
        transforms[0].postTrans.x = -1.0f / 256.0f;
        transforms[1].postTrans.x = 1.0f / 256.0f;
    }
    m_dir = rand32() & 3;
}

void    SVEffect6::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_separate)
        {
            float const f1 = Util::Clamp(frameNum * 2.0f - (m_dir & 1 ? 128.0f : 256.0f), 0.0f, 256.0f);
            float const f2 = Util::Clamp(frameNum * 2.0f, 0.0f, 256.0f);
            if (m_dir & 2)
            {
                frames[0] = f1;
                frames[1] = f2;
            }
            else
            {
                frames[0] = f2;
                frames[1] = f1;
            }
        }
        else
        {
            frames[0] = frameNum;
        }
    }
}

void    SVEffect6::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4;
    }
}
