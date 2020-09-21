// Voronoi

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"
#include "VoronoiDiagramGenerator.h"

using std::min;
using std::max;
using std::sort;
using std::vector;

class P33_Edge;
class P33_Point;

class PEffect33 : public PEffect
{
public:
    PEffect33(char const *tag, int32 n);
    virtual ~PEffect33();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_number;
    int32   m_mult;
    int32   m_add;
};

class P33_Edge
{
public:
    P33_Edge(float x1, float y1, float x2, float y2) :
    m_x1(x1),
    m_y1(y1),
    m_x2(x2),
    m_y2(y2),
    m_tx(x2 - x1),
    m_ty(y2 - y1),
    m_minx(min(x1, x2)),
    m_maxx(max(x1, x2))
    {
    }

    float   X1() const { return m_x1; }
    float   Y1() const { return m_y1; }
    float   X2() const { return m_x2; }
    float   Y2() const { return m_y2; }

    float   SolveForX(float y) const;
    bool    operator<(P33_Edge const &other) const { return m_y1 < other.m_y1; }

private:
    float   m_x1;
    float   m_y1;
    float   m_x2;
    float   m_y2;
    float   m_tx;
    float   m_ty;
    float   m_minx;
    float   m_maxx;
};

class P33_Point
{
public:
    P33_Point() :
        m_x1(-1000.0f), m_y1(-1000.0f), m_x2(-1000.0f), m_y2(-1000.0f)
    {
    }

    float   X() const { return m_x; }
    float   Y() const { return m_y; }
    float   X1() const { return m_x1; }
    float   X2() const { return m_x2; }
    float   Y1() const { return m_y1; }
    float   Y2() const { return m_y2; }
    void    SetXY(float x, float y) { m_x = x; m_y = y; }
    void    AddNeighbor(P33_Edge const &edge) { m_neighbors.push_back(edge); }
    void    AddLooseEnd(float x, float y, float w, float h);
    void    ProcessLooseEnds(float w, float h);
    vector<P33_Edge> const &GetNeighbors() const { return m_neighbors; }
    void    SortNeighbors() { sort(m_neighbors.begin(), m_neighbors.end()); }

private:
    float           m_x, m_y;
    float           m_sy;
    float           m_x1, m_x2;
    float           m_sx;
    float           m_y1, m_y2;
    vector<P33_Edge>   m_neighbors;
};

#ifdef NDEBUG
dec(PEffect33, peffect33_0, 16);
dec(PEffect33, peffect33_1, 64);
dec(PEffect33, peffect33_2, 256);
#endif

PEffect33::PEffect33(char const *tag, int32 n)
{
    SetControl(0, 0, 0.0f);
    m_number = n;
    m_mult = 256 / n;
    m_add = m_mult / 2;
    m_count = 2;
    m_tag = tag;
}

PEffect33::~PEffect33()
{
}

void    PEffect33::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float xvals[256];
    float yvals[256];
    P33_Point points[256];

    int32 fill = 0;
    for (int32 y = 0; y < h; y++)
    {
        memset(&data[y][0], fill, w);
        fill = 255 - fill;
    }

    int32 corners[4];
    for (int32 i = 0; i < 4; i++)
    {
        for (;;)
        {
            int32 const c = rnd32(15) + 1;
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

    xvals[0] = (float) rnd32(w);
    yvals[0] = (float) rnd32(h);
    points[0].SetXY(xvals[0], yvals[0]);

    int32 nextYStartIndex = 0;
    float llDist = xvals[0] * xvals[0] + yvals[0] * yvals[0];

    for (int32 i = 1; i < m_number; i++)
    {
        int32 j;
        for (j = 0; j < 4; j++)
        {
            if (i == corners[j])
            {
                break;
            }
        }

        int32 const xr = j == 4 ? w : w / 3;
        int32 const yr = j == 4 ? h : h / 3;
        int32 const xa = j & 1 ? w - xr : 0;
        int32 const ya = j & 2 ? h - yr : 0;

        for (;;)
        {
            float const x = (float) (rnd32(xr) + xa);
            float const y = (float) (rnd32(yr) + ya);

            for (j = 0; j < i - 1; j++)
            {
                float const x1 = xvals[j];
                float const y1 = yvals[j];
                float const dx = x < x1 ? x1 - x : x - x1;
                float const dy = y < y1 ? y1 - y : y - y1;
                if (dx < 10.0f && dy < 10.0f || dx < 1.0f || dy < 1.0f)
                {
                    break;
                }
            }
            if (j == i - 1)
            {
                xvals[i] = x;
                yvals[i] = y;
                points[i].SetXY(x, y);

                float const dist = x * x + y * y;
                if (dist < llDist || dist == llDist && y > yvals[nextYStartIndex])
                {
                    nextYStartIndex = i;
                    llDist = dist;
                }
                break;
            }
        }
    }

    Voronoi::generator.generateVoronoi(xvals, yvals, m_number, -1.0f, (float) w + 1.0f, -1.0f, (float) h + 1.0f);
    Voronoi::generator.resetIterator();

    float x1, y1;
    float x2, y2;
    int32 i1, i2;
    while (Voronoi::generator.getNext(x1, y1, x2, y2, i1, i2))
    {
        if (x1 == -1.0f && x2 == -1.0f || x1 == (float) w + 1.0f && x2 == (float) w + 1.0f ||
            y1 == -1.0f && y2 == -1.0f || y1 == (float) h + 1.0f && y2 == (float) h + 1.0f)
        {
            continue;
        }

        if (y2 < y1)
        {
            float const tx = x1;
            x1 = x2;
            x2 = tx;
            float const ty = y1;
            y1 = y2;
            y2 = ty;
        }
        P33_Edge const edge(x1, y1, x2, y2);
        points[i2].AddNeighbor(edge);
        points[i1].AddNeighbor(edge);

        if (y1 == -1.0f || x1 == -1.0f || x1 == (float) w + 1.0f)
        {
            points[i1].AddLooseEnd(x1, y1, (float) w, (float) h);
            points[i2].AddLooseEnd(x1, y1, (float) w, (float) h);
        }
        if (y2 == (float) h + 1.0f || x2 == -1.0f || x2 == (float) w + 1.0f)
        {
            points[i1].AddLooseEnd(x2, y2, (float) w, (float) h);
            points[i2].AddLooseEnd(x2, y2, (float) w, (float) h);
        }
    }

    for (int32 i = 0; i < m_number; i++)
    {
        points[i].ProcessLooseEnds((float) w, (float) h);
    }

    for (int32 i = 0; i < m_number; i++)
    {
        points[i].SortNeighbors();
        P33_Point const &point = points[i];
        auto neighbors = point.GetNeighbors().begin();
        P33_Edge const *leftEdge = &(*neighbors++);
        P33_Edge const *rightEdge = &(*neighbors++);
        int32 y = (int32) min(leftEdge->Y1(), rightEdge->Y1());
        if (y < 0)
        {
            y = 0;
        }
        while ((int32) leftEdge->Y2() == y && neighbors != point.GetNeighbors().end())
        {
            leftEdge = &(*neighbors++);
        }
        while ((int32) rightEdge->Y2() == y && neighbors != point.GetNeighbors().end())
        {
            rightEdge = &(*neighbors++);
        }
        // can we use slopes here?
        if (leftEdge->SolveForX((float) y + 2.0f) > rightEdge->SolveForX((float) y + 2.0f))
        {
            P33_Edge const *temp = leftEdge;
            leftEdge = rightEdge;
            rightEdge = temp;
        }
        for (;;)
        {
            if (y >= h)
            {
                break;
            }
            float const yf = (float) y;
            if ((yf > leftEdge->Y2() || yf > rightEdge->Y2()) && neighbors == point.GetNeighbors().end())
            {
                break;
            }
            float const mid = (leftEdge->SolveForX(yf) + rightEdge->SolveForX(yf)) * 0.5f;
            while (yf > leftEdge->Y2() || yf > rightEdge->Y2())
            {
                if (neighbors == point.GetNeighbors().end())
                {
                    break;
                }
                P33_Edge const *nextEdge = &(*neighbors++);
                if (nextEdge->X1() < mid)
                {
                    leftEdge = nextEdge;
                }
                else
                {
                    rightEdge = nextEdge;
                }
            }

            float const xl1 = leftEdge->SolveForX(yf);
            float const xr1 = rightEdge->SolveForX(yf);
            float const xl2 = Util::Clamp(xl1, 0.0f, (float) w);
            float const xr2 = Util::Clamp(xr1, 0.0f, (float) w);
            int32 const xl = (int32) xl2;
            int32 const xr = (int32) xr2;
            if (xr > xl)
            {
                memset(&data[y][xl], i * m_mult + m_add, xr - xl);
            }
            y++;
        }
    }
}

float   P33_Edge::SolveForX(float y) const
{
    if (m_ty == 0.0f)
    {
        return (m_x1 + m_x2) * 0.5f;
    }
    float const dy = y - m_y1;
    float const r = dy / m_ty;
    float const x = r * m_tx + m_x1;
    float const xc = Util::Clamp(x, m_minx, m_maxx);
    return xc;
}

void    P33_Point::AddLooseEnd(float x, float y, float w, float h)
{
    if (x == -1.0f || x == w + 1.0f)
    {
        m_sx = x;
        if (m_y1 == -1000.0f)
        {
            m_y1 = y;
        }
        else if (m_y2 == -1000.0f)
        {
            // This insertion ensures m_y1 <= m_y2
            if (m_y1 > y)
            {
                m_y2 = m_y1;
                m_y1 = y;
            }
            else
            {
                m_y2 = y;
            }
        }
    }
    else if (y == -1.0f || y == h + 1.0f)
    {
        m_sy = y;
        if (m_x1 == -1000.0f)
        {
            m_x1 = x;
        }
        else if (m_x2 == -1000.0f)
        {
            // This insertion ensures m_x1 <= m_x2
            if (m_x1 > x)
            {
                m_x2 = m_x1;
                m_x1 = x;
            }
            else
            {
                m_x2 = x;
            }
        }
    }
}

void    P33_Point::ProcessLooseEnds(float w, float h)
{
    int32 count = (m_x1 != -1000.0f ? 1 : 0) + (m_x2 != -1000.0f ? 1 : 0) +
                (m_y1 != -1000.0f ? 1 : 0) + (m_y2 != -1000.0f ? 1 : 0);
    if (count == 1 || count == 3)
    {
        return;
    }
    if (count == 0)
    {
        return;
    }
    if (m_x2 == -1000.0f && m_y2 == -1000.0f)
    {
        // Corner - we'll add the missing vertical edge, but not the horizontal one.
        // m_sx, m_sy is the corner itself, and the other end of the edge will be
        // m_sx, m_y1
        float const y1 = min(m_sy, m_y1);
        float const y2 = max(m_sy, m_y1);
        P33_Edge edge(m_sx, y1, m_sx, y2);
        AddNeighbor(edge);
        return;
    }
    if (m_y1 != -1000.0f)
    {
        // Side edge not in a corner
        // m_sx is the x coord, m_y1 and m_y2 are the y extremities
        P33_Edge edge(m_sx, m_y1, m_sx, m_y2);
        AddNeighbor(edge);
    }
}
