// Split roll 4 horizonatl blocks up and down

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

class SVEffect18 : public VEffect
{
public:
    SVEffect18(char const *tag, int32 o);
    virtual         ~SVEffect18();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    static Vertex   s_localVertices[4 * 8];
    static uint16   s_localIndices[6 * 8];
};

Vertex  SVEffect18::s_localVertices[4 * 8];
uint16  SVEffect18::s_localIndices[6 * 8];

#ifdef NDEBUG
dec(SVEffect18, sveffect18_0, 0);
dec(SVEffect18, sveffect18_1, 1);
#endif

SVEffect18::SVEffect18(char const *tag, int32 o)
{
    m_order = o;
    m_count = 2;
    m_tag = tag;
}

SVEffect18::~SVEffect18()
{
}

void    SVEffect18::Init()
{
    m_vertices = s_localVertices;
    GenerateRectangles(s_localVertices, 1, 4);
    memcpy(&s_localVertices[4 * 4], &s_localVertices[0], 4 * 4 * sizeof(Vertex));
    for (int32 i = 16; i < 32; i += 4)
    {
        float const o = i < 24 ? -0.5f : 0.5f;
        float const y1 = m_vertices[i].y;
        s_localVertices[i].y = m_vertices[i + 2].y + o;
        s_localVertices[i + 2].y = y1 + o;
        float const y2 = m_vertices[i + 1].y;
        s_localVertices[i + 1].y = m_vertices[i + 3].y + o;
        s_localVertices[i + 3].y = y2 + o;
    }

    m_numVerts = 4 * 8;

    m_indices = s_localIndices;
    GenerateIndices(s_localIndices, 8);

    m_numIndex = 6 * 8;
}

void    SVEffect18::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;

    memset(&transforms[0], 0, 8 * sizeof(AATrans));

    transforms[0].rotation.x = 1.0f;
    transforms[1].rotation.x = 1.0f;
    transforms[2].rotation.x = -1.0f;
    transforms[3].rotation.x = -1.0f;
    transforms[0].rotation.w = M_PI_2_f / 64.0f;
    transforms[1].rotation.w = M_PI_2_f / 64.0f;
    transforms[2].rotation.w = M_PI_2_f / 64.0f;
    transforms[3].rotation.w = M_PI_2_f / 64.0f;
    transforms[0].preTrans.y = 1.0f;
    transforms[1].preTrans.y = 0.5f;
    transforms[2].preTrans.y = -0.5f;
    transforms[3].preTrans.y = -1.0f;
    transforms[0].preTrans.w = 1.0f;
    transforms[1].preTrans.w = 1.0f;
    transforms[2].preTrans.w = 1.0f;
    transforms[3].preTrans.w = 1.0f;

    transforms[4].rotation.x = 1.0f;
    transforms[5].rotation.x = 1.0f;
    transforms[6].rotation.x = -1.0f;
    transforms[7].rotation.x = -1.0f;
    transforms[4].rotation.w = -M_PI_2_f / 64.0f;
    transforms[5].rotation.w = -M_PI_2_f / 64.0f;
    transforms[6].rotation.w = -M_PI_2_f / 64.0f;
    transforms[7].rotation.w = -M_PI_2_f / 64.0f;
    transforms[4].preTrans.y = 2.0f;
    transforms[5].preTrans.y = 1.0f;
    transforms[6].preTrans.y = -1.0f;
    transforms[7].preTrans.y = -2.0f;
    transforms[4].preTrans.w = 1.0f;
    transforms[5].preTrans.w = 1.0f;
    transforms[6].preTrans.w = 1.0f;
    transforms[7].preTrans.w = 1.0f;
}

void    SVEffect18::GenerateFrames(float frameNum, float *frames) const
{
    if (frameNum >= 128)
    {
        frameNum = 384 - frameNum;
    }
    VEffect::GenerateFrames(frameNum, frames);
}

void    SVEffect18::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        // Use numTri != NULL as a way to detect this is a phase 1 call, so we don't set any of these in phase 2
        if (baseTrans != NULL)
        {
            *baseTrans = frameNum >= 128 ? 4 : 0;
        }
        if (startIndex != NULL)
        {
            *startIndex = frameNum >= 128 ? 6 * 4 : 0;
        }
        *numTri = 8;
    }
}
