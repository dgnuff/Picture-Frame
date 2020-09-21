// Dual spin the image in and out in 4 pieces

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class DVeffect5 : public VEffect
{
public:
    DVeffect5(char const *tag, int32 s, int32 t);
    virtual         ~DVeffect5();
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

Vertex  DVeffect5::s_localVertices[2][4 * 4];
uint16  DVeffect5::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(DVeffect5, dveffect5_0, 0, 0);
dec(DVeffect5, dveffect5_1, 1, 0);
dec(DVeffect5, dveffect5_2, 0, 1);
dec(DVeffect5, dveffect5_3, 1, 1);
dec(DVeffect5, dveffect5_4, 0, 2);
dec(DVeffect5, dveffect5_5, 1, 2);
dec(DVeffect5, dveffect5_6, 0, 3);
dec(DVeffect5, dveffect5_7, 1, 4);
#endif

DVeffect5::DVeffect5(char const *tag, int32 s, int32 t)
{
    m_sequence = s;
    m_type = t;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVeffect5::~DVeffect5()
{
}

void    DVeffect5::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];
    GenerateRectangles(s_localVertices[0], 2, 2);
    memcpy(s_localVertices[1], s_localVertices[0], 4 * 4 * sizeof(Vertex));

    m_numVerts = 4 * 4;
    m_numVerts_p2 = 4 * 4;

    m_indices = s_localIndices;
    m_indices_p2 = s_localIndices;
    GenerateIndices(s_localIndices, 4);

    m_numIndex = 6 * 4;
    m_numIndex_p2 = 6 * 4;
}

void    DVeffect5::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 5 * sizeof(AATrans));

    if (m_type == 0 || m_type == 1)
    {
        float const spread = m_type == 0 ? 1.0f : 0.0f;
        float const scale = m_type == 0 ? 0.5f : 0.0f;
        for (int32 i = 0; i < 4; i++)
        {
            float const x = m_type == 0 ? (float) (i % 2) - 0.5f : 0.0f;
            float const y = m_type == 0 ? (float) (i / 2) - 0.5f : 0.0f;
            GenerateRandomTrans(&transforms[i], w, h, x, y, 0.0f, 0.0f, spread, scale);
        }
    }
    else
    {
        uint32 r = rand32();
        for (int32 i = 0; i < 4; i++)
        {
            float const s = rndf32() * (M_2PI_f + M_PI_f) + M_PI_f;
            float const d = m_type == 2 ? 0.0f : r & 1 ? s / 256.0f : -s / 256.0f;
            r >>= 1;
            transforms[i].rotation.z = 1.0f;
            transforms[i].rotation.w = d;
        }
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

    uint32 which[4];
    Util::RandomHalf(which, 4);
    uint32 p = rand32();
    float const ctrl = m_type == 0 ? 0.0f : -1.0f;

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
                s_localVertices[1][i4 + j].ctrl = ctrl;
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
                s_localVertices[0][i4 + j].ctrl = ctrl;
                s_localVertices[1][i4 + j].z = 0.0f;
                s_localVertices[1][i4 + j].w = 4.0f;
                s_localVertices[1][i4 + j].db = 0.5f;
                s_localVertices[1][i4 + j].ctrl = 0.0f;
            }
            m_permute[((p ^= 2) & 2) + 1] = i;
        }
    }
}

void    DVeffect5::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
{
    if (frames == NULL || frames_p2 == NULL)
    {
        return;
    }

    if (m_sequence == 0)
    {
        for (int32 i = 0; i < 8; i++)
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
            frameNum -= 64;
        }
    }
}

void    DVeffect5::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4 * 2;
    }
    if (numTri_p2 != NULL)
    {
        *numTri_p2 = 4 * 2;
    }
}
