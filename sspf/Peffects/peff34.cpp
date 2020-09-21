// Diagonal dual wipes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect34 : public PEffect
{
public:
    PEffect34(char const *tag, int32 l, int32 d);
    virtual ~PEffect34();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_lean;
};

#ifdef NDEBUG
dec(PEffect34, peffect34_0, 0, 0);
dec(PEffect34, peffect34_2, 1, 0);
dec(PEffect34, peffect34_4, 0, 1);
dec(PEffect34, peffect34_6, 1, 1);
#endif

PEffect34::PEffect34(char const *tag, int32 l, int32 d)
{
    m_lean = l;
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_square = 1;
    m_tag = tag;
}

PEffect34::~PEffect34()
{
}

void    PEffect34::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    int32 const t2 = (w + h) / 2;
    float const t = (float) (w + h);
    for (int32 x = 0; x < w; x++)
    {
        int32 const x1 = w - 1 - x;
        int32 const x2 = m_lean ? x1 : x;
        for (int32 y = 0; y < h; y++)
        {
            bool const s = x + y > t2;
            float const f = (float) (x1 + y) / t;
            uint8 const v = (uint8) floor(256.0f * (s ? f : 1.0f - f));
            data[y][x2] = v;
        }
    }
}
