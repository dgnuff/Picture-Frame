// Two piece diagonal slides

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect35 : public VEffect
{
public:
    SVEffect35(char const *tag, int32 o, int32 l);
    virtual         ~SVEffect35();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;

private:
    int32           m_lean;
    int32           m_separate;

    static Vertex   s_localVertices[4 * 2];
    static uint16   s_localIndices[2][3 * 2];
};

Vertex  SVEffect35::s_localVertices[4 * 2];
uint16  SVEffect35::s_localIndices[2][3 * 2] =
{
    {
        0, 1, 2,
        5, 6, 7,
    },
    {
        0, 1, 3,
        7, 4, 6,
    },
};

#ifdef NDEBUG
dec(SVEffect35, sveffect35_0, 0, 0);
dec(SVEffect35, sveffect35_1, 1, 0);
dec(SVEffect35, sveffect35_2, 0, 1);
dec(SVEffect35, sveffect35_3, 1, 1);
#endif

SVEffect35::SVEffect35(char const *tag, int32 o, int32 l)
{
    m_order = o;
    m_count = 3;
    m_lean = l;
    m_tag = tag;
}

SVEffect35::~SVEffect35()
{
}

void    SVEffect35::Init()
{
    m_vertices = s_localVertices;
    memcpy(s_localVertices, s_vertices, 4 * sizeof(Vertex));
    memcpy(&s_localVertices[4], s_vertices, 4 * sizeof(Vertex));

    for (int32 i = 4; i < 8; i++)
    {
        s_localVertices[i].z = 1.0f;
        s_localVertices[i].w = 1.0f;
    }

    m_numVerts = 4 * 2;

    m_indices = s_localIndices[m_lean ? 1 : 0];

    m_numIndex = 3 * 2;
}

void     SVEffect35::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    switch (rnd32(7))
    {
    case 0:
        if (m_lean)
        {
            transforms[0].postTrans.x = 2.0f / 256.0f;
            transforms[1].postTrans.x = -2.0f / 256.0f;
        }
        else
        {
            transforms[0].postTrans.x = -2.0f / 256.0f;
            transforms[1].postTrans.x = 2.0f / 256.0f;
        }
        break;
    case 1:
        transforms[0].postTrans.y = 2.0f / 256.0f;
        transforms[1].postTrans.y = -2.0f / 256.0f;
        break;
    case 2:
        if (m_lean)
        {
            transforms[0].postTrans.x = -2.0f / 256.0f;
            transforms[0].postTrans.y = 2.0f / 256.0f;
            transforms[1].postTrans.x = 2.0f / 256.0f;
            transforms[1].postTrans.y = -2.0f / 256.0f;
        }
        else
        {
            transforms[0].postTrans.x = 2.0f / 256.0f;
            transforms[0].postTrans.y = 2.0f / 256.0f;
            transforms[1].postTrans.x = -2.0f / 256.0f;
            transforms[1].postTrans.y = -2.0f / 256.0f;
        }
        break;
    case 3:
        if (m_lean)
        {
            transforms[0].postTrans.x = 1.0f / 256.0f;
            transforms[0].postTrans.y = 1.0f / 256.0f;
            transforms[1].postTrans.x = -1.0f / 256.0f;
            transforms[1].postTrans.y = -1.0f / 256.0f;
        }
        else
        {
            transforms[0].postTrans.x = -1.0f / 256.0f;
            transforms[0].postTrans.y = 1.0f / 256.0f;
            transforms[1].postTrans.x = 1.0f / 256.0f;
            transforms[1].postTrans.y = -1.0f / 256.0f;
        }
        break;
    case 4:
        if (m_lean)
        {
            transforms[0].postTrans.x = 2.0f / 256.0f;
            transforms[0].postTrans.y = -2.0f / 256.0f;
            transforms[1].postTrans.x = -2.0f / 256.0f;
            transforms[1].postTrans.y = 2.0f / 256.0f;
        }
        else
        {
            transforms[0].postTrans.x = -2.0f / 256.0f;
            transforms[0].postTrans.y = -2.0f / 256.0f;
            transforms[1].postTrans.x = 2.0f / 256.0f;
            transforms[1].postTrans.y = 2.0f / 256.0f;
        }
        break;
    case 5:
        transforms[0].rotation.y = m_lean ? 1.0f : -1.0f;
        transforms[1].rotation.y = -transforms[0].rotation.y;
        transforms[0].rotation.w = M_PI_2_f / 256.0f;
        transforms[1].rotation.w = M_PI_2_f / 256.0f;
        transforms[0].preTrans.x = -transforms[0].rotation.y;
        transforms[1].preTrans.x = transforms[0].rotation.y;
        transforms[0].preTrans.w = 1.0f;
        transforms[1].preTrans.w = 1.0f;
        break;
    case 6:
        transforms[0].rotation.x = -1.0f;
        transforms[1].rotation.x = 1.0f;
        transforms[0].rotation.w = M_PI_2_f / 256.0f;
        transforms[1].rotation.w = M_PI_2_f / 256.0f;
        transforms[0].preTrans.y = -1.0f;
        transforms[1].preTrans.y = 1.0f;
        transforms[0].preTrans.w = 1.0f;
        transforms[1].preTrans.w = 1.0f;
        break;
    }
    m_separate = rnd32(6);
}

void    SVEffect35::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_separate & 4)
        {
            frames[0] = frameNum;
            frames[1] = frameNum;
        }
        else
        {
            float const f1 = Util::Clamp(frameNum * 2.0f - (m_separate & 1 ? 128.0f : 256.0f), 0.0f, 256.0f);
            float const f2 = Util::Clamp(frameNum * 2.0f, 0.0f, 256.0f);
            if (m_separate & 2)
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
    }
}
