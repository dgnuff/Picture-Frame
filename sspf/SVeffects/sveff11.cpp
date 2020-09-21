// Two piece curtained slide from side edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::max;

class SVEffect11 : public VEffect
{
public:
    SVEffect11(char const *tag, int32 o, int32 d, int32 s);
    virtual             ~SVEffect11();
    virtual void        Init();
    virtual void        GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void        GenerateFrames(float frameNum, float *frames) const;
    virtual void        GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32               m_direction;
    int32               m_sequence;
    float               m_offset;

    static Vertex       s_localVertices[4 * 2];
    static D3D11_RECT   s_rects[2];
    static uint16       s_localIndices[6 * 2];
};

Vertex      SVEffect11::s_localVertices[4 * 2];
D3D11_RECT  SVEffect11::s_rects[2];
uint16      SVEffect11::s_localIndices[6 * 2];

#ifdef NDEBUG
dec(SVEffect11, sveffect11_0, 0, 0, 0);
dec(SVEffect11, sveffect11_1, 1, 0, 0);
dec(SVEffect11, sveffect11_2, 0, 1, 0);
dec(SVEffect11, sveffect11_3, 1, 1, 0);
dec(SVEffect11, sveffect11_4, 0, 0, 1);
dec(SVEffect11, sveffect11_5, 1, 0, 1);
dec(SVEffect11, sveffect11_6, 0, 1, 1);
dec(SVEffect11, sveffect11_7, 1, 1, 1);
#endif

SVEffect11::SVEffect11(char const *tag, int32 o, int32 d, int32 s)
{
    m_order = o;
    m_passes = 2;
    m_direction = d;
    m_sequence = s;
    m_usages = USE_SCISSORS_P1;
    m_tag = tag;
}

SVEffect11::~SVEffect11()
{
}

void    SVEffect11::Init()
{
    m_vertices = s_localVertices;

    m_numVerts = 4 * 2;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 2);

    m_numIndex = 6 * 2;
}

void     SVEffect11::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    float xFactors[2];
    GenerateRectangles(s_localVertices, 2, 1, s_rects, w, h, xFactors, NULL);
    for (int32 i = 0; i < 8; i++)
    {
        s_localVertices[i].z = (float) (i / 4);
        s_localVertices[i].w = (float)(i / 4);
    }

    m_transforms = transforms;
    memset(&transforms[0], 0, 2 * sizeof(AATrans));
    float const numerator = (m_direction ? -1.0f : 1.0f) * (m_sequence ? 2.0f : 1.0f);
    transforms[0].postTrans.x = numerator * xFactors[0] / 253.0f;
    transforms[1].postTrans.x = numerator * xFactors[1] / 253.0f;

    static float offsets[] = { 64.0f, 128.0f, -64.0f, -128.0f };

    if (m_sequence)
    {
        m_offset = offsets[rand32() & 3];
    }
}

void    SVEffect11::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_sequence)
        {
            if (m_offset > 0)
            {
                frames[0] = frameNum * 2.0f;
                frames[1] = max(frames[0] - m_offset, 0.0f);
            }
            else
            {
                frames[1] = frameNum * 2.0f;
                frames[0] = max(frames[1] + m_offset, 0.0f);
            }
        }
        else
        {
            for (int32 i = 0; i < 2; i++)
            {
                frames[i] = frameNum;
            }
        }
    }
}

void    SVEffect11::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (startIndex != NULL)
    {
        *startIndex = pass * 6;
    }
    if (scissor != NULL && pass < 2)
    {
        *scissor = &s_rects[pass];
    }
}
