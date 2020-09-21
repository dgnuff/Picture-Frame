// Voronoi tesselation

#include "pch.h"

#include "sspf.h"
#include "peffect.h"
#include "util.h"
#include <vector>

using std::vector;

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
        Util::RecordError(" ");
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

class Point
{
public:
    Point();
    Point(float x, float y);
    ~Point();

    float   X() const;
    float   Y() const;
    float   DistanceSq(Point const &other) const;

private:
    float m_x;
    float m_y;
};

class Node
{
friend class Rect;

public:
    Node();
    Node(float x, float y, uint payload);
    ~Node();

    float X() const;
    float Y() const;
    Point const &P() const;
    uint Payload() const;

private:
    Point   m_point;
    uint    m_payload;
};

class Rect
{
public:
    Rect(float x, float y, float w, float h);
    ~Rect();

    float   Left() const;
    float   Top() const;
    float   Right() const;
    float   Bottom() const;

    bool    Intersects(Rect const &other) const;
    bool    Contains(Node const &node) const;

private:
    float   m_left;
    float   m_top;
    float   m_right;
    float   m_bottom;
};

#define CAPACITY    4

class QuadTree
{
public:
    QuadTree(float l, float t, float r, float b);
    ~QuadTree();

    void        Insert(Node const &node);
    void        Search(float l, float t, float r, float b, vector<Node const *> &results) const;
    Node const  *Nearest(float x, float y) const;
    void        Nearest(float x, float y, vector<Node const *> &results, uint count) const;

private:
    Rect        m_boundingBox;
    float       m_midx;
    float       m_midy;

    Node        m_points[CAPACITY];
    uint        m_count;

    QuadTree    *m_nw;
    QuadTree    *m_ne;
    QuadTree    *m_sw;
    QuadTree    *m_se;

    void        Search(Rect const range, vector<Node const *> &results) const;
    Node const  *Nearest(float x, float y, Node const *candidate, float dist) const;
    void        Nearest(float x, float y, vector<Node const *> &results, vector<float> &dists, uint count) const;
    void        PushDown(vector<Node const *> &results, vector<float> &dists, uint count, Node const *result, float dist) const;
    void        Divide();
};

class PEffect33 : public PEffect
{
public:
    PEffect33(char const *tag, int n);
    virtual ~PEffect33();
    virtual void            GenerateOne(uchar **data, int w, int h) const;

private:
    int     m_number;

    //static bool    Compare(Point const &a, Point const &b);
    static uint     FindClosest(Point const *array, uint arraySize, Point const &target);
    static void     FindTransition(Point const *array, uint arraySize, uint index, float sx, float y, uint &newIndex, float &ex);
    static float    YAxisEquidistant(Point const &p1, Point const &p2, float x);
    static float    XAxisEquidistant(Point const &p1, Point const &p2, float y);


    static void     FindTransitionA(QuadTree const &quadTree, Point const &current, uint index, float sx, float y, uint &newIndex, float &ex);

};

#ifdef _MSC_VER
//dec(PEffect33, peffect33_0, 16);
//dec(PEffect33, peffect33_1, 64);
dec(PEffect33, peffect33_2, 256);
#endif

uint perfxx;

PEffect33::PEffect33(char const *tag, int n)
{
    SetControl(0, 0, 0.0f);
    m_number = n;
    m_count = 2;
    m_tag = tag;

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
    Perf *perf = new Perf(sp);

    Point points[256];

    QuadTree quadTree(0.0f, 0.0f, (float) w, (float) h);

    for (int i = 0; i < m_number; i++)
    {
        for (;;)
        {
            points[i] = Point(rndf32() * (float) w, rndf32() * (float) h);
            int j;
            for (j = 0; j < i - 1; j++)
            {
                float const dx = fabs(points[i].X() - points[j].X());
                float const dy = fabs(points[i].Y() - points[j].Y());
                if (dx < 10.0f && dy < 10.0f || dx < 1.0f || dy < 1.0f)
                {
                    break;
                }
            }
            if (j == i - 1 || i == 0)
            {
                break;
            }
        }
        Node const node(points[i].X(), points[i].Y(), i);
        quadTree.Insert(node);
    }

    int const mult = 256 / m_number;

    for (int y = 0; y < h; y++)
    {
        Point p(0.0f, (float) y);
        uint index = FindClosest(points, m_number, p);
        Node const * const node = quadTree.Nearest(0.0f, (float) y);
        uint index1 = node->Payload();
        float sx = 0.0f;
        float ex = 0.0f;
        do
        {
            uint newIndex;
            FindTransition(points, m_number, index, sx, (float) y, newIndex, ex);
            //uint newIndex1;
            //float ex1 = 0.0f;
            //FindTransitionA(quadTree, points[index], index, sx, (float) y, newIndex1, ex1);
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

    delete perf;

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

//bool    PEffect33::Compare(Point const &a, Point const &b)
//{
//    return a.m_y < b.m_y;
//}

uint    PEffect33::FindClosest(Point const *array, uint arraySize, Point const &target)
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

void    PEffect33::FindTransition(Point const *array, uint arraySize, uint index, float sx, float y, uint &newIndex, float &ex)
{
    static StaticPerf sp("FindTransition");
    Perf perf(sp);

    Point const &current = array[index];
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
            Point const test(ex + 0.01f, y);
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

void    PEffect33::FindTransitionA(QuadTree const &quadTree, Point const &current, uint index, float sx, float y, uint &newIndex, float &ex)
{
    static StaticPerf sp("FindTransitionA");
    Perf perf(sp);

    ex = 1.0e6f;
    vector<Node const *> results;
    quadTree.Nearest(current.X(), current.Y(), results, 5);

    for (auto node : results)
    {
        //Node const * const node = results[i];
        if (node == NULL)
        {
            continue;
            // break; ??
        }
        if (node->Payload() == index)
        {
            // i == 0?
            continue;
        }
        float const nex = XAxisEquidistant(current, node->P(), y);
        if (nex > sx && nex < ex)
        {
            ex = nex;
            newIndex = node->Payload();
        }
        else if (nex > sx && nex == ex && false)
        {
            // This is very unlikely, but we still cover for it.  ex, y is a "triple point" meaning three lines meet here.
            // Without this there's a 50/50 chance we'll select the wrong index coming out of here.
            // points at index and newIndex are the same distance from ex, y.  We can select which to use by moving a short
            // distance to the right and checking distances again.  Whichever is shorter is the one we should use
            Point const test(ex + 0.01f, y);
            float const distanceSqI = test.DistanceSq(current);
            float const distanceSqNewIndex = test.DistanceSq(node->P());
            if (distanceSqI < distanceSqNewIndex)
            {
                ex = nex;
                newIndex = node->Payload();
            }
        }
    }
}

// Transpose about the line x == y and call XAxisEquidistant.
float   PEffect33::YAxisEquidistant(Point const &p1, Point const &p2, float x)
{
    static StaticPerf sp("YAxisEquidistant");
    Perf perf(sp);

    Point const p1a(p1.Y(), p1.X());
    Point const p2a(p2.Y(), p2.X());
    return XAxisEquidistant(p1a, p2a, x);
}

float   PEffect33::XAxisEquidistant(Point const &p1, Point const &p2, float y)
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


Point::Point()
{
}

Point::Point(float x, float y) :
    m_x(x),
    m_y(y)
{
}

Point::~Point()
{
}

float   Point::X() const
{
    return m_x;
}

float   Point::Y() const
{
    return m_y;
}

float   Point::DistanceSq(Point const &other) const
{
    float const dx = m_x - other.m_x;
    float const dy = m_y - other.m_y;
    return dx * dx + dy * dy;
}
Node::Node()
{
}

Node::Node(float x, float y, uint payload) :
m_point(x, y),
m_payload(payload)
{
}

Node::~Node()
{
}

float   Node::X() const
{
    return m_point.X();
}

float   Node::Y() const
{
    return m_point.Y();
}

Point const &Node::P() const
{
    return m_point;
}

uint    Node::Payload() const
{
    return m_payload;
}

Rect::Rect(float l, float t, float r, float b) :
m_left(l),
m_top(t),
m_right(r),
m_bottom(b)
{
}

Rect::~Rect()
{
}

float   Rect::Left() const
{
    return m_left;
}

float   Rect::Top() const
{
    return m_top;
}

float   Rect::Right() const
{
    return m_right;
}

float   Rect::Bottom() const
{
    return m_bottom;
}

bool    Rect::Intersects(Rect const &other) const
{
    return other.m_left <= m_right && m_left <= other.m_right &&
            other.m_top <= m_bottom && m_top <= other.m_bottom;
}

bool    Rect::Contains(Node const &node) const
{
    return node.X() <= m_right && m_left <= node.X() &&
            node.Y() <= m_bottom && m_top <= node.Y();
}

QuadTree::QuadTree(float l, float t, float r, float b) :
m_boundingBox(l, t, r, b),
m_midx((l + r) * 0.5f),
m_midy((t + b) * 0.5f),
m_count(0),
m_nw(NULL),
m_ne(NULL),
m_sw(NULL),
m_se(NULL)
{
}

QuadTree::~QuadTree()
{
    delete m_nw;
    delete m_ne;
    delete m_sw;
    delete m_se;
}

void    QuadTree::Insert(Node const &node)
{
    static StaticPerf sp("QuadTree::Insert");
    Perf perf(sp);

    if (!m_boundingBox.Contains(node))
    {
        return;
    }
    if (m_count < CAPACITY)
    {
        m_points[m_count++] = node;
        return;
    }
    if (m_nw == NULL)
    {
        Divide();
    }
    if (node.X() < m_midx)
    {
        if (node.Y() < m_midy)
        {
            m_nw->Insert(node);
        }
        else
        {
            m_sw->Insert(node);
        }
    }
    else
    {
        if (node.Y() < m_midy)
        {
            m_ne->Insert(node);
        }
        else
        {
            m_se->Insert(node);
        }
    }
}

void    QuadTree::Search(float l, float t, float r, float b, vector<Node const *> &results) const
{
    static StaticPerf sp("QuadTree::Search");
    Perf perf(sp);

    Rect const range(l, t, r, b);
    Search(range, results);
}

void    QuadTree::Search(Rect const range, vector<Node const *> &results) const
{
    if (!m_boundingBox.Intersects(range) || m_count == 0)
    {
        return;
    }
    for (uint i = 0; i < m_count; i++)
    {
        if (range.Contains(m_points[i]))
        {
            results.push_back(&m_points[i]);
        }
    }
    if (m_nw != NULL)
    {
        m_nw->Search(range, results);
        m_ne->Search(range, results);
        m_sw->Search(range, results);
        m_se->Search(range, results);
    }
}

Node const  *QuadTree::Nearest(float x, float y) const
{
    static StaticPerf sp("QuadTree::Nearest");
    Perf perf(sp);

    Node const *candidate = Nearest(x, y, NULL, 1.0e20f);
    Point const here(x, y);
    float const distance = sqrtf(here.DistanceSq(candidate->P()));
    Rect const range(x - distance, y - distance, x + distance, y + distance);

    vector<Node const *> results;
    Search(range, results);

    float distanceSq = 1.0e20f;

    for (auto node : results)
    {
        float const distSq = sqrtf(here.DistanceSq(node->P()));
        if (distSq < distanceSq)
        {
            candidate = node;
            distanceSq = distSq;
        }
    }
    return candidate;
}

Node const  *QuadTree::Nearest(float x, float y, Node const *candidate, float dist) const
{
    Point const here(x, y);
    for (uint i = 0; i < m_count; i++)
    {
        float const distance = here.DistanceSq(m_points[i].P());
        if (distance < dist)
        {
            dist = distance;
            candidate = &m_points[i];
        }
    }
    if (m_nw != NULL)
    {
        if (x < m_midx)
        {
            if (y < m_midy)
            {
                candidate = m_nw->Nearest(x, y, candidate, dist);
            }
            else
            {
                candidate = m_sw->Nearest(x, y, candidate, dist);
            }
        }
        else
        {
            if (y < m_midy)
            {
                candidate = m_ne->Nearest(x, y, candidate, dist);
            }
            else
            {
                candidate = m_se->Nearest(x, y, candidate, dist);
            }
        }
    }
    return candidate;
}

void    QuadTree::Nearest(float x, float y, vector<Node const *> &results, uint count) const
{
    static StaticPerf sp("QuadTree::Nearest (2)");
    Perf perf(sp);

    results.resize(count);
    vector<float> dists;
    dists.resize(count);
    for (uint i = 0; i < count; i++)
    {
        results[i] = NULL;
        dists[i] = 1.0e20f;
    }
    Nearest(x, y, results, dists, count);

    Point const here(x, y);
    float const distance = sqrtf(dists[count - 1]);
    Rect const range(x - distance, y - distance, x + distance, y + distance);
    results.clear();
    Search(range, results);
}

void    QuadTree::Nearest(float x, float y, vector<Node const *> &results, vector<float> &dists, uint count) const
{
    Point const here(x, y);
    for (uint i = 0; i < m_count; i++)
    {
        float const distance = here.DistanceSq(m_points[i].P());
        PushDown(results, dists, count, &m_points[i], distance);
    }
    if (m_nw != NULL)
    {
        if (x < m_midx)
        {
            if (y < m_midy)
            {
                m_nw->Nearest(x, y, results, dists, count);
            }
            else
            {
                m_sw->Nearest(x, y, results, dists, count);
            }
        }
        else
        {
            if (y < m_midy)
            {
                m_ne->Nearest(x, y, results, dists, count);
            }
            else
            {
                m_se->Nearest(x, y, results, dists, count);
            }
        }
    }
}

void    QuadTree::PushDown(vector<Node const *> &results, vector<float> &dists, uint count, Node const *result, float dist) const
{
    if (dist >= dists[count - 1])
    {
        return;
    }
    results[count - 1] = result;
    dists[count - 1] = dist;
    int swap = count - 2;
    while (swap >= 0 && dist < dists[swap])
    {
        results[swap + 1] = results[swap];
        results[swap] = result;
        dists[swap + 1] = dists[swap];
        dists[swap] = dist;
        swap--;
    }
}

void    QuadTree::Divide()
{
    m_nw = new QuadTree(m_boundingBox.Left(), m_boundingBox.Top(), m_midx, m_midy);
    m_ne = new QuadTree(m_midx, m_boundingBox.Top(), m_boundingBox.Right(), m_midy);
    m_sw = new QuadTree(m_boundingBox.Left(), m_midy, m_midx, m_boundingBox.Bottom());
    m_se = new QuadTree(m_midx, m_midy, m_boundingBox.Right(), m_boundingBox.Bottom());
}
