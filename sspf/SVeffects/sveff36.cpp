// Two piece diagonal split rotate from corners

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect36 : public VEffect
{
public:
    SVEffect36(char const *tag, int32 o, int32 l, int32 w);
    virtual         ~SVEffect36();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;

private:
    int32           m_lean;
    int32           m_which;
    int32           m_separate;

    static Vertex   s_localVertices[4 * 2];
    static uint16   s_localIndices[2][3 * 2];
};

Vertex  SVEffect36::s_localVertices[4 * 2];
uint16  SVEffect36::s_localIndices[2][3 * 2] =
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
dec(SVEffect36, sveffect36_0, 0, 0, 0);
dec(SVEffect36, sveffect36_1, 1, 0, 0);
dec(SVEffect36, sveffect36_2, 0, 1, 0);
dec(SVEffect36, sveffect36_3, 1, 1, 0);
dec(SVEffect36, sveffect36_4, 0, 0, 1);
dec(SVEffect36, sveffect36_5, 1, 0, 1);
dec(SVEffect36, sveffect36_6, 0, 1, 1);
dec(SVEffect36, sveffect36_7, 1, 1, 1);
#endif

SVEffect36::SVEffect36(char const *tag, int32 o, int32 l, int32 w)
{
    m_order = o;
    m_lean = l;
    m_which = w;
    m_tag = tag;
}

SVEffect36::~SVEffect36()
{
}

void    SVEffect36::Init()
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

void     SVEffect36::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    if (m_which)
    {
        if (m_lean)
        {
            transforms[0].rotation.z = 1.0f;
            transforms[0].rotation.w = 1.2f / 256.0f;
            transforms[0].preTrans.x = -1.0f;
            transforms[0].preTrans.y = 1.0f;
            transforms[0].preTrans.w = 1.0f;
            transforms[1].rotation.z = 1.0f;
            transforms[1].rotation.w = 1.2f / 256.0f;
            transforms[1].preTrans.x = 1.0f;
            transforms[1].preTrans.y = -1.0f;
            transforms[1].preTrans.w = 1.0f;
        }
        else
        {
            transforms[0].rotation.z = 1.0f;
            transforms[0].rotation.w = 0.8f / 256.0f;
            transforms[0].preTrans.x = -1.0f;
            transforms[0].preTrans.y = -1.0f;
            transforms[0].preTrans.w = 1.0f;
            transforms[1].rotation.z = 1.0f;
            transforms[1].rotation.w = 0.8f / 256.0f;
            transforms[1].preTrans.x = 1.0f;
            transforms[1].preTrans.y = 1.0f;
            transforms[1].preTrans.w = 1.0f;
        }
    }
    else
    {
        if (m_lean)
        {
            transforms[0].rotation.z = 1.0f;
            transforms[0].rotation.w = -0.8f / 256.0f;
            transforms[0].preTrans.x = 1.0f;
            transforms[0].preTrans.y = -1.0f;
            transforms[0].preTrans.w = 1.0f;
            transforms[1].rotation.z = 1.0f;
            transforms[1].rotation.w = -0.8f / 256.0f;
            transforms[1].preTrans.x = -1.0f;
            transforms[1].preTrans.y = 1.0f;
            transforms[1].preTrans.w = 1.0f;
        }
        else
        {
            transforms[0].rotation.z = 1.0f;
            transforms[0].rotation.w = -1.2f / 256.0f;
            transforms[0].preTrans.x = 1.0f;
            transforms[0].preTrans.y = 1.0f;
            transforms[0].preTrans.w = 1.0f;
            transforms[1].rotation.z = 1.0f;
            transforms[1].rotation.w = -1.2f / 256.0f;
            transforms[1].preTrans.x = -1.0f;
            transforms[1].preTrans.y = -1.0f;
            transforms[1].preTrans.w = 1.0f;
        }
    }
    m_separate = rand32() & 7;
}

void    SVEffect36::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_separate & 4)
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
        else
        {
            frames[0] = frameNum;
            frames[1] = frameNum;
        }
    }
}
