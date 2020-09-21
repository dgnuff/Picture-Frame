// V shaped shear of the whole image from side edges

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::min;
using std::max;

class SVEffect22 : public VEffect
{
public:
    SVEffect22(char const *tag, int32 o, int32 e, int32 p);
    virtual         ~SVEffect22();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_edge;
    int32           m_point;
    float           m_limit;

    static Vertex   s_localVertices[4 * 2];
    static uint16   s_localIndices[6 * 2];
};

Vertex  SVEffect22::s_localVertices[4 * 2];
uint16  SVEffect22::s_localIndices[6 * 2];

#ifdef NDEBUG
dec(SVEffect22, sveffect22_0, 0, 0, 0);
dec(SVEffect22, sveffect22_1, 1, 0, 0);
dec(SVEffect22, sveffect22_2, 0, 1, 0);
dec(SVEffect22, sveffect22_3, 1, 1, 0);
dec(SVEffect22, sveffect22_4, 0, 0, 1);
dec(SVEffect22, sveffect22_5, 1, 0, 1);
dec(SVEffect22, sveffect22_6, 0, 1, 1);
dec(SVEffect22, sveffect22_7, 1, 1, 1);
#endif

SVEffect22::SVEffect22(char const *tag, int32 o, int32 e, int32 p)
{
    m_order = o;
    m_edge = e;
    m_point = p;
    m_tag = tag;
}

SVEffect22::~SVEffect22()
{
}

void    SVEffect22::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 1, 2);

    for (int32 i = 0; i < 4; i++)
    {
        s_localVertices[i + 2].z = 1.0f;
        s_localVertices[i + 4].w = 0.0f;
    }

    m_numVerts = 4 * 2;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 2);

    m_numIndex = 6 * 2;
}

void    SVEffect22::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    transforms[0].postTrans.x = m_edge ? 2.0f / 128.0f : -2.0f / 128.0f;
    m_limit = rand32() & 1 ? 128.0f : 1024.0f;
}

void    SVEffect22::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_point)
        {
            frames[0] = min(frameNum, m_limit);
            frames[1] = max(frameNum - 128.0f, 0.0f);
        }
        else
        {
            frames[0] = max(frameNum - 128.0f, 0.0f);
            frames[1] = min(frameNum, m_limit);
        }
    }
}

void    SVEffect22::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = 4;
    }
}
