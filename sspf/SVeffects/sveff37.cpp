// Grow / shrink from center 4 tiles

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect37 : public VEffect
{
public:
    SVEffect37(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect37();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_sequence;
    uint32          m_permute[4];

    static Vertex   s_localVertices[4 * 4];
    static uint16   s_localIndices[6 * 4];
};

Vertex  SVEffect37::s_localVertices[4 * 4];
uint16  SVEffect37::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(SVEffect37, sveffect37_0, 0, 0);
dec(SVEffect37, sveffect37_1, 1, 0);
dec(SVEffect37, sveffect37_2, 0, 1);
dec(SVEffect37, sveffect37_3, 1, 1);
#endif

SVEffect37::SVEffect37(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_sequence = s;
    m_tag = tag;
}

SVEffect37::~SVEffect37()
{
}

void    SVEffect37::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 2, 2);
    for (int32 i = 0; i < 4 * 4; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].ctrl = -1.0f;
    }

    m_numVerts = 4 * 4;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
}

void    SVEffect37::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 4 * sizeof(AATrans));

    uint32 p;
    int32 r;
    if (m_sequence == 1)
    {
        p = rand32();
        r = p & 4;
        for (int32 i = 0; i < 4; i++)
        {
            m_permute[i] = r ? 3 - p++ % 4 : p++ % 4;
        }
    }

    memset(&transforms[0], 0, 4 * sizeof(AATrans));

    switch (rnd32(3))
    {
    case 0:
    default:
        break;

    case 1:
        for (int32 i = 0; i < 4; i++)
        {
            GenerateRandomTrans(&transforms[i], w, h, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
        break;

    case 2:
        r = rnd32(3);
        for (int32 i = 0; i < 4; i++)
        {
            float d = 0.0f;
            switch (r)
            {
            case 0:
            default:
                d = M_2PI_f / 256.0f;
                break;
            case 1:
                d = -M_2PI_f / 256.0f;
                break;
            case 2:
                d = rand32() & 1 ? M_2PI_f / 256.0f : -M_2PI_f / 256.0f;
                break;
            }
            transforms[i].rotation.z = 1.0f;
            transforms[i].rotation.w = d;
        }
        break;
    }

    transforms[0].preTrans.x = 1.0f / 2.0f;
    transforms[0].preTrans.y = 1.0f / 2.0f;
    transforms[0].preTrans.w = 1.0f;

    transforms[1].preTrans.x = -1.0f / 2.0f;
    transforms[1].preTrans.y = 1.0f / 2.0f;
    transforms[1].preTrans.w = 1.0f;

    transforms[2].preTrans.x = 1.0f / 2.0f;
    transforms[2].preTrans.y = -1.0f / 2.0f;
    transforms[2].preTrans.w = 1.0f;

    transforms[3].preTrans.x = -1.0f / 2.0f;
    transforms[3].preTrans.y = -1.0f / 2.0f;
    transforms[3].preTrans.w = 1.0f;
}

void    SVEffect37::GenerateFrames(float frameNum, float *frames) const
{
    if (frames == NULL)
    {
        return;
    }

    if (m_sequence == 1)
    {
        for (int32 i = 0; i < 4; i++)
        {
            uint32 const p = m_permute[i];
            float const f = Util::Clamp(frameNum, 0.0f, 128.0f) * 2.0f;
            frames[p] = f;
            frameNum -= 32.0f;
        }
    }
    else
    {
        for (int32 i = 0; i < 4; i++)
        {
            frames[i] = frameNum;
        }
    }
}

void    SVEffect37::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 8;
    }
}
