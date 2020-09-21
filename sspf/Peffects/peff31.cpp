// Diagonal dual fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect31 : public PEffect
{
public:
    PEffect31(char const *tag, int32 d, int32 l);
    virtual ~PEffect31();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_lean;
};

#ifdef NDEBUG
dec(PEffect31, peffect31_0, 0, 0);
dec(PEffect31, peffect31_1, 1, 0);
dec(PEffect31, peffect31_2, 0, 1);
dec(PEffect31, peffect31_3, 1, 1);
#endif

PEffect31::PEffect31(char const *tag, int32 d, int32 l)
{
    SetControl(d, 1, 2.0f);
    m_lean = l;
    m_count = 2;
    m_tag = tag;
}

PEffect31::~PEffect31()
{
}

void    PEffect31::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h; y++)
    {
        int32 const y1 = m_lean ? y : h - y - 1;
        float const yr = (float) y / (float) h;
        for (int32 x = 0; x < w; x++)
        {
            float const xr = (float) x / (float) w;
            uint8 const v = xr > yr ? 192 : 64;
            data[y1][x] = v;
        }
    }
}
