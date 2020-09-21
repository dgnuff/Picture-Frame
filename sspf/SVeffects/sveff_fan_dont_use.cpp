// Center Fan

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "sveffect.h"
#include <algorithm>

using std::max;

class SVEffect39 : public SVEffect
{
public:
    SVEffect39(char const *tag, int o, int c, int s);
    virtual         ~SVEffect39();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans **transforms);
    virtual void    GetTransData(int pass, int frameNum, D3DXVECTOR4 const **transforms, uint *numTrans, uint *startIndex, uint *numTri, FRECT const **scissor) const;

private:
    uint            m_count;
    int             m_sections;
    void            GenerateVertices();

    Vertex          m_verts[32 * 3];
};

#ifdef _MSC_VER
dec(SVEffect39, sveffect39_0, 0, 32, 1);
//dec(SVEffect39, sveffect39_1, 1, 32, 1);
//dec(SVEffect39, sveffect39_2, 0, 8, 1);
//dec(SVEffect39, sveffect39_3, 1, 8, 1);

//dec(SVEffect39, sveffect39_2, 0, 1);
//dec(SVEffect39, sveffect39_3, 1, 1);
//dec(SVEffect39, sveffect39_4, 0, 2);
//dec(SVEffect39, sveffect39_5, 1, 2);
#endif

SVEffect39::SVEffect39(char const *tag, int o, int c, int s)
{
    m_order = o;
    m_passes = 4; //  max(2, m_sections);
    m_sections = s;
    m_count = c;
    m_tag = tag;
}

SVEffect39::~SVEffect39()
{
}

void    SVEffect39::Init()
{
    m_vertices = m_verts;

    GenerateVertices();

    m_numVerts = m_count * 3;

    static ushort indices[32 * 3];
    m_indices = indices;

    for (int i = 0; i < 32 * 3; i++)
    {
        m_indices[i] = i;
    }

    m_numIndex = m_count * 3;
}

void    SVEffect39::GenerateVertices()
{
    static const struct
    {
        int x;
        int y;
    } centerPointsIndices[33] =
    {
        { 4, 0 },
        { 5, 0 },
        { 6, 0 },
        { 7, 0 },
        { 8, 0 },
        { 8, 1 },
        { 8, 2 },
        { 8, 3 },
        { 8, 4 },
        { 8, 5 },
        { 8, 6 },
        { 8, 7 },
        { 8, 8 },
        { 7, 8 },
        { 6, 8 },
        { 5, 8 },
        { 4, 8 },
        { 3, 8 },
        { 2, 8 },
        { 1, 8 },
        { 0, 8 },
        { 0, 7 },
        { 0, 6 },
        { 0, 5 },
        { 0, 4 },
        { 0, 3 },
        { 0, 2 },
        { 0, 1 },
        { 0, 0 },
        { 1, 0 },
        { 2, 0 },
        { 3, 0 },
        { 4, 0 },
    };

    struct
    {
        float x;
        float y;
    } centerPoints[33];

    float loci[9];

    float const anglePerStep = M_2PI_f / 32.0f;
    for (int i = 0; i < 5; i++)
    {
        float const angle = anglePerStep * (float) i;
        float const offset = tanf(angle) * 0.5f;
        loci[4 - i] = 0.5f - offset;
        loci[4 + i] = 0.5f + offset;
    }

    for (int i = 0; i < 33; i++)
    {
        centerPoints[i].x = loci[centerPointsIndices[i].x];
        centerPoints[i].y = loci[centerPointsIndices[i].y];
    }

    uint const piecesPerSection = m_count / m_sections;
    uint const skip = 32 / m_count;
    float db = 0.0f;
    for (uint i = 0, a = 0; i < 32; i += skip)
    {
        uint const next = i + skip;

        GenerateVertex(i * 3 + 0, 0.5f, 0.5f, 0, i, db);
        GenerateVertex(i * 3 + 1, centerPoints[i].x, centerPoints[i].y, 0, i, db);
        GenerateVertex(i * 3 + 2, centerPoints[next].x, centerPoints[next].y, 0, i, db);

        if (++a >= piecesPerSection)
        {
            a = 0;
            db = 0.0f;
        }
        else
        {
            db -= 1.0f / 256.0f;
        }
    }
}

void    SVEffect39::GenerateTrans(AATrans **transforms)
{
    m_transforms = transforms;
    memset(&transforms[0][0], 0, 32 * sizeof(AATrans));

    uint const piecesPerSection = m_count / m_sections;
    float const anglePerSection = M_2PI_f / ((float) m_count * 256.0f);
    float angle = 0.0f;
    for (uint i = 0; i < m_count; i++)
    {
        if (i % piecesPerSection == 0)
        {
            angle = 0.0f;
        }
        angle += anglePerSection;
        m_transforms[0][i].rotation.z = 1.0f;
        m_transforms[0][i].rotation.w = angle;
    }
}

void    SVEffect39::GetTransData(int pass, int frameNum, D3DXVECTOR4 const **transforms, uint *numTrans, uint *startIndex, uint *numTri, FRECT const **scissor) const
{
    if (transforms != NULL)
    {
        *transforms = (D3DXVECTOR4 *) m_transforms[0];
    }
    if (numTrans != NULL)
    {
        *numTrans = m_count * 3;
    }
    if (startIndex != NULL)
    {
        *startIndex = pass * 8 * 3; // 0;
    }
    if (numTri != NULL)
    {
        *numTri = 1; //  m_count;
    }
}
