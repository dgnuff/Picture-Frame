// Edge Triangle wipe and fold up

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;

class SVEffect10 : public VEffect
{
public:
    SVEffect10(char const *tag, int32 o, int32 s);
    virtual         ~SVEffect10();
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

Vertex  SVEffect10::s_localVertices[4 * 4];
uint16  SVEffect10::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(SVEffect10, sveffect10_0, 0, 0);
dec(SVEffect10, sveffect10_1, 1, 0);
dec(SVEffect10, sveffect10_2, 0, 1);
dec(SVEffect10, sveffect10_3, 1, 1);
dec(SVEffect10, sveffect10_4, 0, 2);
dec(SVEffect10, sveffect10_5, 1, 2);
#endif

SVEffect10::SVEffect10(char const *tag, int32 o, int32 s)
{
    m_order = o;
    m_sequence = s;
    m_tag = tag;
}

SVEffect10::~SVEffect10()
{
}

void    SVEffect10::Init()
{
    m_vertices = s_localVertices;

    GenerateVertex(s_localVertices, 0, 0.0f, 0.0f, 0, 0);
    GenerateVertex(s_localVertices, 1, 0.5f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices, 2, 0.0f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices, 3, 0.0f, 1.0f, 0, 0);

    GenerateVertex(s_localVertices, 4, 0.0f, 1.0f, 1, 1);
    GenerateVertex(s_localVertices, 5, 0.5f, 0.5f, 1, 1);
    GenerateVertex(s_localVertices, 6, 0.5f, 1.0f, 1, 1);
    GenerateVertex(s_localVertices, 7, 1.0f, 1.0f, 1, 1);

    GenerateVertex(s_localVertices, 8, 1.0f, 1.0f, 2, 2);
    GenerateVertex(s_localVertices, 9, 0.5f, 0.5f, 2, 2);
    GenerateVertex(s_localVertices, 10, 1.0f, 0.5f, 2, 2);
    GenerateVertex(s_localVertices, 11, 1.0f, 0.0f, 2, 2);

    GenerateVertex(s_localVertices, 12, 1.0f, 0.0f, 3, 3);
    GenerateVertex(s_localVertices, 13, 0.5f, 0.5f, 3, 3);
    GenerateVertex(s_localVertices, 14, 0.5f, 0.0f, 3, 3);
    GenerateVertex(s_localVertices, 15, 0.0f, 0.0f, 3, 3);

    m_numVerts = 4 * 4;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
}

void    SVEffect10::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;

    uint32 p;
    int32 r;
    switch (m_sequence)
    {
    case 0:
    default:
        break;

    case 1:
        Util::GeneratePermutation(m_permute, 4);
        break;

    case 2:
        p = rand32();
        r = p & 4;
        for (int32 i = 0; i < 4; i++)
        {
            m_permute[i] = r ? 3 - p++ % 4 : p++ % 4;
        }
        break;
    }

    memset(&transforms[0], 0, 4 * sizeof(AATrans));

    switch (rnd32(3))
    {
    case 0:
    default:
        transforms[0].rotation.y = 1.0f;
        transforms[0].rotation.w = (float) -M_PI_2_f / 128.0f;
        transforms[0].preTrans.x = 1.0f;
        transforms[0].preTrans.w = 1.0f;

        transforms[1].rotation.x = -1.0f;
        transforms[1].rotation.w = (float) M_PI_2_f / 128.0f;
        transforms[1].preTrans.y = -1.0f;
        transforms[1].preTrans.w = 1.0f;

        transforms[2].rotation.y = -1.0f;
        transforms[2].rotation.w = (float) -M_PI_2_f / 128.0f;
        transforms[2].preTrans.x = -1.0f;
        transforms[2].preTrans.w = 1.0f;

        transforms[3].rotation.x = 1.0f;
        transforms[3].rotation.w = (float) M_PI_2_f / 128.0f;
        transforms[3].preTrans.y = 1.0f;
        transforms[3].preTrans.w = 1.0f;
        break;

    case 1:
        transforms[0].postTrans.x = -1.0f / 128.0f;
        transforms[1].postTrans.y = 1.0f / 128.0f;
        transforms[2].postTrans.x = 1.0f / 128.0f;
        transforms[3].postTrans.y = -1.0f / 128.0f;
        break;

    case 2:
        for (int32 i = 0; i < 4; i++)
        {
            static float x[4] = { -0.5f,  0.0f,  0.5f,  0.0f, };
            static float y[4] = {  0.0f,  0.5f,  0.0f, -0.5f, };
            GenerateRandomTrans(&transforms[i], w, h, x[i], y[i], -x[i], -y[i], 0.7f, 1.0f);
        }
        break;
    }
}

void    SVEffect10::GenerateFrames(float frameNum, float *frames) const
{
    if (frames == NULL)
    {
        return;
    }

    switch (m_sequence)
    {
    case 0:
    default:
        for (int32 i = 0; i < 4; i++)
        {
            frames[i] = frameNum / 2.0f;
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

void    SVEffect10::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 2 * 4;
    }
}
