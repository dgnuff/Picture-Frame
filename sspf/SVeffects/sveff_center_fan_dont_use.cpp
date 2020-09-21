// Center Fan

#include "pch.h"

#include "sspf.h"
#include "veffect.h"

using std::max;

class SVEffect43 : public VEffect
{
public:
    SVEffect43(char const *tag, int32 o, int32 d, int32 s);
    virtual         ~SVEffect43();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;
    int32           m_sections;
    int32           m_numTri;

    bool            FindPoint(int32 which, float aspectRatio, float *x, float *y);

    static Vertex   s_localVertices[36 * 3];
    static uint16   s_localIndices[36 * 3];
};

Vertex  SVEffect43::s_localVertices[36 * 3];
uint16  SVEffect43::s_localIndices[36 * 3];

#ifdef NDEBUG
//dec(SVEffect43, sveffect43_0, 0, -1, 32);
//dec(SVEffect43, sveffect43_1, 0, 1, 32);
//dec(SVEffect43, sveffect43_2, 0, 8, 1);
//dec(SVEffect43, sveffect43_3, 1, 8, 1);

//dec(SVEffect43, sveffect43_2, 0, 1);
//dec(SVEffect43, sveffect43_3, 1, 1);
//dec(SVEffect43, sveffect43_4, 0, 2);
//dec(SVEffect43, sveffect43_5, 1, 2);
#endif

SVEffect43::SVEffect43(char const *tag, int32 o, int32 d, int32 s)
{
    m_order = o;
    m_direction = d;
    m_sections = s;
    m_tag = tag;
}

SVEffect43::~SVEffect43()
{
}

void    SVEffect43::Init()
{
    m_vertices = s_localVertices;
    m_indices = s_localIndices;
}

void    SVEffect43::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    m_transforms = transforms;
    memset(&transforms[0], 0, m_sections * sizeof(AATrans));

    float const aspectRatio = (float) h / (float) w;

    int32 const basis = (rand32() & 3) * m_sections / 4;
    int32 vertexNum = 0;
    m_numTri = 0;

    float const dbStep = 0.002f;

    float x1, y1;
    bool v1 = FindPoint(basis * m_direction, aspectRatio, &x1, &y1);

    for (int32 which = 0; which < m_sections; which++)
    {
        float const rotateFactor = (float) (m_sections - which) / (float) m_sections;
        transforms[which].rotation.z = -1.0f;
        transforms[which].rotation.w = M_2PI_f * rotateFactor * (float) m_direction / 256.0f;

        float x2, y2;
        bool v2 = FindPoint((which + basis + 1) * m_direction, aspectRatio, &x2, &y2);
        GenerateVertex(s_localVertices, vertexNum, 0.5f, 0.5f, which, which, 0.1f + which * dbStep);
        s_localIndices[vertexNum] = (uint16) vertexNum;
        vertexNum++;
        GenerateVertex(s_localVertices, vertexNum, x1, y1, which, which, 0.1f + which * dbStep);
        s_localIndices[vertexNum] = (uint16) vertexNum;
        vertexNum++;
        GenerateVertex(s_localVertices, vertexNum, x2, y2, which, which, 0.1f + which * dbStep);
        s_localIndices[vertexNum] = (uint16) vertexNum;
        vertexNum++;
        m_numTri++;
        x1 = x2;
        y1 = y2;
        v1 = v2;
    }
    m_numVerts = vertexNum;
    m_numIndex = vertexNum;
}

void    SVEffect43::GenerateFrames(float frameNum, float *frames) const
{
    float const frameDelta = (float) (256 / m_sections);
    if (frames != NULL)
    {
        for (int32 i = 0; i < m_sections; i++)
        {
            frames[i] = frameNum;
            //frameNum = max(frameNum - frameDelta, 0.0f);
        }
    }
}

void    SVEffect43::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = m_numTri;
    }
}

bool    SVEffect43::FindPoint(int32 which, float aspectRatio, float *x, float *y)
{
    which &= m_sections - 1;
    float const angle = M_2PI_f * (float) which / (float) m_sections;
    float const dx = cos(angle);
    float const dy = sin(angle);
    // If dy == 0 we've got one of the two horizontals, in which case set the xIntercept to huge to cause this
    // to be rejected, otherwise set it to dx / abs(dy) which is where it will intercept the y = +/- aspectRatio lines.
    // Using aspectRatio causes this to work within a screen aspect ratio rectangle.
    float const xIntercept = dy == 0 ? dx * 100.0f : (dx / abs(dy)) * aspectRatio;
    float const yIntercept = dx == 0 ? dy * 100.0f : dy / abs(dx);
    float const xLenSQ = xIntercept * xIntercept + aspectRatio * aspectRatio;
    float const yLenSQ = yIntercept * yIntercept + 1.0f;
    if (xLenSQ < yLenSQ)
    {
        if (x != NULL)
        {
            *x = (xIntercept + 1.0f) * 0.5f;
        }
        if (y != NULL)
        {
            *y = yIntercept < 0.0f ? 0.0f : 1.0f;
        }
        return false;
    }
    else
    {
        if (x != NULL)
        {
            *x = xIntercept < 0.0f ? 0.0f : 1.0f;
        }
        if (y != NULL)
        {
            *y = (yIntercept / aspectRatio + 1.0f) * 0.5f;
        }
        return true;
    }
}
