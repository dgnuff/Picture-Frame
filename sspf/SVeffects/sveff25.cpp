// Rotate 2 by 2 squares about the centers of edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class SVEffect25 : public VEffect
{
public:
    SVEffect25(char const *tag, int32 o, int32 e, int32 s);
    virtual         ~SVEffect25();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_sequence;
    uint32          m_permute[4];

    static Vertex   s_localVertices[4 * 4];
    static uint16   s_localIndices[6 * 4];
};

Vertex  SVEffect25::s_localVertices[4 * 4];
uint16  SVEffect25::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(SVEffect25, sveffect25_0, 0, 0, 0);
dec(SVEffect25, sveffect25_1, 1, 0, 0);
dec(SVEffect25, sveffect25_2, 0, 1, 0);
dec(SVEffect25, sveffect25_3, 1, 1, 0);
dec(SVEffect25, sveffect25_4, 0, 0, 1);
dec(SVEffect25, sveffect25_5, 1, 0, 1);
dec(SVEffect25, sveffect25_6, 0, 1, 1);
dec(SVEffect25, sveffect25_7, 1, 1, 1);
#endif

SVEffect25::SVEffect25(char const *tag, int32 o, int32 e, int32 s)
{
    m_order = o;
    m_edge = e;
    m_sequence = s;
    m_tag = tag;
}

SVEffect25::~SVEffect25()
{
}

void    SVEffect25::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 2, 2);

    for (int32 i = 0; i < 4; i++)
    {
        int32 const i4 = i * 4;
        for (int32 j = 0; j < 4; j++)
        {
            s_localVertices[i4 + j].z = (float) i;
        }
    }
    m_numVerts = 4 * 4;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
}

void    SVEffect25::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 4 * sizeof(AATrans));
    for (int32 i = 0; i < 4; i++)
    {
        float const rotationDir = i == 0 || i == 3 ? -1.0f : 1.0f;
        if (m_edge)
        {
            transforms[i].rotation.z = 1.0f;
            transforms[i].rotation.w = (float) M_PI_2_f * rotationDir / 256.0f;
            transforms[i].preTrans.x = 0.0f;
            transforms[i].preTrans.y = 1.0f - (float) (i & 2);
            transforms[i].preTrans.w = 1.0f;
        }
        else
        {
            transforms[i].rotation.z = 1.0f;
            transforms[i].rotation.w = (float) M_PI_2_f * -rotationDir / 256.0f;
            transforms[i].preTrans.x = 1.0f - (float) (i & 1) * 2.0f;
            transforms[i].preTrans.y = 0.0f;
            transforms[i].preTrans.w = 1.0f;
        }
    }
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

void    SVEffect25::GenerateFrames(float frameNum, float *frames) const
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

void    SVEffect25::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4 * 2;
    }
}
