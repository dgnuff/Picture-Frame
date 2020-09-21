// Slide 2 by 2 squares in or out diagonally from the corners

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect16 : public VEffect
{
public:
    SVEffect16(char const *tag, int32 o, int32 d);
    virtual             ~SVEffect16();
    virtual void        Init();
    virtual void        GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void        GenerateFrames(float frameNum, float *frames) const;
    virtual void        GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32               m_direction;
    int32               m_sequence;
    uint32              m_permute[4];
    static Vertex       s_localVertices[4 * 4];
    static D3D11_RECT   s_rects[4];
    static uint16       s_localIndices[6 * 4];
};

Vertex      SVEffect16::s_localVertices[4 * 4];
D3D11_RECT  SVEffect16::s_rects[4];
uint16      SVEffect16::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(SVEffect16, sveffect16_0, 0, 0);
dec(SVEffect16, sveffect16_1, 1, 0);
dec(SVEffect16, sveffect16_2, 0, 1);
dec(SVEffect16, sveffect16_3, 1, 1);
dec(SVEffect16, sveffect16_4, 0, 2);
dec(SVEffect16, sveffect16_5, 1, 2);
dec(SVEffect16, sveffect16_6, 0, 3);
dec(SVEffect16, sveffect16_7, 1, 3);
dec(SVEffect16, sveffect16_8, 0, 4);
dec(SVEffect16, sveffect16_9, 1, 4);
#endif

SVEffect16::SVEffect16(char const *tag, int32 o, int32 d)
{
    m_order = o;
    m_passes = 4;
    m_direction = d;
    m_usages = USE_SCISSORS_P1;
    m_tag = tag;
}

SVEffect16::~SVEffect16()
{
}

void    SVEffect16::Init()
{
    m_vertices = s_localVertices;

    m_numVerts = 4 * 4;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
}

void    SVEffect16::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    GenerateRectangles(s_localVertices, 2, 2, s_rects, w, h);
    for (int32 i = 0; i < 16; i++)
    {
        s_localVertices[i].z = m_vertices[i].w;
    }

    m_transforms = transforms;
    m_sequence = rnd32(3);
    switch (m_sequence)
    {
    case 1:
        Util::GeneratePermutation(m_permute, 4);
        break;

    case 2:
        uint32 p;
        int32 r;
        p = rand32();
        r = p & 4;
        for (int32 i = 0; i < 4; i++)
        {
            m_permute[i] = r ? 3 - p++ % 4 : p++ % 4;
        }
        break;

    default:
        break;
    }

    memset(&transforms[0], 0, 4 * sizeof(AATrans));
    for (int32 i = 0; i < 4; i++)
    {
        float const numerator = m_sequence ? 1.6f : 1.0f;
        float const divisor = numerator / 254.0f;
        if (m_direction == 4)
        {
            transforms[i].postTrans.x = (-1.0f + (float) (i & 1) * 2.0f) * divisor;
            transforms[i].postTrans.y = (-1.0f + (float) (i & 2)) * divisor;
            if (rand32() & 1)
            {
                transforms[i].postTrans.x *= rndf32();
            }
            else
            {
                transforms[i].postTrans.y *= rndf32();
            }
        }
        else
        {
            transforms[i].postTrans.x = (-1.0f + (float) (i & 1) * 2.0f) * (m_direction & 1 ? 1.0f : -1.0f) * divisor;
            transforms[i].postTrans.y = (-1.0f + (float) (i & 2)) * (m_direction & 2 ? 1.0f : -1.0f) * divisor;
        }
    }
}

void    SVEffect16::GenerateFrames(float frameNum, float *frames) const
{
    if (frames == NULL)
    {
        return;
    }

    switch (m_sequence)
    {
    default:
        for (int32 i = 0; i < 4; i++)
        {
            frames[i] = frameNum;
        }
        break;

    case 1:
    case 2:
        for (int32 i = 0; i < 4; i++)
        {
            uint32 const p = m_permute[i];
            frames[p] = frameNum;
            frameNum = max(frameNum - 32.0f, 0.0f);
        }
        break;
    }
}

void    SVEffect16::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (startIndex != NULL)
    {
        *startIndex = 6 * pass;
    }
    if (scissor != NULL && m_direction == 0)
    {
        *scissor = &s_rects[pass];
    }
}
