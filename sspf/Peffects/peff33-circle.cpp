// Voronoi tesselation

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

struct Point;

class PEffect33 : public PEffect
{
public:
    PEffect33(char const *tag, int n);
    virtual ~PEffect33();
    virtual void            GenerateOne(uchar **data, int w, int h) const;

private:
    bool            Circle(int r, uchar **data, int w, int h, Point *points) const;
    bool            CheckPoint(int x, int y, uchar **data, int w, int h, Point *points) const;
    bool            SetPoint(int x, int y, int i, uchar **data) const;

    int             m_number;
    int             m_mult;
    int             m_add;
};

struct Point
{
    int     m_x;
    int     m_y;
};

#ifdef _MSC_VER
//dec(PEffect33, peffect33_0, 16);
//dec(PEffect33, peffect33_1, 64);
dec(PEffect33, peffect33_2, 256);
#endif

PEffect33::PEffect33(char const *tag, int n)
{
    //SetControl(0, 0, 0.0f);
    m_number = n == 256 ? 255 : n;
    m_mult = 256 / n;
    m_add = n == 256 ? 1 : m_mult / 2;
    //m_count = 2;
    m_tag = tag;
}

PEffect33::~PEffect33()
{
}

void    PEffect33::GenerateOne(uchar **data, int w, int h) const
{
    Point points[255];

    for (int y = 0; y < h; y++)
    {
        memset(&data[y][0], 0, w);
    }

    points[0].m_x = rnd32(w);
    points[0].m_y = rnd32(h);
    SetPoint(points[0].m_x, points[0].m_y, 0, data);

    for (int i = 1; i < m_number; i++)
    {
        for (;;)
        {
            int const x = rnd32(w);
            int const y = rnd32(h);

            int j;
            for (j = 0; j < i - 1; j++)
            {
                int const x1 = points[j].m_x;
                int const y1 = points[j].m_y;
                int const dx = x < x1 ? x1 - x : x - x1;
                int const dy = y < y1 ? y1 - y : y - y1;
                if (dx < 10 && dy < 10 || dx == 0 || dy == 0)
                {
                    break;
                }
            }
            if (j == i - 1)
            {
                points[i].m_x = x;
                points[i].m_y = y;
                SetPoint(x, y, i, data);
                break;
            }
        }
    }
    for (int r = 1; Circle(r, data, w, h, points); r++)
        ;
}

bool    PEffect33::Circle(int r, uchar **data, int w, int h, Point *points) const
{
    int x;
    int y;
    int d;
    bool result = false;

    x = 0;
    y = r;
    d = 2 * (1 - r);
    while (y >= x)
    {
        bool const r1 = CheckPoint(x, y, data, w, h, points);
        bool const r2 = CheckPoint(-x, y, data, w, h, points);
        bool const r3 = CheckPoint(x, -y, data, w, h, points);
        bool const r4 = CheckPoint(-x, -y, data, w, h, points);
        bool r5 = false;
        if (y != x)
        {
            bool const r6 = CheckPoint(y, x, data, w, h, points);
            bool const r7 = CheckPoint(-y, x, data, w, h, points);
            bool const r8 = CheckPoint(y, -x, data, w, h, points);
            bool const r9 = CheckPoint(-y, -x, data, w, h, points);
            r5 = r6 || r7 || r8 || r9;
        }
        if (d + y > 0)
        {
            d = d - 2 * --y + 1;
        }
        if (x > d)
        {
            d = d + 2 * ++x + 1;
        }
        if (r1 || r2 || r3 || r4 || r5)
        {
            result = true;
        }
    }
    return result;
}

bool    PEffect33::CheckPoint(int x, int y, uchar **data, int w, int h, Point *points) const
{
    bool found = false;
    for (int i = 0; i < m_number; i++)
    {
        int const x1 = points[i].m_x + x;
        int const y1 = points[i].m_y + y;
        if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h)
        {
            if (SetPoint(x1, y1, i, data))
            {
                found = true;
            }
        }
    }
    return found;
}

bool    PEffect33::SetPoint(int x, int y, int i, uchar **data) const
{
    uchar const v = (uchar) (i * m_mult + m_add);

    if (data[y][x] != 0 && data[y][x] != v)
    {
        return false;
    }
    data[y][x] = v;
    return true;
}
