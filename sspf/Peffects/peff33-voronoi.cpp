// Voronoi tesselation

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

using std::min;
using std::max;
using std::sort;
using std::map;
using std::set;
using std::vector;

static float lerp(float begin, float finish, float factor)
{
    return begin + (finish - begin) * factor;
}

typedef unsigned __int64 uint64;

class StaticPerf
{
public:
    StaticPerf(char const *name) :
    m_name(name),
    m_total(0),
    m_only(0)
    {
        m_next = s_perfList;
        s_perfList = this;
    }
    char const *Name() const { return m_name; }
    uint64 Total() const { return m_total; }
    uint64 Only() const { return m_only; }

    void AddData(uint64 total, uint64 only)
    {
        m_total += total;
        m_only += only;
    }

    static void Reset()
    {
        for (StaticPerf *sp = s_perfList; sp != NULL; sp = sp->m_next)
        {
            sp->m_total = 0;
            sp->m_only = 0;
        }
    }

    static void Dump()
    {
        for (StaticPerf *sp = s_perfList; sp != NULL; sp = sp->m_next)
        {
            Util::RecordError("%s: total: %u, only: %u", sp->m_name, (uint) sp->m_total, (uint) sp->m_only);
        }
        Util::RecordError("");
    }

private:
    char const *m_name;
    uint64 m_total;
    uint64 m_only;
    StaticPerf *m_next;

    static StaticPerf *s_perfList;
};

StaticPerf *StaticPerf::s_perfList = NULL;

class Perf;

Perf *current = NULL;

class Perf
{
public:
    Perf(StaticPerf &sp) :
    m_sp(sp)
    {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        m_start = t.QuadPart;
        m_elsewhere = 0;
        m_next = current;
        current = this;
    }
    ~Perf()
    {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        m_start = t.QuadPart - m_start;
        m_elsewhere += m_start;
        m_sp.AddData(m_start, m_elsewhere);
        if (m_next != NULL)
        {
            m_next->m_elsewhere -= m_start;
            current = m_next;
        }
    }

private:
    StaticPerf &m_sp;
    uint64 m_start;
    uint64 m_elsewhere;
    Perf *m_next;
};

#define         NOINDEX         (-11111)

class Node
{
public:
    Node() {}
    Node(float x, float y) : m_x(x), m_y(y) {}
    ~Node() {}

    float   X() const { return m_x; }
    float   Y() const { return m_y; }
    set<Node const *> const     &Neighbors() const { return m_neighbors; }

    void    AddNeighbor(Node const *p)
    {
        if (p != NULL)
        {
            m_neighbors.insert(p);
        }
    }
    void    SetXY(float x, float y)
    {
        m_x = x;
        m_y = y;
    }
    float   DistanceSq(Node const &other) const
    {
        float const dx = m_x - other.m_x;
        float const dy = m_y - other.m_y;
        return dx * dx + dy * dy;
    }

private:
    float               m_x;
    float               m_y;
    set<Node const *>  m_neighbors;
};


class Point
{
public:
    Point() {}
    Point(float x, float y, int i = NOINDEX) : m_x(x), m_y(y), m_index(i)  {}
    ~Point() {}

    float   X() const       { return m_x; }
    float   Y() const       { return m_y; }
    int     Index() const   { return m_index; }
    Point   operator-(Point const &p) const         { return Point(m_x - p.m_x, m_y - p.m_y); }
    float   operator*(Point const &p) const         { return m_x * p.m_x + m_y * p.m_y; }
    bool    operator==(Point const &p) const        { return m_index == p.m_index; }
    bool    operator<(Point const &p) const         { return m_x == p.m_x ? m_y < p.m_y : m_x < p.m_x; }

    void    SetXY(float x, float y)
    {
        m_x = x;
        m_y = y;
    }
    void    SetIndex(int i)
    {
        m_index = i;
    }

private:
    float               m_x;
    float               m_y;
    int                 m_index;
};

class DTriangle
{
public:
    DTriangle(): p1(0.0f, 0.0f), p2(0.0f, 1.0f), p3(1.0f, 0.0f)
    {
        initCC();
    }
    DTriangle(Point const &pp1, Point const &pp2, Point const &pp3) : p1(pp1), p2(pp2), p3(pp3)
    {
        initCC();
    }

    bool inside(Point const &pp) const;
    bool isInsideCC(Point const &p) const;

    Point p1;
    Point p2;
    Point p3;

    Point CCCenter;
    float CCR;
    float CCR2;
    bool nonDegenerate;

    void initCC();
};

class Edge
{
public:
    Edge() {}
    Edge(Point const &node1p, Point const &node2p)
    : node1(node1p), node2(node2p)
    {
    }

    Edge inversed() const
    {
        return Edge(node2, node1);
    }

    bool operator==(const Edge& e) const
    {
        return node1.Index() == e.node1.Index() && node2.Index() == e.node2.Index();
    }

    bool within(Point p) const
    {
        float const n1x = node1.X();
        float const n2x = node2.X();
        if (n1x < n2x)
        {
            return n1x <= p.X() && p.X() <= n2x;
        }
        else
        {
            return n2x <= p.X() && p.X() <= n1x;
        }
    }

/*
    double relCross(Point p) const
    {
    return LineSeg(node1, node2, LineSeg::line).relCross(p);
    }

    double Distance(Point p) const
    {
        return LineSeg(node1, node2, LineSeg::segment).Distance(p);
    }
*/

    Point node1;
    Point node2;
};

class PEffect33 : public PEffect
{
public:
    PEffect33(char const *tag, int n);
    virtual ~PEffect33();
    virtual void            GenerateOne(uchar **data, int w, int h) const;

private:
    int     m_number;

    static bool     Compare(Point const &a, Point const &b);
    static uint     FindClosest(Node const *array, uint arraySize, Node const &target);
    static void     FindTransition(Node const *array, uint arraySize, uint index, float sx, float y, uint &newIndex, float &ex);
    static float    YAxisEquidistant(Node const &p1, Node const &p2, float x);
    static float    XAxisEquidistant(Node const &p1, Node const &p2, float y);
    void            cleanEdges(vector<Edge> &edges) const;
    void            Triangulate(Node *nodes) const;
};

#ifdef _MSC_VER
//dec(PEffect33, peffect33_0, 16);
//dec(PEffect33, peffect33_1, 64);
dec(PEffect33, peffect33_2, 256);
//class PEffect33 peffect33_2("peffect33_2(256)", 256);
#endif

uint perfxx;

PEffect33::PEffect33(char const *tag, int n)
{
    //SetControl(0, 0, 0.0f);
    m_number = n;
    //m_count = 2;
    //m_tag = tag;
    srand32(0);
LARGE_INTEGER xx;
QueryPerformanceFrequency(&xx);
perfxx = xx.LowPart;
}

PEffect33::~PEffect33()
{
}

void    PEffect33::GenerateOne(uchar **data, int w, int h) const
{
    static StaticPerf sp("GenerateOne");
    {
    Perf perf(sp);

    Node points[256];

    static StaticPerf sp1("SetupPoints");
    {
    Perf perf(sp1);

    points[0].SetXY(rndf32() * (float) w, rndf32() * (float) h);

    for (int i = 1; i < m_number; i++)
    {
        for (;;)
        {
            points[i].SetXY(rndf32() * (float) w, rndf32() * (float) h);

            int j;
            for (j = 0; j < i - 1; j++)
            {
                float const dx = points[i].X() - points[j].X();
                float const dy = points[i].Y() - points[j].Y();
                if (dx > -10.0f && dx < 10.0f && dy > -10.0f && dy < 10.0f || dx > -1.0f && dx < 1.0f || dy > -1.0f && dy < 1.0f)
                {
                    break;
                }
            }
            if (j == i - 1)
            {
                break;
            }
        }
    }

    }

    Triangulate(points);

    int const mult = 256 / m_number;

    for (int y = 0; y < h; y++)
    {
        Node p(0.0f, (float) y);
        uint index = FindClosest(points, m_number, p);
        float sx = 0.0f;
        float ex = 0.0f;
        do
        {
            uint newIndex;
            FindTransition(points, m_number, index, sx, (float) y, newIndex, ex);
            if (ex > (float) w || ex < 0.0f)
            {
                ex = (float) w;
            }
            int const isx = (int) sx;
            int const iex = (int) ex;
            if (iex > isx)
            {
                memset(&data[y][isx], index * mult, iex - isx);
            }
            sx = ex;
            index = newIndex;
        } while (ex < (float) w);
    }

    }

    Util::RecordError("GenerateOne %d nodes, %d ticks / sec", m_number, perfxx);

    StaticPerf::Dump();
    StaticPerf::Reset();

    /*
    for (uint y = 0; y < h; y++)
    {
        Point p;
        p.m_y = (float) y;
        for (uint x = 0; x < w; x++)
        {
            p.m_x = (float) x;
            uint const index = FindClosest(points, 256, p);
            data[y][x] = (uchar) index;
        }
    }
    */
}

bool    PEffect33::Compare(Point const &a, Point const &b)
{
    return a.X() < b.X();
}

uint    PEffect33::FindClosest(Node const *array, uint arraySize, Node const &target)
{
    static StaticPerf sp("FindClosest");
    Perf perf(sp);

    float distanceCap = 1.0e20f;
    uint index = 0;
    for (uint i = 0; i < arraySize; i++)
    {
        float const distance = target.DistanceSq(array[i]);
        if (distance < distanceCap)
        {
            distanceCap = distance;
            index = i;
        }
    }
    return index;
}

void    PEffect33::FindTransition(Node const *array, uint arraySize, uint index, float sx, float y, uint &newIndex, float &ex)
{
    static StaticPerf sp("FindTransition");
    Perf perf(sp);

    Node const &current = array[index];
    ex = 1.0e6f;
    for (uint i = 0; i < arraySize; i++)
    {
        if (i == index)
        {
            continue;
        }
        float const nex = XAxisEquidistant(current, array[i], y);
        if (nex > sx && nex < ex)
        {
            ex = nex;
            newIndex = i;
        }
        else if (nex > sx && nex == ex && false)
        {
            // This is very unlikely, but we still cover for it.  ex, y is a "triple point" meaning three lines meet here.
            // Without this there's a 50/50 chance we'll select the wrong index coming out of here.
            // points at index and newIndex are the same distance from ex, y.  We can select which to use by moving a short
            // distance to the right and checking distances again.  Whichever is shorter is the one we should use
            Node const test(ex + 0.01f, y);
            float const distanceSqI = test.DistanceSq(array[i]);
            float const distanceSqNewIndex = test.DistanceSq(array[newIndex]);
            if (distanceSqI < distanceSqNewIndex)
            {
                ex = nex;
                newIndex = i;
            }
        }
    }
}

// Transpose about the line x == y and call XAxisEquidistant.
float   PEffect33::YAxisEquidistant(Node const &p1, Node const &p2, float x)
{
    static StaticPerf sp("YAxisEquidistant");
    Perf perf(sp);

    Node const p1a(p1.Y(), p1.X());
    Node const p2a(p2.Y(), p2.X());
    return XAxisEquidistant(p1a, p2a, x);
}

float   PEffect33::XAxisEquidistant(Node const &p1, Node const &p2, float y)
{
    static StaticPerf sp("XAxisEquidistant");
    Perf perf(sp);

    float const perpy = p2.X() - p1.X();
    if (perpy == 0.0f)
    {
        return 1.0e20f;
    }
    float const perpx = p1.Y() - p2.Y();
    float const midx = (p1.X() + p2.X()) * 0.5f;
    float const midy = (p1.Y() + p2.Y()) * 0.5f;
    float const deltay = midy - y;
    float const deltax = deltay * perpx / perpy;
    return midx - deltax;
}

void    PEffect33::cleanEdges(vector<Edge>& edges) const
{
    // remove edges, that exist in list two times (in both directions)

    // mark edges to delete
    vector<bool> toDelete(edges.size(), false);
    map<uint, int> found;

    for (int i = 0; i < ((int)edges.size()) - 1; ++i)
    {
        uint const i1 = edges[i].node1.Index() & 0xffff;
        uint const i2 = edges[i].node2.Index() & 0xffff;
        uint const ident = i1 < i2 ? i1 << 16 | i2 : i2 << 16 | i1;
        auto it = found.find(ident);
        if (it == found.end())
        {
            found[ident] = i;
        }
        else
        {
            toDelete[i] = true;
            toDelete[it->second] = true;
        }
    }

    int dest_idx = 0;
    for (int i = 0; i < (int)edges.size(); ++i)
    {
        if (!toDelete[i])
        {
            edges[dest_idx++] = edges[i];
        }
    }
    edges.resize(dest_idx);
}

void    PEffect33::Triangulate(Node *nodes) const
{
    vector<Point> points(m_number);
    for (int i = 0; i < m_number; i++)
    {
        points[i].SetXY(nodes[i].X(), nodes[i].Y());
        points[i].SetIndex(i);
    }

    sort(points.begin(), points.end());

    static StaticPerf sp("Triangulate");
    Perf perf(sp);

    vector<DTriangle> result;

    result.clear();

    // "normal" triangles
    vector<DTriangle> Triangles;

    // "infinite Triangles" represented by the finite edge
    // we will start with 2 infinite triangles
    vector<Edge> infiniteTriangles(2);

    // this special node represents the "infinite point"
    // within edges to "infinity"
    Point const infiniteNode(0.0f, 0.0f, -1);

    // we use first and last point as starting configuration
    // there are no (finite) triangles, but 2 infinite triangles

    infiniteTriangles[0] = Edge(points[0], points[m_number - 1]);
    infiniteTriangles[1] = Edge(points[m_number - 1], points[0]);

    // insert remaining nodes
    for (int i = 1; i < m_number - 1; i++)
    {
        Point const &currentNode = points[i];

        // remove triangles left of this x
        // we dont have to look at triangles again, that are
        // left of the current x coordinate.
        // left here means, that the circumcircle is left of this x
        // we do not remove any infinite triangle

        int dest_idx = 0;
        for (auto it = Triangles.begin(); it != Triangles.end(); it++)
        {
            //if (currentNode.X() < it->CCCenter.X() - it->CCR)
            if (currentNode.X() > it->CCCenter.X() + it->CCR)
                {
                // move triangle to result
                result.push_back(*it);
            }
            else
            {
                // keep triangle in workset
                Triangles[dest_idx++] = *it;
            }
        }
        Triangles.resize(dest_idx);

        // insert current node:
        // - remove triangles, which circumcircle contains node
        // - add new triangles with current node and edges of eliminated triangles

        // remove triangles, which circumcircle contains node
        vector<Edge> edges;

        dest_idx = 0;
        bool done = false;
        for (uint i = 0; i < infiniteTriangles.size(); i++)
        {
            Edge const &current = infiniteTriangles[i];
            Point const &p1 = current.node1;
            Point const &p2 = current.node2;
            if (p1.X() < p2.X())
            {
                float const y = lerp(p1.Y(), p2.Y(), (currentNode.X() - p1.X()) / (p2.X() - p1.X()));
                if (/* done || currentNode.X() < p1.X() || currentNode.X() > p2.X || */ currentNode.Y() < y)
                {
                    infiniteTriangles[dest_idx++] = current;
                }
                else
                {
                    edges.push_back(current);
                    edges.push_back(Edge(current.node2, infiniteNode));
                    edges.push_back(Edge(infiniteNode, current.node1));
                    done = true;
                }
            }
            else
            {
                float const y = lerp(p2.Y(), p1.Y(), (currentNode.X() - p2.X()) / (p1.X() - p2.X()));
                if (/* done || currentNode.X() < p2.X() || currentNode.X() > p1.X || */ currentNode.Y() > y)
                {
                    infiniteTriangles[dest_idx++] = current;
                }
                else
                {
                    edges.push_back(current);
                    edges.push_back(Edge(current.node2, infiniteNode));
                    edges.push_back(Edge(infiniteNode, current.node1));
                    done = true;
                }
            }
        }
        infiniteTriangles.resize(dest_idx);

        dest_idx = 0;
        for (auto it = Triangles.begin(); it != Triangles.end(); it++)
        {
            if (it->isInsideCC(currentNode))
            {
                // remove triangle
                // keep edges
                // cout << "finite " ;
                edges.push_back(Edge(it->p1, it->p2));
                edges.push_back(Edge(it->p2, it->p3));
                edges.push_back(Edge(it->p3, it->p1));
            }
            else
            {
                // keep triangle
                Triangles[dest_idx++] = *it;
            }
        }
        Triangles.resize(dest_idx);

        cleanEdges(edges);

        // Create new triangles from the edges and the current Node.
        for (auto it = edges.begin(); it != edges.end(); ++it)
        {
            const Edge& currentEdge = *it;
            if (currentEdge.node1 == infiniteNode)
            {
                infiniteTriangles.push_back(Edge(currentEdge.node2, currentNode));
            }
            else if (currentEdge.node2 == infiniteNode)
            {
                infiniteTriangles.push_back(Edge(currentNode, currentEdge.node1));
            }
            else
            {
                Triangles.push_back(DTriangle(currentEdge.node1, currentEdge.node2, currentNode));
            }
        }
    } // all nodes handled

    // insertion finished, add workset to result

    for (auto it = Triangles.begin(); it != Triangles.end(); it++)
    {
        result.push_back(*it);
    }

    for (auto it = result.begin(); it != result.end(); it++)
    {
        DTriangle &current = *it;
        uint const i1 = current.p1.Index();
        Node *p1 = i1 < (uint) m_number ? &nodes[i1] : NULL;
        uint const i2 = current.p2.Index();
        Node *p2 = i2 < (uint) m_number ? &nodes[i2] : NULL;
        uint const i3 = current.p3.Index();
        Node *p3 = i3 < (uint) m_number ? &nodes[i3] : NULL;
        if (p1 != NULL)
        {
            p1->AddNeighbor(p2);
            p1->AddNeighbor(p3);
        }
        if (p2 != NULL)
        {
            p2->AddNeighbor(p1);
            p2->AddNeighbor(p3);
        }
        if (p3 != NULL)
        {
            p3->AddNeighbor(p1);
            p3->AddNeighbor(p2);
        }
    }
}

bool DTriangle::inside(Point const &pp) const
{
    if (!nonDegenerate)
    {
        return false;
    }
    // Compute vectors
    Point const v0 = p2 - p1;
    Point const v1 = p3 - p1;
    Point const vp = pp - p1;

    // Compute dot products
    float const dot00 = v0 * v0;
    float const dot01 = v0 * v1;
    float const dot0p = v0 * vp;
    float const dot11 = v1 * v1;
    float const dot1p = v1 * vp;

    // Compute barycentric coordinates
    float const invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    float const u = (dot11 * dot0p - dot01 * dot1p) * invDenom;
    float const v = (dot00 * dot1p - dot01 * dot0p) * invDenom;

    // Check if point is in triangle
    return u >= 0 && v >= 0 && u + v <= 1;
}

bool DTriangle::isInsideCC(Point const &p) const
{
    // returns if point is in the triangle's circumcircle.
    // a point exactly on the circle is also considered to be in the circle.
    Point const dist2 = p - CCCenter;
    return dist2 * dist2 <= CCR2;
  }

void    DTriangle::initCC()
{
    float const x0 = p1.X();
    float const y0 = p1.Y();

    float const x1 = p2.X();
    float const y1 = p2.Y();

    float const x2 = p3.X();
    float const y2 = p3.Y();

    float const y10 = y1 - y0;
    float const y21 = y2 - y1;

    float const m0 = - (x1 - x0) / y10;
    float const m1 = - (x2 - x1) / y21;

    if (nonDegenerate = m0 != m1)
    {
        float const mx0 = (x0 + x1) * 0.5f;
        float const my0 = (y0 + y1) * 0.5f;

        float const mx1 = (x1 + x2) * 0.5f;
        float const my1 = (y1 + y2) * 0.5f;

        float const cx = (m0 * mx0 - m1 * mx1 + my1 - my0) / (m0 - m1);
        float const cy = m0 * (cx - mx0) + my0;

        CCCenter.SetXY(cx, cy);

        float const dx = x0 - cx;
        float const dy = y0 - cy;

        CCR2 = dx * dx + dy * dy;
        CCR = sqrtf(CCR2);
    }
}







