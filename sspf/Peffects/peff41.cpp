// Diagonal sparkle wipes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect41 : public PEffect
{
public:
    PEffect41(char const *tag, int32 d, int32 l);
    virtual ~PEffect41();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_lean;
};

#ifdef NDEBUG
dec(PEffect41, peffect41_0, 0, 0);
dec(PEffect41, peffect41_1, 1, 0);
dec(PEffect41, peffect41_2, 0, 1);
dec(PEffect41, peffect41_3, 1, 1);
#endif

PEffect41::PEffect41(char const *tag, int32 d, int32 l)
{
    SetControl(d, 0, 0.0f);
    m_lean = l;
    m_count = 2;
    m_tag = tag;
}

PEffect41::~PEffect41()
{
}

void    PEffect41::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h; y++)
    {
        float const yf = (float) y / (float) h;
        int32 const y1 = m_lean ? h - 1 - y : y;
        uint32 r;
        for (int32 x = 0; x < w; x++)
        {
            float const xf = (float) x / (float) w;
            float const f = (xf + yf) / 2.0f;
            uint8 const v = (uint8) floor(192.0f * f);
            if ((x & 3) == 0)
            {
                r = rand32();
            }
            data[y1][x] = v + (r & 0x3f);
            r = r >> 8;
        }
    }
}
