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
    }

    int const mult = 256 / m_number;

    for (int y = 0; y < h; y++)
    {
        Point p(0.0f, (float) y);
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
