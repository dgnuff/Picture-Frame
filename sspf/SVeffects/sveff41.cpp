// 256 region Voronoi

#include "pch.h"

#include "sspf.h"
#include "veffect.h"
#include "VoronoiDiagramGenerator.h"

using std::swap;
using std::sort;
using std::max;
using std::vector;

class S41_Edge;
class S41_Point;

class SVEffect41 : public VEffect
{
public:
    SVEffect41(char const *tag, int32 o, int32 d);
    virtual ~SVEffect41();
    virtual void    Init();
    virtual void    GenerateTrans(AATrans *transforms, uint32 w, uint32 h);
    virtual void    GenerateFrames(float frameNum, float *frames) const;
    virtual void    GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;

private:
    int32           m_direction;
    int32           m_pieces;
    int32           m_edge;
    uint32          m_triCount;

    static Vertex   s_localVertices[MAX_VERTEX];
    static uint16   s_localIndices[MAX_INDEX];
};

Vertex  SVEffect41::s_localVertices[MAX_VERTEX];
uint16  SVEffect41::s_localIndices[MAX_INDEX];

class S41_Edge
{
public:
    S41_Edge(float x1, float y1, float x2, float y2) :
    m_x1(x1),
    m_y1(y1),
    m_x2(x2),
    m_y2(y2)
    {
    }

    float   X1() const                          { return m_x1; }
    float   Y1() const                          { return m_y1; }
    float   X2() const                          { return m_x2; }
    float   Y2() const                          { return m_y2; }

private:
    float           m_x1, m_y1;
    float           m_x2, m_y2;
};

class S41_Point
{
public:
    S41_Point() :
    m_x(0.0f),
    m_y(0.0f)
    {
    }

    float   X() const                           { return m_x; }
    float   Y() const                           { return m_y; }
    vector<S41_Edge> const &GetEdges() const    { return m_edges; }
    void    SetXY(float x, float y)             { m_x = x; m_y = y; }
    void    AddEdge(float x1, float y1, float x2, float y2);
    bool    IndexToXYEdge(int i, int &e, float &x, float &y);
    void    ProcessLooseEnds();

private:
    float               m_x;
    float               m_y;
    vector<int32>       m_ends;
    vector<S41_Edge>    m_edges;
};

#ifdef NDEBUG
dec(SVEffect41, sveffect41_0,  0, 0);
dec(SVEffect41, sveffect41_1,  1, 0);
dec(SVEffect41, sveffect41_2,  0, 1);
dec(SVEffect41, sveffect41_3,  1, 1);
dec(SVEffect41, sveffect41_4,  0, 2);
dec(SVEffect41, sveffect41_5,  1, 2);
dec(SVEffect41, sveffect41_6,  0, 3);
dec(SVEffect41, sveffect41_7,  1, 3);
dec(SVEffect41, sveffect41_8,  0, 4);
dec(SVEffect41, sveffect41_9,  1, 4);
dec(SVEffect41, sveffect41_10, 0, 5);
dec(SVEffect41, sveffect41_11, 1, 5);
#endif

SVEffect41::SVEffect41(char const *tag, int32 o, int32 d)
{
    m_order = o;
    m_direction = d;
    m_tag = tag;
}

SVEffect41::~SVEffect41()
{
}

void    SVEffect41::Init()
{
    m_vertices = s_localVertices;
    m_numVerts = MAX_VERTEX;

    m_indices = s_localIndices;
    m_numIndex = MAX_INDEX;
}

static bool ComparePoints(S41_Point const &a, S41_Point const &b, int32 edge)
{
    switch (edge)
    {
    case 0:
    default:
        return a.X() < b.X();

    case 1:
        return b.X() < a.X();

    case 2:
        return a.Y() < b.Y();

    case 3:
        return b.Y() < a.Y();
    }
}

void    SVEffect41::GenerateTrans(AATrans *transforms, uint32 w, uint32 h)
{
    static int pieces[3] = { 256, 64, 16 };
    m_pieces = pieces[rnd32(3)];

    vector<S41_Point> points;
    points.resize(m_pieces);

    int32 corners[4];
    for (int32 i = 0; i < 4; i++)
    {
        for (;;)
        {
            int32 const c = rnd32(m_pieces - 2) + 1;
            int32 j;
            for (j = 0; j < i; j++)
            {
                if (c == corners[j])
                {
                    break;
                }
            }
            if (j == i)
            {
                corners[i] = c;
                break;
            }
        }
    }

    points[0].SetXY(rndf32(), rndf32());

    for (int32 i = 1; i < m_pieces; i++)
    {
        int32 j;
        for (j = 0; j < 4; j++)
        {
            if (i == corners[j])
            {
                break;
            }
        }

        float const xr = j == 4 ? 1.0f : 0.25f;
        float const yr = j == 4 ? 1.0f : 0.25f;
        float const xa = j & 1 ? 0.75f : 0.0f;
        float const ya = j & 2 ? 0.75f : 0.0f;

        for (;;)
        {
            float const x = (float) rndf32() * xr + xa;
            float const y = (float) rndf32() * yr + ya;

            for (j = 0; j < i - 1; j++)
            {
                float const x1 = points[j].X();
                float const y1 = points[j].Y();
                float const dx = x < x1 ? x1 - x : x - x1;
                float const dy = y < y1 ? y1 - y : y - y1;
                if (dx < 0.02f && dy < 0.02f || dx < 0.002f || dy < 0.002f)
                {
                    break;
                }
            }
            if (j == i - 1)
            {
                points[i].SetXY(x, y);
                break;
            }
        }
    }

    if (m_direction == 4)
    {
        m_edge = rand32() & 3;
        auto compare = [=](S41_Point const &a, S41_Point const &b)
        {
            return ComparePoints(a, b, m_edge);
        };
        sort(points.begin(), points.end(), compare);
    }

    float xvals[256];
    float yvals[256];

    for (int32 i = 0; i < m_pieces; i++)
    {
        xvals[i] = points[i].X();
        yvals[i] = points[i].Y();
    }

    Voronoi::generator.generateVoronoi(xvals, yvals, m_pieces, 0.0f, 1.0f, 0.0f, 1.0f);
    Voronoi::generator.resetIterator();

    float x1, y1;
    float x2, y2;
    int32 i1, i2;
    while (Voronoi::generator.getNext(x1, y1, x2, y2, i1, i2))
    {
        if (x1 != x2 || y1 != y2)
        {
            points[i1].AddEdge(x1, y1, x2, y2);
            points[i2].AddEdge(x1, y1, x2, y2);
        }
    }

    for (int32 i = 0; i < m_pieces; i++)
    {
        points[i].ProcessLooseEnds();
    }

    m_transforms = transforms;

    int32 v = 0;
    uint16 vertexIndex = 0;
    uint16 *index = s_localIndices;
    m_triCount = 0;

    float const ctrl = m_direction == 5 ? -1.0f : 0.0f;

    for (int32 i = 0; i < m_pieces; i++)
    {
        S41_Point &point(points[i]);

        float const x = point.X();
        float const y = point.Y();

        float postX;
        float postY;
        float spread;
        float scale;
        int32 frame;

        static const float postXs[4] = { -1.0f, 1.0f,  0.0f, 0.0f, };
        static const float postYs[4] = {  0.0f, 0.0f, -1.0f, 1.0f, };

        switch (m_direction)
        {
        case 0:
        case 3:
        default:
            postX = 0.0f;
            postY = 0.0f;
            spread = 1.0f;
            scale = 0.45f;
            frame = m_direction == 3 ? i * 128 / m_pieces : 0;
            break;

        case 1:
            postX = -0.75f + x * 1.6f;
            postY = -0.75f + y * 1.6f;
            spread = 0.25f;
            scale = 0.45f;
            frame = 0;
            break;

        case 2:
            postX = 0.75f - x * 1.6f;
            postY = 0.75f - y * 1.6f;
            spread = 0.5f;
            scale = 0.35f;
            frame = 0;
            break;

        case 4:
            postX = postXs[m_edge];
            postY = postYs[m_edge];
            spread = 0.25f;
            scale = 0.35f;
            frame = i * 128 / m_pieces;
            break;

        case 5:
            postX = 0.0f;
            postY = 0.0f;
            spread = 0.0f;
            scale = 0.0f;
            frame = 0;
            break;
        }

        GenerateRandomTrans(&transforms[i], w, h, x * 2.0f - 0.875f, y * 2.0f - 0.875f, postX, postY, spread, scale);

        if (m_direction == 5)
        {
            transforms[i].preTrans.x = x * -2.0f + 0.9375f;
            transforms[i].preTrans.y = y * -2.0f + 0.9375f;
            transforms[i].preTrans.w = 1.0f;
        }

        vector<S41_Edge> const &edges = point.GetEdges();
        // There's a lot of room for optimization here.
        // Basically figure a way to key vertices from the x, y pair, and then only insert them
        // into the vertices array once.
        int const centerIndex = vertexIndex++;
        GenerateVertex(s_localVertices, centerIndex, x, y, frame, i, 0.0f, ctrl);

        for (auto edge : edges)
        {
            *index++ = centerIndex;
            GenerateVertex(s_localVertices, vertexIndex, edge.X1(), edge.Y1(), frame, i, 0.0f, ctrl);
            *index++ = vertexIndex++;
            GenerateVertex(s_localVertices, vertexIndex, edge.X2(), edge.Y2(), frame, i, 0.0f, ctrl);
            *index++ = vertexIndex++;
            m_triCount++;
        }
    }
}

void    SVEffect41::GenerateFrames(float frameNum, float *frames) const
{
    if (frames != NULL)
    {
        if (false && m_direction == 3)
        {
            frameNum *= 2.0f;
            for (int32 i = 0; i < 8; i++)
            {
                frames[i] = frameNum;
                frameNum = max(frameNum - 32.0f, 0.0f);
            }
        }
        else if (m_direction == 4 || m_direction == 3)
        {
            frameNum *= 2.0f;
            for (int32 i = 0; i < 128; i++)
            {
                frames[i] = frameNum;
                frameNum = max(frameNum - 2.0f, 0.0f);
            }
        }
        else
        {
            frames[0] = frameNum;
        }
    }
}

void    SVEffect41::GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const
{
    if (numTri != NULL)
    {
        *numTri = m_triCount;
    }
}

void    S41_Point::AddEdge(float x1, float y1, float x2, float y2)
{
    if (x2 == 0.0f || x2 == 1.0f || y2 == 0.0f || y2 == 1.0f)
    {
        swap(x1, x2);
        swap(y1, y2);
    }
    S41_Edge const edge(x1, y1, x2, y2);
    int32 const index = (int32) m_edges.size();
    m_edges.push_back(edge);
    if (x1 == 0.0f || x1 == 1.0f || y1 == 0.0f || y1 == 1.0f)
    {
        m_ends.push_back(index);
    }
}

bool    S41_Point::IndexToXYEdge(int i, int &e, float &x, float &y)
{
    int const w = m_ends[i];
    x = m_edges[w].X1();
    y = m_edges[w].Y1();
    if (x == 0.0f)
    {
        e = 0;
    }
    else if (x == 1.0f)
    {
        e = 1;
    }
    else if (y == 0.0f)
    {
        e = 2;
    }
    else if (y == 1.0f)
    {
        e = 3;
    }
    else
    {
        return false;
    }
    return true;
}

void    S41_Point::ProcessLooseEnds()
{
    uint32 const s = (uint32) m_ends.size();
    if (s < 2)
    {
        return;
    }

    // This will do spurious work in the case where m_ends.size() > 2.  However I'm just not that bothered since:
    // 1. Empirical testing shows it just doesn't happen that often, and when it does, it will only mean an extra poly
    // or two to be rendered.  Since this program simply isn't render rate bound, that's a complete non-issue.
    // 2. The effort required to correctly find and remove the unneeded members of m_ends is way bigger than I'm
    // willing to spend on what is a complete non-issue anyway.
    for (uint32 i1 = 0; i1 < s; i1++)
    {
        for (uint32 i2 = i1 + 1; i2 < s; i2++)
        {
            float const x1 = m_edges[m_ends[i1]].X1();
            float const y1 = m_edges[m_ends[i1]].Y1();
            float const x2 = m_edges[m_ends[i2]].X1();
            float const y2 = m_edges[m_ends[i2]].Y1();
            if (x1 == x2 && (x1 == 0.0f || x1 == 1.0f) || y1 == y2 && (y1 == 0.0f || y1 == 1.0f))
            {
                AddEdge(x1, y1, x2, y2);
            }
            else
            {
                float const x3 = x1 == 0.0f || x1 == 1.0f ? x1 : x2;
                float const y3 = y1 == 0.0f || y1 == 1.0f ? y1 : y2;
                AddEdge(x1, y1, x3, y3);
                AddEdge(x3, y3, x2, y2);
            }
        }
    }
}
