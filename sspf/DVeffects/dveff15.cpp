// Edge Triangle shrink

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

class DVEffect15 : public VEffect
{
public:
    DVEffect15(char const *tag, int32 s, int32 t);
    virtual         ~DVEffect15();
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

Vertex  DVEffect15::s_localVertices[2][4 * 4];
uint16  DVEffect15::s_localIndices[6 * 4];

#ifdef NDEBUG
dec(DVEffect15, dveffect15_0, 0, 0);
dec(DVEffect15, dveffect15_1, 1, 0);
dec(DVEffect15, dveffect15_2, 0, 1);
dec(DVEffect15, dveffect15_3, 1, 1);
dec(DVEffect15, dveffect15_4, 0, 2);
dec(DVEffect15, dveffect15_5, 1, 2);
#endif

DVEffect15::DVEffect15(char const *tag, int32 s, int32 t)
{
    m_sequence = s;
    m_type = t;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVEffect15::~DVEffect15()
{
}

void    DVEffect15::Init()
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

void    DVEffect15::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, 5 * sizeof(AATrans));

    int32 r;
    switch (m_type)
    {
    case 0:
    default:
        break;

    case 1:
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

    case 2:
        for (int32 i = 0; i < 4; i++)
        {
            GenerateRandomTrans(&transforms[i], w, h, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
        break;
    }

    transforms[0].preTrans.x = 0.5f;
    transforms[0].preTrans.w = 1.0f;

    transforms[1].preTrans.y = -0.5f;
    transforms[1].preTrans.w = 1.0f;

    transforms[2].preTrans.x = -0.5f;
    transforms[2].preTrans.w = 1.0f;

    transforms[3].preTrans.y = 0.5f;
    transforms[3].preTrans.w = 1.0f;

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
                s_localVertices[1][i4 + j].ctrl = -1.0f;
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
                s_localVertices[0][i4 + j].ctrl = -1.0f;
                s_localVertices[1][i4 + j].z = 0.0f;
                s_localVertices[1][i4 + j].w = 4.0f;
                s_localVertices[1][i4 + j].db = 0.5f;
                s_localVertices[1][i4 + j].ctrl = 0.0f;
            }
            m_permute[((p ^= 2) & 2) + 1] = i;
        }
    }
}

void    DVEffect15::GenerateFrames(float frameNum, float *frames, float *frames_p2) const
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

void    DVEffect15::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
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
