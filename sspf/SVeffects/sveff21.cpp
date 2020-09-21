// Shear of the whole image from an edge

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::min;
using std::max;

class SVEffect21 : public VEffect
{
public:
    SVEffect21(char const *tag, int32 o, int32 e);
    virtual         ~SVEffect21();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;

private:
    int32           m_edge;
    int32           m_which;
    float           m_limit;

    static Vertex   s_localVertices[4][4];
};

Vertex  SVEffect21::s_localVertices[4][4];

#ifdef NDEBUG
dec(SVEffect21, sveffect21_0, 0, 0);
dec(SVEffect21, sveffect21_1, 1, 0);
dec(SVEffect21, sveffect21_2, 0, 1);
dec(SVEffect21, sveffect21_3, 1, 1);
dec(SVEffect21, sveffect21_4, 0, 2);
dec(SVEffect21, sveffect21_5, 1, 2);
dec(SVEffect21, sveffect21_6, 0, 3);
dec(SVEffect21, sveffect21_7, 1, 3);
#endif

SVEffect21::SVEffect21(char const *tag, int32 o, int32 e)
{
    m_order = o;
    m_edge = e;
    m_tag = tag;
}

SVEffect21::~SVEffect21()
{
}

void    SVEffect21::Init()
{
    Vertex *vertices;
    vertices = &s_localVertices[m_edge][0];
    memcpy(vertices, s_vertices, 4 * sizeof(Vertex));

    if (m_edge & 2)
    {
        vertices[1].z = 1.0f;
        vertices[3].z = 1.0f;
    }
    else
    {
        vertices[2].z = 1.0f;
        vertices[3].z = 1.0f;
    }

    m_vertices = vertices;
    m_numVerts = 4;

    m_indices = s_indices;
    m_numIndex = 6;
}

void    SVEffect21::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, sizeof(AATrans));
    if (m_edge & 2)
    {
        transforms[0].postTrans.y = m_edge & 1 ? 2.0f / 128.0f : -2.0f / 128.0f;
    }
    else
    {
        transforms[0].postTrans.x = m_edge & 1 ? 2.0f / 128.0f : -2.0f / 128.0f;
    }
    uint32 const r = rand32();
    m_which = r & 1;
    m_limit = r & 2 ? 128.0f : 1024.0f;
}

void    SVEffect21::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (m_which)
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
