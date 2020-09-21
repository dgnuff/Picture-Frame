// Dual Edge Triangle fold up and spin out

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class DVEffect14 : public VEffect
{
public:
    DVEffect14(char const *tag, int32 s, int32 t);
    virtual         ~DVEffect14();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames, float *frames_p2) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_sequence;
    int32           m_type;
    uint32          m_permute[4];

    static Vertex   s_localVertices[2][4 * 4];
    static uint16   s_localIndices[6 * 4];
};

Vertex  DVEffect14::s_localVertices[2][4 * 4];
uint16  DVEffect14::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(DVEffect14, dveffect14_0, 0, 0);
dec(DVEffect14, dveffect14_1, 1, 0);
dec(DVEffect14, dveffect14_2, 0, 1);
dec(DVEffect14, dveffect14_3, 1, 1);
dec(DVEffect14, dveffect14_4, 0, 2);
dec(DVEffect14, dveffect14_5, 1, 2);
#endif

DVEffect14::DVEffect14(char const *tag, int32 s, int32 t)
{
    m_sequence = s;
    m_type = t;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect14::~DVEffect14()
{
}

void    DVEffect14::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];

    GenerateVertex(s_localVertices[0], 0, 0.0f, 0.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 1, 0.5f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 2, 0.0f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 3, 0.0f, 1.0f, 0, 0);

    GenerateVertex(s_localVertices[0], 4, 0.0f, 1.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 5, 0.5f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 6, 0.5f, 1.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 7, 1.0f, 1.0f, 0, 0);

    GenerateVertex(s_localVertices[0], 8, 1.0f, 1.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 9, 0.5f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 10, 1.0f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 11, 1.0f, 0.0f, 0, 0);

    GenerateVertex(s_localVertices[0], 12, 1.0f, 0.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 13, 0.5f, 0.5f, 0, 0);
    GenerateVertex(s_localVertices[0], 14, 0.5f, 0.0f, 0, 0);
    GenerateVertex(s_localVertices[0], 15, 0.0f, 0.0f, 0, 0);

    memcpy(s_localVertices[1], s_localVertices[0], 4 * 4 * sizeof(Vertex));

    m_numVerts = 4 * 4;
    m_numVerts_p2 = 4 * 4;

    m_indices = s_localIndices;
    m_indices_p2 = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
    m_numIndex_p2 = 6 * 4;
}

void    DVEffect14::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 5 * sizeof(AATrans));

    static const float x[4] = { -0.5f, 0.0f, 0.5f,  0.0f, };
    static const float y[4] = { 0.0f, 0.5f, 0.0f, -0.5f, };

    float const divisor = rand32() & 1 ? 1.0f / 256.0f : 1.0f / 128.0f;

    switch (m_type)
    {
    case 0:
    default:
        transforms[0].rotation.y = 1.0f;
        transforms[0].rotation.w = -M_PI_2_f * divisor;
        transforms[0].preTrans.x = 1.0f;
        transforms[0].preTrans.w = 1.0f;

        transforms[1].rotation.x = -1.0f;
        transforms[1].rotation.w = M_PI_2_f * divisor;
        transforms[1].preTrans.y = -1.0f;
        transforms[1].preTrans.w = 1.0f;

        transforms[2].rotation.y = -1.0f;
        transforms[2].rotation.w = -M_PI_2_f * divisor;
        transforms[2].preTrans.x = -1.0f;
        transforms[2].preTrans.w = 1.0f;

        transforms[3].rotation.x = 1.0f;
        transforms[3].rotation.w = M_PI_2_f * divisor;
        transforms[3].preTrans.y = 1.0f;
        transforms[3].preTrans.w = 1.0f;
        break;

    case 1:
        transforms[0].postTrans.x = -divisor;
        transforms[1].postTrans.y = divisor;
        transforms[2].postTrans.x = divisor;
        transforms[3].postTrans.y = -divisor;
        break;

    case 2:
        for (int32 i = 0; i < 4; i++)
        {
            GenerateRandomTrans(&transforms[i], w, h, x[i], y[i], x[i] * -2.0f, y[i] * -2.0f, 0.5f, 0.5f);
        }
        break;
    }

    uint32 which[4];
    Util::RandomHalf(which, 4);
    uint32 p = rand32();

    for (int32 i = 0; i < 4; i++)
    {
        int32 const i4 = i * 4;
        if (which[i])
        {
            for (int32 j = 0; j < 4; j++)
            {
                s_localVertices[0][i4 + j].z = 0.0f;
                s_localVertices[0][i4 + j].w = 4.0f;
                s_localVertices[0][i4 + j].db = 0.5f;
                s_localVertices[0][i4 + j].ctrl = 0.0f;
                s_localVertices[1][i4 + j].z = (float) i;
                s_localVertices[1][i4 + j].w = (float) i;
                s_localVertices[1][i4 + j].db = 0.0f;
                s_localVertices[1][i4 + j].ctrl = 0.0f;
            }
            m_permute[((p ^= 1) & 1) * 2] = i;
        }
        else
        {
            for (int32 j = 0; j < 4; j++)
            {
                s_localVertices[0][i4 + j].z = (float) i;
                s_localVertices[0][i4 + j].w = (float) i;
                s_localVertices[0][i4 + j].db = 0.0f;
                s_localVertices[0][i4 + j].ctrl = 0.0f;
                s_localVertices[1][i4 + j].z = 0.0f;
                s_localVertices[1][i4 + j].w = 4.0f;
                s_localVertices[1][i4 + j].db = 0.5f;
                s_localVertices[1][i4 + j].ctrl = 0.0f;
            }
            m_permute[((p ^= 2) & 2) + 1] = i;
        }
    }
}

void    DVEffect14::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames == NULL || frames_p2 == NULL)
    {
        return;
    }

    if (m_sequence == 0)
    {
        for (int32 i = 0; i < 4; i++)
        {
            frames[i] = frameNum;
            frames_p2[i] = 256.0f - frameNum;
        }
    }
    else
    {
        for (int32 i = 0; i < 2; i++)
        {
            uint32 const p = m_permute[i * 2];
            uint32 const p2 = m_permute[i * 2 + 1];
            float const f = Util::Clamp(frameNum, 0.0f, 128.0f) * 2.0f;
            float const f2 = 256.0f - f;
            frames[p] = f;
            frames_p2[p] = f2;
            frames[p2] = f;
            frames_p2[p2] = f2;
            frameNum -= 64.0f;
        }
    }
}

void    DVEffect14::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 2 * 4;
    }
    if (numTri_p2 != NULL)
    {
        *numTri_p2 = 2 * 4;
    }
}
