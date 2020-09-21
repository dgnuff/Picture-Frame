// Diagonal wipes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect11 : public PEffect
{
public:
    PEffect11(char const *tag, int32 s, int32 l, int32 d);
    virtual ~PEffect11();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_lean;
};

#ifdef NDEBUG
dec(PEffect11, peffect11_0, 0, 0, 0);
dec(PEffect11, peffect11_1, 1, 0, 0);
dec(PEffect11, peffect11_2, 0, 1, 0);
dec(PEffect11, peffect11_3, 1, 1, 0);
dec(PEffect11, peffect11_4, 0, 0, 1);
dec(PEffect11, peffect11_5, 1, 0, 1);
dec(PEffect11, peffect11_6, 0, 1, 1);
dec(PEffect11, peffect11_7, 1, 1, 1);
#endif

PEffect11::PEffect11(char const *tag, int32 s, int32 l, int32 d)
{
    m_soft = s;
    m_lean = l;
    SetControl(d, s, 32.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect11::~PEffect11()
{
}

void    PEffect11::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    int32 const t = w + h;
    for (int32 x = 0; x < w; x++)
    {
        int32 const x1 = m_lean ? w - 1 - x : x;
        for (int32 y = 0; y < h; y++)
        {
            float const f = (float) (x + y) / (float) t;
            uint8 const v = (uint8) floor(m * f + a);
            data[y][x1] = v;
        }
    }
}
