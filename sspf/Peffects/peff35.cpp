// Diagonal split wipes

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect35 : public PEffect
{
public:
    PEffect35(char const *tag, int32 s, int32 l, int32 d);
    virtual ~PEffect35();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_lean;
};

#ifdef NDEBUG
dec(PEffect35, peffect35_0, 0, 0, 0);
dec(PEffect35, peffect35_1, 1, 0, 0);
dec(PEffect35, peffect35_2, 0, 1, 0);
dec(PEffect35, peffect35_3, 1, 1, 0);
dec(PEffect35, peffect35_4, 0, 0, 1);
dec(PEffect35, peffect35_5, 1, 0, 1);
dec(PEffect35, peffect35_6, 0, 1, 1);
dec(PEffect35, peffect35_7, 1, 1, 1);
#endif

PEffect35::PEffect35(char const *tag, int32 s, int32 l, int32 d)
{
    m_soft = s;
    m_lean = l;
    SetControl(d, s, 32.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect35::~PEffect35()
{
}

void    PEffect35::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const t = (float) (w + h);
    for (int32 x = 0; x < w; x++)
    {
        int32 const x1 = m_lean ? w - 1 - x : x;
        for (int32 y = 0; y < h; y++)
        {
            float const s = (float) (x + y);
            float const f = (s * 2.0f > t ? t - s : s) * 2.0f / t;
            uint8 const v = (uint8) Util::Clamp((int32) floor(m * f + a), 0, 255);
            data[y][x1] = v;
        }
    }
}
