// Boxes from corners

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect21 : public PEffect
{
public:
    PEffect21(char const *tag, int32 s, int32 d);
    virtual ~PEffect21();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect21, peffect21_0, 0, 0);
dec(PEffect21, peffect21_1, 1, 0);
dec(PEffect21, peffect21_2, 0, 1);
dec(PEffect21, peffect21_3, 1, 1);
#endif

PEffect21::PEffect21(char const *tag, int32 s, int32 d)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_tag = tag;
}

PEffect21::~PEffect21()
{
}

void    PEffect21::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    int32 const c = rand32() & 3;
    for (int32 x = 0; x < w; x++)
    {
        int32 const x1 = c & 1 ? w - x - 1 : x;
        float const xf = (float) x1 / (float) w;
        for (int32 y = 0; y < h; y++)
        {
            int32 const y1 = c & 2 ? h - y - 1 : y;
            float const yf = (float) y1 / (float) h;
            float const f = xf < yf ? xf : yf;
            uint8 const v = (uint8) floor(m * f + a);
            data[y][x] = v;
        }
    }
}
