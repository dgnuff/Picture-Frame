// Spiral boxes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect6 : public PEffect
{
public:
    PEffect6(char const *tag, int32 c, int32 d);
    virtual ~PEffect6();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32     m_corner;
};

#ifdef NDEBUG
dec(PEffect6, peffect6_0, 0, 0);
dec(PEffect6, peffect6_1, 1, 0);
dec(PEffect6, peffect6_2, 2, 0);
dec(PEffect6, peffect6_3, 3, 0);
dec(PEffect6, peffect6_4, 0, 1);
dec(PEffect6, peffect6_5, 1, 1);
dec(PEffect6, peffect6_6, 2, 1);
dec(PEffect6, peffect6_7, 3, 1);
#endif

PEffect6::PEffect6(char const *tag, int32 c, int32 d)
{
    m_corner = c;
    SetControl(d, 0, 0.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect6::~PEffect6()
{
}

void    PEffect6::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint8 used[16][16];
    int32 x2;
    int32 y2;

    memset(used, 255, 256);
    int32 dx = 1;
    int32 dy = 0;
    int32 v = 0;
    int32 x = 0;
    int32 y = 0;
    while (v < 255)
    {
        used[y][x] = (uint8) v++;
        int32 x1 = x + dx;
        int32 y1 = y + dy;
        if (dx == 1 && (x1 > 15 || used[y1][x1] != 255))
        {
            dx = 0;
            dy = 1;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dy == 1 && (y1 > 15 || used[y1][x1] != 255))
        {
            dx = -1;
            dy = 0;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dx == -1 && (x1 < 0 || used[y1][x1] != 255))
        {
            dx = 0;
            dy = -1;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dy == -1 && (y1 < 0 || used[y1][x1] != 255))
        {
            dx = 1;
            dy = 0;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dx == 1 && (x1 > 15 || used[x1][y1] != 255))
        {
            break;
        }
        x = x1;
        y = y1;
    }

    int32 const xs = w / 16;
    int32 const ys = h / 16;
    int32 const direction = rand32() & 1;
    for (y = 0; y < 16; y++)
    {
        int32 const y1 = y * ys;
        for (x = 0; x < 16; x++)
        {
            int32 const x1 = x * xs;
            int32 const xc = m_corner & 1 ? 15 - x : x;
            int32 const yc = m_corner & 2 ? 15 - y : y;
            v = direction ? used[xc][yc] : used[yc][xc];
            for (y2 = 0; y2 < ys; y2++)
            {
                for (x2 = 0; x2 < xs; x2++)
                {
                    data[y1 + y2][x1 + x2] = (uint8) v;
                }
            }
        }
    }
}
