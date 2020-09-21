// Moving fan wipe about corners

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::max;
using std::swap;
using std::vector;

class SVEffect42 : public VEffect
{
public:
    SVEffect42(char const *tag, int32 o, int32 d, int32 s);
    virtual         ~SVEffect42();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;
    int32           m_sections;
    int32           m_rotationMode;
    float           m_dbStep;
    int32           m_numTri;

    bool            FindPoint(int32 which, float aspectRatio, float *x, float *y);
    float           MirrorPoint(float v, float m);
    void            GenerateTriangle(float x0, float y0, float x1, float y1, float x2, float y2, int32 which);

    static Vertex   s_localVertices[13 * 3];
    static uint16   s_localIndices[13 * 3];
};

Vertex  SVEffect42::s_localVertices[13 * 3];
uint16  SVEffect42::s_localIndices[13 * 3];

#ifdef NDEBUG
dec(SVEffect42, sveffect42_0, 0, 0, 6);
dec(SVEffect42, sveffect42_1, 1, 0, 6);
dec(SVEffect42, sveffect42_2, 0, 1, 6);
dec(SVEffect42, sveffect42_3, 1, 1, 6);
dec(SVEffect42, sveffect42_4, 0, 0, 12);
dec(SVEffect42, sveffect42_5, 1, 0, 12);
dec(SVEffect42, sveffect42_6, 0, 1, 12);
dec(SVEffect42, sveffect42_7, 1, 1, 12);
#endif

SVEffect42::SVEffect42(char const *tag, int32 o, int32 d, int32 s)
{
    m_order = o;
    m_direction = d;
    m_sections = s;
    m_tag = tag;
}

SVEffect42::~SVEffect42()
{
}

void    SVEffect42::Init()
{
    // Can't setup vertices during Init(), since the vertices we generate depend on the screen x and y dimensions.
    // They cannot be determined here, because in the event we have two contexts on two screens with different
    // aspect ratios, we'll have to change what gets generated every time GenerateTrans() gets called.
    m_vertices = s_localVertices;

    for (int32 i = 0; i < 13 * 3; i++)
    {
        s_localIndices[i] = (uint16) i;
    }
    m_indices = s_localIndices;
}

void    SVEffect42::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    int32 const corner = rand32();
    float const xm = corner & 8 ? 1.0f : -1.0f;
    float const ym = corner & 4 ? 1.0f : -1.0f;
    float const xc = MirrorPoint(0.0f, xm);
    float const yc = MirrorPoint(0.0f, ym);
    float const rotateDirection = xm * ym * (float) (m_direction * 2 - 1) / 256.0f;

    m_transforms = transforms;
    memset(&transforms[0], 0, m_sections * sizeof(AATrans));

    float const aspectRatio = (float) h / (float) w;

    m_rotationMode = corner & 1;
    m_dbStep = 0.005f * (0.5f - (float) (m_rotationMode | corner >> 1 & 1));
    m_numTri = 0;

    float x1, y1;
    bool v1 = m_direction ? FindPoint(0, aspectRatio, &x1, &y1) : FindPoint(m_sections, aspectRatio, &x1, &y1);
    x1 = MirrorPoint(x1, xm);
    y1 = MirrorPoint(y1, ym);

    for (int32 which = 0; which < m_sections; which++)
    {
        transforms[which].rotation.z = -1.0f;
        transforms[which].rotation.w = M_PI_2_f * rotateDirection;

        transforms[which].preTrans.x = xm;
        transforms[which].preTrans.y = ym;
        transforms[which].preTrans.w = 1.0f;

        float x2, y2;
        bool v2 = m_direction ? FindPoint(which + 1, aspectRatio, &x2, &y2) : FindPoint(m_sections - (which + 1), aspectRatio, &x2, &y2);
        x2 = MirrorPoint(x2, xm);
        y2 = MirrorPoint(y2, ym);

        GenerateTriangle(xc, yc, x1, y1, x2, y2, which);
        if (v1 != v2)
        {
            if (m_direction)
            {
                GenerateTriangle(x1, y2, x1, y1, x2, y2, which);
            }
            else
            {
                GenerateTriangle(x2, y1, x1, y1, x2, y2, which);
            }
        }

        x1 = x2;
        y1 = y2;
        v1 = v2;
    }
    m_numVerts = m_numTri * 3;
    m_numIndex = m_numTri * 3;
}

void    SVEffect42::GenerateFrames(float frameNum, float *frames) const
{
    float const frameDelta = 256.0f / (float) m_sections;
    if (frames != NULL)
    {
        for (int32 i = 0; i < m_sections; i++)
        {
            if (m_rotationMode)
            {
                frames[i] = frameNum;
                frameNum = max(frameNum - frameDelta, 0.0f);
            }
            else
            {
                float const rotateFactor = (float) (m_sections - i) / (float) m_sections;
                frames[i] = frameNum * rotateFactor;
            }
        }
    }
}

void    SVEffect42::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = m_numTri;
    }
}

bool    SVEffect42::FindPoint(int32 which, float aspectRatio, float *x, float *y)
{
    float const angle = M_PI_2_f * (float) which / (float) m_sections;
    float const dx = cos(angle);
    float const dy = sin(angle);
    // If which == 0 we've got one of the two horizontals, in which case set the xIntercept to huge to cause this
    // to be rejected, otherwise set it to dx / abs(dy) which is where it will intercept the y = +/- aspectRatio lines.
    // Using aspectRatio causes this to work within a screen aspect ratio rectangle.
    float const xIntercept = which == 0 ? 100.0f : (dx / dy) * aspectRatio;
    // Likewise if which == m_sections we have a vertical, so set the yIntercept to large
    float const yIntercept = which == m_sections ? 100.0f : dy / dx;
    float const xLenSQ = xIntercept * xIntercept + aspectRatio * aspectRatio;
    float const yLenSQ = yIntercept * yIntercept + 1.0f;
    if (xLenSQ < yLenSQ)
    {
        if (x != NULL)
        {
            *x = xIntercept; //  (xIntercept + 1.0f) * 0.5f;
        }
        if (y != NULL)
        {
            *y = 1.0f;
        }
        return false;
    }
    else
    {
        if (x != NULL)
        {
            *x = 1.0f;
        }
        if (y != NULL)
        {
            *y = yIntercept / aspectRatio; //  (yIntercept / aspectRatio + 1.0f) * 0.5f;
        }
        return true;
    }
}

float   SVEffect42::MirrorPoint(float v, float m)
{
    return v * m + (1.0f - m) * 0.5f;
}

void    SVEffect42::GenerateTriangle(float x0, float y0, float x1, float y1, float x2, float y2, int32 which)
{
    int32 vertexNum = m_numTri++ * 3;

    GenerateVertex(s_localVertices, vertexNum++, x0, y0, which, which, 0.1f + which * m_dbStep);
    GenerateVertex(s_localVertices, vertexNum++, x1, y1, which, which, 0.1f + which * m_dbStep);
    GenerateVertex(s_localVertices, vertexNum, x2, y2, which, which, 0.1f + which * m_dbStep);
}
