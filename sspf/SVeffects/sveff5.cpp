// Rotate 2 by 2 squares about the corners

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect5 : public VEffect
{
public:
    SVEffect5(char const *tag, int32 o, int32 d, int32 a);
    virtual         ~SVEffect5();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    float           m_rotationDir;
    int32           m_alternating;
    int32           m_sequence;
    uint32          m_permute[4];

    static Vertex   s_localVertices[4 * 4];
    static uint16   s_localIndices[6 * 4];
};

Vertex  SVEffect5::s_localVertices[4 * 4];
uint16  SVEffect5::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(SVEffect5, sveffect5_0, 0,  1, 0);
dec(SVEffect5, sveffect5_1, 1,  1, 0);
dec(SVEffect5, sveffect5_2, 0, -1, 0);
dec(SVEffect5, sveffect5_3, 1, -1, 0);
dec(SVEffect5, sveffect5_4, 0,  1, 1);
dec(SVEffect5, sveffect5_5, 1,  1, 1);
dec(SVEffect5, sveffect5_6, 0, -1, 1);
dec(SVEffect5, sveffect5_7, 1, -1, 1);
#endif

SVEffect5::SVEffect5(char const *tag, int32 o, int32 d, int32 a)
{
    m_order = o;
    m_rotationDir = (float) d;
    m_alternating = a;
    m_tag = tag;
}

SVEffect5::~SVEffect5()
{
}

void    SVEffect5::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 2, 2);

    for (int32 i = 0; i < 16; i++)
    {
        float const i4 = (float)(i / 4);
        float const offset = -i4 / 64.0f;
        s_localVertices[i].db = offset;
        s_localVertices[i].z = i4;
    }

    m_numVerts = 4 * 4;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
}

void    SVEffect5::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 4 * sizeof(AATrans));
    for (int32 i = 0; i < 4; i++)
    {
        float const sign = m_alternating && (i == 1 || i == 2) ? -1.0f : 1.0f;
        float const x = -1.0f + (float) (i & 1) * 2.0f;
        float const y = -1.0f + (float) (i & 2);
        transforms[i].rotation.z = 1.0f;
        transforms[i].rotation.w = M_PI_2_f * m_rotationDir * sign / 256.0f;
        transforms[i].preTrans.x = -x;
        transforms[i].preTrans.y = -y;
        transforms[i].preTrans.w = 1.0f;
    }
    if (m_sequence = rand32() & 1)
    {
        int32 const dir = rnd32(3) - 1;
        if (dir == 0)
        {
            Util::GeneratePermutation(m_permute, 4);
        }
        else
        {
            int32 p = rand32();
            for (int32 i = 0; i < 4; i++)
            {
                m_permute[i] = p = p + dir & 3;
            }
        }
    }
}

void    SVEffect5::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_sequence)
        {
            for (int32 i = 0; i < 4; i++)
            {
                int32 const i2 = m_permute[i];
                float const f = Util::Clamp(frameNum * 2.0f, 0.0f, 256.0f);
                frames[i2] = f;
                frameNum -= 32;
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
}

void    SVEffect5::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4 * 2;
    }
}
