// Roll 4 vertical blocks side to side

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect19 : public VEffect
{
public:
    SVEffect19(char const *tag, int32 o, int32 d);
    virtual         ~SVEffect19();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;

    static Vertex   s_localVertices[4 * 16];
    static uint16   s_localIndices[6 * 16];
};

Vertex  SVEffect19::s_localVertices[4 * 16];
uint16  SVEffect19::s_localIndices[6 * 16];

#ifdef NDEBUG
dec(SVEffect19, sveffect19_0, 0, 0);
dec(SVEffect19, sveffect19_1, 1, 0);
dec(SVEffect19, sveffect19_2, 0, 1);
dec(SVEffect19, sveffect19_3, 1, 1);
#endif

SVEffect19::SVEffect19(char const *tag, int32 o, int32 d)
{
    m_order = o;
    m_direction = d;
    m_tag = tag;
}

SVEffect19::~SVEffect19()
{
}

void    SVEffect19::Init()
{
    static Vertex verts[4 * 16];
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 4, 1);

    memcpy(&s_localVertices[4 * 4], &s_localVertices[0], 4 * 4 * sizeof(Vertex));
    for (int32 i = 16; i < 32; i += 2)
    {
        float const x = s_localVertices[i].x;
        s_localVertices[i].x = s_localVertices[i + 1].x + 0.5f;
        s_localVertices[i + 1].x = x + 0.5f;
    }

    memcpy(&s_localVertices[4 * 8], &s_localVertices[0], 4 * 8 * sizeof(Vertex));

    m_numVerts = 4 * 16;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 16);

    m_numIndex = 6 * 16;
}

void    SVEffect19::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;

    memset(&transforms[0], 0, 16 * sizeof(AATrans));

    static struct
    {
        float axis;
        float pre[2];
        float post[4];
    } params[2] =
    {
        { -1.0f, { 1.0f, 0.5f }, { 0.0f, -1.0f, -1.0f, -2.0f } },
        {  1.0f, { 0.5f, 0.0f }, { 0.0f,  0.0f,  1.0f,  1.0f } },
    };

    int32 m = 0;
    for (int32 i = 0; i < 4; i++)
    {
        for (int32 j = 0; j < 4; j++)
        {
            transforms[m].rotation.y = params[m_direction].axis;
            transforms[m].rotation.w = M_PI_2_f / 32.0f;
            transforms[m].preTrans.x = params[m_direction].pre[i & 1] - (float) j * 0.5f;
            transforms[m].preTrans.w = 1.0f;
            transforms[m].postTrans.x = params[m_direction].post[i];
            transforms[m].postTrans.w = 1.0f;
            m++;
        }
    }
}

void    SVEffect19::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        frames[0] = frameNum == 256.0f ? 64.0f : fmod(frameNum, 64.0f);
    }
}

void    SVEffect19::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        // Use numTri != NULL as a way to detect this is a phase 1 call, so we don't set any of these in phase 2
        if (frameNum == 256.0f)
        {
            frameNum--;
        }
        uint32 const group = (uint32) frameNum / 64;
        if (baseTrans != NULL)
        {
            *baseTrans = group * 4;
        }
        if (startIndex != NULL)
        {
            *startIndex = group * 6 * 4;
        }
        *numTri = 8;
    }
}
