// Spin the image in or out in 16 pieces

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect3 : public VEffect
{
public:
    SVEffect3(char const *tag, int32 o, int32 d);
    virtual         ~SVEffect3();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;

    static Vertex   s_localVertices[4 * 16];
    static uint16   s_localIndices[6 * 16];
};

Vertex  SVEffect3::s_localVertices[4 * 16];
uint16  SVEffect3::s_localIndices[6 * 16];

#ifdef NDEBUG
dec(SVEffect3, sveffect3_0, 0, 0);
dec(SVEffect3, sveffect3_1, 1, 0);
dec(SVEffect3, sveffect3_2, 0, 1);
dec(SVEffect3, sveffect3_3, 1, 1);
dec(SVEffect3, sveffect3_4, 0, 2);
dec(SVEffect3, sveffect3_5, 1, 2);
dec(SVEffect3, sveffect3_6, 0, 3);
dec(SVEffect3, sveffect3_7, 1, 3);
#endif

SVEffect3::SVEffect3(char const *tag, int32 o, int32 d)
{
    m_order = o;
    m_direction = d;
    m_tag = tag;
}

SVEffect3::~SVEffect3()
{
}

void    SVEffect3::Init()
{
    m_vertices = s_localVertices;

    GenerateRectangles(s_localVertices, 4, 4);

    m_numVerts = 4 * 16;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 16);

    m_numIndex = 6 * 16;
}

void    SVEffect3::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;

    memset(m_transforms, 0, 16 * sizeof(AATrans));

    for (int32 i = 0; i < 16; i++)
    {
        float const y = (float) (i / 4) * 0.5f - 0.75f;
        float const x = (float) (i % 4) * 0.5f - 0.75f;

        float preX = x;
        float preY = y;
        float postX = 0.0f;
        float postY = 0.0f;
        float postSpread = 1.0f;
        float postScale = 0.4f;

        switch (m_direction)
        {
        case 0:
        default:
            break;

        case 1:
            postX = x;
            postY = y;
            postSpread = 0.25f;
            postScale = 0.4f;
            break;

        case 2:
            postX = -x;
            postY = -y;
            postSpread = 0.25f;
            postScale = 0.35f;
            break;

        case 3:
            postSpread = 0.0f;
            postScale = 0.0f;
            break;
        }

        GenerateRandomTrans(&transforms[i], w, h, preX, preY, postX, postY, postSpread, postScale);

        for (int32 v = 0; v < 4; v++)
        {
            s_localVertices[i * 4 + v].ctrl = m_direction == 3 ? -1.0f : 0.0f;
        }
    }
}

void    SVEffect3::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 16 * 2;
    }
}
