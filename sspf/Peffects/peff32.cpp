// Bowtie / diamond dual fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect32 : public PEffect
{
public:
    PEffect32(char const *tag, int32 d, int32 a);
    virtual ~PEffect32();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_alt;
};

#ifdef NDEBUG
dec(PEffect32, peffect32_0, 0, 0);
dec(PEffect32, peffect32_1, 1, 0);
dec(PEffect32, peffect32_2, 0, 1);
dec(PEffect32, peffect32_3, 1, 1);
#endif

PEffect32::PEffect32(char const *tag, int32 d, int32 a)
{
    SetControl(d, 1, 2.0f);
    m_alt = a;
    m_square = 1;
    m_tag = tag;
}

PEffect32::~PEffect32()
{
}

void    PEffect32::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h / 2; y++)
    {
        int32 const y1 = m_alt ? y : h / 2 - y - 1;
        int32 const y2 = h - y1 - 1;
        float const yr = (float) y / (float) h;
        for (int32 x1 = 0; x1 < w / 2; x1 ++)
        {
            int32 const x2 = w - x1 - 1;
            float const xr = (float) x1 / (float) w;
            uint8 const v = xr > yr ? 192 : 64;
            data[y1][x1] = v;
            data[y1][x2] = v;
            data[y2][x1] = v;
            data[y2][x2] = v;
        }
    }
}
