// Spin the image in or out in 256 pieces

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::min;
using std::max;

class SVEffect4 : public VEffect
{
public:
    SVEffect4(char const *tag, int32 o, int32 d);
    virtual         ~SVEffect4();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;

    static Vertex   s_localVertices[4 * 256];
    static uint16   s_localIndices[6 * 256];
};

Vertex  SVEffect4::s_localVertices[4 * 256];
uint16  SVEffect4::s_localIndices[6 * 256];

#ifdef NDEBUG
dec(SVEffect4, sveffect4_0, 0, 0);
dec(SVEffect4, sveffect4_1, 1, 0);
dec(SVEffect4, sveffect4_2, 0, 1);
dec(SVEffect4, sveffect4_3, 1, 1);
dec(SVEffect4, sveffect4_4, 0, 2);
dec(SVEffect4, sveffect4_5, 1, 2);
dec(SVEffect4, sveffect4_6, 0, 3);
dec(SVEffect4, sveffect4_7, 1, 3);
dec(SVEffect4, sveffect4_8, 0, 4);
dec(SVEffect4, sveffect4_9, 1, 4);
dec(SVEffect4, sveffect4_10, 0, 5);
dec(SVEffect4, sveffect4_11, 1, 5);
#endif

SVEffect4::SVEffect4(char const *tag, int32 o, int32 d)
{
    m_order = o;
    m_direction = d;
    m_tag = tag;
}

SVEffect4::~SVEffect4()
{
}

void    SVEffect4::Init()
{

    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 16, 16);
    m_numVerts = 4 * 256;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 256);

    m_numIndex = 6 * 256;
}

void    SVEffect4::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    int32 v = 0;
    uint32 permute[256];
    if (m_direction == 3)
    {
        Util::GeneratePermutation(permute, 256);
    }
    if (m_direction == 2 || m_direction == 1 || m_direction == 4)
    {
        h = w;
    }
    for (int32 y = 0; y < 16; y++)
    {
        int32 const y1 = y >= 8 ? 15 - y : y;
        for (int32 x = 0; x < 16; x++)
        {
            int32 const x1 = x >= 8 ? 15 - x : x;
            float frame = 0.0f;
            switch (m_direction)
            {
            case 3:
                frame = (float) (permute[y * 16 + x] & 127);
                break;

            case 4:
                frame = 7.0f - (float) min(y1, x1);
                break;

            default:
                break;
            }
            for (int32 o = 0; o < 4; o++)
            {
                s_localVertices[v].z = frame;
                s_localVertices[v++].ctrl = m_direction == 5 ? -1.0f : 0.0f;
            }
        }
    }

    memset(m_transforms, 0, 256 * sizeof(AATrans));

    for (int32 i = 0, t = 0; i < 16; i++)
    {
        for (int32 j = 0; j < 16; j++, t++)
        {
            t = i * 16 + j;

            float const y = (float) i * 0.125f - 0.9375f;
            float const x = (float) j * 0.125f - 0.9375f;

            float preX = x;
            float preY = y;
            float postX;
            float postY;
            float spread;
            float scale;

            switch (m_direction)
            {
            case 0:
            case 3:
            case 4:
            default:
                postX = 0.0f;
                postY = 0.0f;
                spread = 1.0f;
                scale = 0.35f;
                break;

            case 1:
                postX = x;
                postY = y;
                spread = 0.25f;
                scale = 0.45f;
                break;

            case 2:
                postX = -x;
                postY = -y;
                spread = 0.5f;
                scale = 0.35f;
                break;

            case 5:
                postX = 0.0f;
                postY = 0.0f;
                spread = 0.0f;
                scale = 0.0f;
                break;
            }
            GenerateRandomTrans(&transforms[t], w, h, preX, preY, postX, postY, spread, scale);
        }
    }
}

void    SVEffect4::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_direction == 3)
        {
            frameNum *= 2.0f;
            for (int32 i = 0; i < 128; i++)
            {
                frames[i] = frameNum;
                frameNum = max(frameNum - 2.0f, 0.0f);
            }
        }
        else if (m_direction == 4)
        {
            frameNum *= 2.0f;
            for (int32 i = 0; i < 8; i++)
            {
                frames[i] = frameNum;
                frameNum = max(frameNum - 32.0f, 0.0f);
            }
        }
        else
        {
            frames[0] = frameNum;
        }
    }
}

void    SVEffect4::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 512;
    }
}
