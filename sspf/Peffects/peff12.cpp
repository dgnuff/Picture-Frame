// Diamond wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect12 : public PEffect
{
public:
    PEffect12(char const *tag, int32 s, int32 d, int32 t);
    virtual ~PEffect12();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect12, peffect12_0, 0, 0, 0);
dec(PEffect12, peffect12_1, 0, 1, 0);
dec(PEffect12, peffect12_2, 1, 0, 0);
dec(PEffect12, peffect12_3, 1, 1, 0);
dec(PEffect12, peffect12_4, 0, 0, 4);
dec(PEffect12, peffect12_5, 0, 1, 4);
#endif

PEffect12::PEffect12(char const *tag, int32 s, int32 d, int32 t)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_square = 1;
    m_tilex = (int16) t;
    m_tiley = (int16) t;
    if (t == 4)
    {
        m_count = 2;
    }
    m_tag = tag;
}

PEffect12::~PEffect12()
{
}

void    PEffect12::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    int32 const t = (w + h) / 2;
    for (int32 x = 0; x < w / 2; x++)
    {
        for (int32 y = 0; y < h / 2; y++)
        {
            float const f = (float) (x + y) / (float) t;
            uint8 const v = (uint8) floor(m * f + a);
            int32 const x2 = w - x - 1;
            int32 const y2 = h - y - 1;
            data[y][x] = v;
            data[y][x2] = v;
            data[y2][x] = v;
            data[y2][x2] = v;
        }
    }
}
