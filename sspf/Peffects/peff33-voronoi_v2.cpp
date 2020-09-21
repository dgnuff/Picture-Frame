#include "sspf.h"

#include "pch.h"

#include "peffect.h"
//#include <clib.h>
#include <memory.h>
#include <malloc.h>
#include <algorithm>
#include <vector>
#include "VoronoiDiagramGenerator.h"

using std::min;
using std::max;
using std::sort;
using std::vector;
typedef unsigned char uchar;

class Edge_;
class Point_;

class PEffect33 : public PEffect
{
public:
    PEffect33(char const *tag, int n);
    virtual ~PEffect33();
    virtual void            GenerateOne(uchar **data, int w, int h) const;

private:
    int     m_number;
    int     m_mult;
    int     m_add;

    static VoronoiDiagramGenerator s_gen;

    //float   Clamp(float v, float low, float high) const;
};

VoronoiDiagramGenerator PEffect33::s_gen;

class Edge_
{
public:
    //Edge_() {}
    Edge_(float x1, float y1, float x2, float y2);

    float   X1() const                          { return m_x1; }
    float   Y1() const                          { return m_y1; }
    float   X2() const                          { return m_x2; }
    float   Y2() const                          { return m_y2; }
    float   SolveForX(float y, bool useMax) const;
    bool operator<(Edge_ const &other) const    { return m_y1 < other.m_y1; }

private:
    float   m_x1, m_y1;
    float   m_x2, m_y2;
    //float   m_s, m_c;
    //float   Slope(float x1, float y1, float x2, float y2);
};

class Point_
{
public:
    Point_() : m_x1(-1000.0f), m_y1(-1000.0f), m_x2(-1000.0f), m_y2(-1000.0f) {}

    float   X() const                           { return m_x; }
    float   Y() const                           { return m_y; }
    float   X1() const                          { return m_x1; }
    float   Y1() const                          { return m_y1; }
    float   X2() const                          { return m_x2; }
    float   Y2() const                          { return m_y2; }
    void    SetXY(float x, float y)             { m_x = x; m_y = y; }
    void    AddNeighbor(Edge_ const &edge)      { m_neighbors.push_back(edge); }
    void    AddLooseEnd(float x, float y);
    vector<Edge_> const &GetNeighbors() const   { return m_neighbors; }
    void    SortNeighbors()                     { sort(m_neighbors.begin(), m_neighbors.end()); }

private:
    float           m_x, m_y;
    float           m_x1, m_y1;
    float           m_x2, m_y2;
    vector<Edge_>   m_neighbors;
};

#ifdef _MSC_VER
dec(PEffect33, peffect33_0, 16);
dec(PEffect33, peffect33_1, 64);
dec(PEffect33, peffect33_2, 256);
//class PEffect33 peffect33_2("peffect33_2(256)", 256);
#endif

PEffect33::PEffect33(char const *tag, int n)
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

static float    Clamp(float v, float low, float high)
{
    return v < low ? low : v > high ? high : v;
}

void    PEffect33::GenerateOne(uchar **data, int w, int h) const
{
    float xvals[256];
    float yvals[256];
    Point_ points[256];

    //unsigned int rrrr = rand32();
    //rrrr = 684358731;
    //srand32(rrrr);

    //static int fill = 0;
    int fill = 0;
    for (int y = 0; y < h; y++)
    {
        memset(&data[y][0], fill, w);
        fill = 255 - fill;
    }
    //fill++;

    /*
    FILE *fp = fopen("errors.txt", "a");
    if (fp)
    {
        fprintf(fp, "rrrr = %d;\n", rrrr);
        fclose(fp);
    }
    */

    xvals[0] = (float) rnd32(w);
    yvals[0] = (float) rnd32(h);
    points[0].SetXY(xvals[0], yvals[0]);

    int nextYStartIndex = 0;
    float llDist = xvals[0] * xvals[0] + yvals[0] * yvals[0];

    for (int i = 1; i < m_number; i++)
    {
        for (;;)
        {
            float const x = (float) rnd32(w);
            float const y = (float) rnd32(h);

            int j;
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

    s_gen.generateVoronoi(xvals, yvals, m_number, -1.0f, (float) w + 1.0f, -1.0f, (float) h + 1.0f);
    s_gen.resetIterator();

    float x1;
    float y1;
    float x2;
    float y2;
    int i1;
    int i2;
    while (s_gen.getNext(x1, y1, x2, y2, i1, i2))
    {
        if (x1 == -1.0f && x2 == -1.0f || x1 == (float) w + 1.0f && x2 == (float) w + 1.0f ||
            y1 == -1.0f && y2 == -1.0f || y1 == (float) h + 1.0f && y2 == (float) h + 1.0f)
        {
            continue;
        }
        if (x1 == -1.0f && y2 == -1.0f || x1 == (float) w + 1.0f && y2 == (float) h + 1.0f ||
            y1 == -1.0f && x2 == -1.0f || y1 == (float) h + 1.0f && x2 == (float) w + 1.0f)
        {
            continue;
        }
        if (x1 == -1.0f && y2 == (float) h + 1.0f || x2 == -1.0f && y1 == (float) h + 1.0f ||
            y1 == -1.0f && x2 == (float) w + 1.0f || y2 == -1.0f && x1 == (float) w + 1.0f)
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
        Edge_ const edge(x1, y1, x2, y2);
        points[i2].AddNeighbor(edge);
        points[i1].AddNeighbor(edge);

        if (y1 == -1.0f || x1 == -1.0f || x1 == (float) w + 1.0f)
        {
            points[i1].AddLooseEnd(x1, y1);
            points[i2].AddLooseEnd(x1, y1);
        }
        if (y2 == (float) h + 1.0f || x2 == -1.0f || x2 == (float) w + 1.0f)
        {
            points[i1].AddLooseEnd(x2, y2);
            points[i2].AddLooseEnd(x2, y2);
        }
    }

    for (int i = 1; i < m_number; i++)
    {
        if (points[i].X1() != -1000.0f && points[i].X2() == -1000.0f)
        {
            int j = 0;
        }
        if (points[i].X1() == -1.0f && points[i].X2() != -1.0f)
        {
            // Top left or bottom left corner.  points[i].Y2() says whether this is top or bottom
            float const y1 = points[i].Y2() == -1.0f ? -1.0f : points[i].Y1();
            float const y2 = points[i].Y2() == -1.0f ? points[i].Y1() : (float) h + 1.0f;
            Edge_ edge(-1.0f, y1, -1.0f, y2);
            points[i].AddNeighbor(edge);
        }
        else if (points[i].X1() != (float) w + 1.0f && points[i].X2() == (float) w + 1.0f)
        {
            // Top right or bottom right corner.  points[i].Y1() says whether this is top or bottom
            float const y1 = points[i].Y1() == -1.0f ? -1.0f : points[i].Y2();
            float const y2 = points[i].Y1() == -1.0f ? points[i].Y2() : (float) h + 1.0f;
            Edge_ edge((float) w + 1.0f, y1, (float) w + 1.0f, y2);
            points[i].AddNeighbor(edge);
        }
        else if (points[i].X1() == -1.0f && points[i].X2() == -1.0f)
        {
            // Left edge not in a corner
            float const y1 = points[i].Y1() < points[i].Y2() ? points[i].Y1() : points[i].Y2();
            float const y2 = points[i].Y1() < points[i].Y2() ? points[i].Y2() : points[i].Y1();
            Edge_ edge(-1.0f, y1, -1.0f, y2);
            points[i].AddNeighbor(edge);
        }
        else if (points[i].X1() == (float) w + 1.0f && points[i].X2() == (float) w + 1.0f)
        {
            // Right edge not in a corner
            float const y1 = points[i].Y1() < points[i].Y2() ? points[i].Y1() : points[i].Y2();
            float const y2 = points[i].Y1() < points[i].Y2() ? points[i].Y2() : points[i].Y1();
            Edge_ edge((float) w + 1.0f, y1, (float) w + 1.0f, y2);
            points[i].AddNeighbor(edge);
        }
    }

    for (int i = 0; i < m_number; i++)
    {
        Point_ const &point = points[i];
        if (point.GetNeighbors().size() < 3)
        {
            continue;
        }
        // Can't use the reference since SortNeighbors is non-const.
        points[i].SortNeighbors();
        auto neighbors = point.GetNeighbors().begin();
        Edge_ const *leftEdge = &(*neighbors++);
        Edge_ const *rightEdge = &(*neighbors++);
        int y = (int) min(leftEdge->Y1(), rightEdge->Y1());
        if (y < 0)
        {
            y = 0;
        }
        /*
        float const epsilon = 0.0002f;
        if (leftEdge->Y1() < rightEdge->Y1() - epsilon || leftEdge->Y1() > rightEdge->Y1() + epsilon)
        {
            int j = 0;
        }
        */
        while ((int) leftEdge->Y2() == y && neighbors != point.GetNeighbors().end())
        {
            leftEdge = &(*neighbors++);
        }
        while ((int) rightEdge->Y2() == y && neighbors != point.GetNeighbors().end())
        {
            rightEdge = &(*neighbors++);
        }
        // can we use slopes here?
        if (leftEdge->SolveForX((float) y + 1.0f, false) > rightEdge->SolveForX((float) y + 1.0f, true))
        {
            Edge_ const *temp = leftEdge;
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
            float const mid = (leftEdge->SolveForX(yf, false) + rightEdge->SolveForX(yf, true)) * 0.5f;
            while (yf > leftEdge->Y2() || yf > rightEdge->Y2())
            {
                if (neighbors == point.GetNeighbors().end())
                {
                    break;
                }
                Edge_ const *nextEdge = &(*neighbors++);
                if (nextEdge->X1() < mid)
                {
                    leftEdge = nextEdge;
                }
                else
                {
                    rightEdge = nextEdge;
                }
            }

            float const xl1 = leftEdge->SolveForX(yf, false);
            float const xr1 = rightEdge->SolveForX(yf, true);
            float const xl2 = Clamp(xl1, 0.0f, (float) w);
            float const xr2 = Clamp(xr1, 0.0f, (float) w);
            int const xl = (int) xl2;
            int const xr = (int) xr2;
            if (xr > xl)
            {
                memset(&data[y][xl], i * m_mult + m_add, xr - xl);
            }
            y++;
        }
    }
}

Edge_::Edge_(float x1, float y1, float x2, float y2) :
m_x1(x1),
m_y1(y1),
m_x2(x2),
m_y2(y2)
//m_s(Slope(x1, y1, x2, y2)),
//m_c(x1 - m_s * y1)
{
}

float   Edge_::SolveForX(float y, bool useMax) const
{
    float const ty = m_y2 - m_y1;
    if (ty == 0.0f)
    {
        return useMax ? max(m_x1, m_x2) : min(m_x1, m_x2);
    }
    float const tx = m_x2 - m_x1;
    float const dy = y - m_y1;
    float const r = dy / ty;
    float const minx = min(m_x1, m_x2);
    float const maxx = max(m_x1, m_x2);
    float const x = r * tx + m_x1;
    float const xc = Clamp(x, minx, maxx);
    return xc;
}

/*
float   Edge_::Slope(float x1, float y1, float x2, float y2)
{
    float const epsilon = 1.0e-6f;

    if (y1 - y2 < -epsilon || y1 - y2 > epsilon)
    {
        return (x1 - x2) / (y1 - y2);
    }
    float const sx = x1 < x2 ? -1.0e6f : 1.0e6f;
    float const sy = y1 < y2 ? -1.0e6f : 1.0e6f;
    return sx * sy;
}
*/

void    Point_::AddLooseEnd(float x, float y)
{
    if (m_x1 == -1000.0f)
    {
        m_x1 = x;
        m_y1 = y;
    }
    else if (m_x2 == -1000.0f)
    {
        // This insertion ensures m_x1 <= m_x2
        if (m_x1 > x)
        {
            m_x2 = m_x1;
            m_y2 = m_y1;
            m_x1 = x;
            m_y1 = y;
        }
        else
        {
            m_x2 = x;
            m_y2 = y;
        }
    }
    else
    {
        int i = 0;
    }
}

/*
void    GenerateOne(uchar **data, int w, int h)
{
    peffect33_2.GenerateOne(data, w, h);
}
*/
