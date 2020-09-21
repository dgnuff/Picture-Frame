// Dual spin the image in and out in 16 pieces

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "veffect.h"

using std::max;
using std::min;

class DVeffect4 : public VEffect
{
public:
    DVeffect4(char const *tag, int32 d);
    virtual         ~DVeffect4();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;

    static Vertex   s_localVertices[2][4 * 16];
    static uint16   s_localIndices[6 * 16];
};

Vertex  DVeffect4::s_localVertices[2][4 * 16];
uint16  DVeffect4::s_localIndices[6 * 16];

#ifdef NDEBUG
dec(DVeffect4, dveffect4_0, 0);
dec(DVeffect4, dveffect4_1, 1);
dec(DVeffect4, dveffect4_2, 2);
#endif

DVeffect4::DVeffect4(char const *tag, int32 d)
{
    m_direction = d;
    m_count = 2;
    m_usages = IS_DOUBLE;
    m_tag = tag;
}

DVeffect4::~DVeffect4()
{
}

void    DVeffect4::Init()
{
    m_vertices = s_localVertices[0];
    m_vertices_p2 = s_localVertices[1];
    GenerateRectangles(s_localVertices[0], 4, 4);
    memcpy(s_localVertices[1], s_localVertices[0], 4 * 16 * sizeof(Vertex));

    m_numVerts = 4 * 16;
    m_numVerts_p2 = 4 * 16;

    m_indices = s_localIndices;
    m_indices_p2 = s_localIndices;
    GenerateIndices(s_localIndices, 16);

    m_numIndex = 6 * 16;
    m_numIndex_p2 = 6 * 16;
}

void    DVeffect4::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    uint32 which[16];
    Util::RandomHalf(which, 16);

    for (int32 i = 0; i < 16; i++)
    {
        int32 const i4 = i * 4;
        if (which[i])
        {
            for (int32 j = 0; j < 4; j++)
            {
                s_localVertices[0][i4 + j].w = 16.0f;
                s_localVertices[0][i4 + j].db = 0.5f;
                s_localVertices[1][i4 + j].w = (float) i;
                s_localVertices[1][i4 + j].db = 0.0f;
            }
        }
        else
        {
            for (int32 j = 0; j < 4; j++)
            {
                s_localVertices[0][i4 + j].w = (float) i;
                s_localVertices[0][i4 + j].db = 0.0f;
                s_localVertices[1][i4 + j].w = 16.0f;
                s_localVertices[1][i4 + j].db = 0.5f;
            }
        }
    }

    m_transforms = transforms;
    for (int32 i = 0; i < 16; i++)
    {
        int32 const y = i / 4;
        int32 const x = i % 4;
        float biasx;
        float biasy;
        float spread;
        float scale;

        switch (m_direction)
        {
        case 0:
        default:
            biasx = 0.0f;
            biasy = 0.0f;
            spread = 1.0f;
            scale = 0.4f;
            break;

        case 1:
            biasx = -0.75f + (float) x * 0.5f;
            biasy = -0.75f + (float) y * 0.5f;
            spread = 0.25f;
            scale = 0.4f;
            break;

        case 2:
            biasx = 0.75f - (float) x * 0.5f;
            biasy = 0.75f - (float) y * 0.5f;
            spread = 0.25f;
            scale = 0.35f;
            break;
        }
        GenerateRandomTrans(&transforms[i], w, h, (float) x * 0.5f - 0.75f, (float) y * 0.5f - 0.75f, biasx, biasy, spread, scale);

        if (m_direction == 3)
        {
            transforms[i].preTrans.x = (float) x * -0.5f + 0.75f;
            transforms[i].preTrans.y = (float) y * -0.5f + 0.75f;
            transforms[i].preTrans.w = 1.0f;
        }
    }

    memset(&transforms[16], 0, sizeof(AATrans));
}

void    DVeffect4::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16 * 2;
    }
    if (numTri_p2 != NULL)
    {
        *numTri_p2 = 16 * 2;
    }
}
