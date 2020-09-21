// Clock wipes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect16 : public PEffect
{
public:
    PEffect16(char const *tag, int32 d, int32 s, int32 a, int32 n, int32 t);
    virtual ~PEffect16();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_angle;
    int32   m_sections;
};

#ifdef NDEBUG
dec(PEffect16, peffect16_0, 0, 0, 0, 0, 0);
dec(PEffect16, peffect16_1, 1, 0, 0, 0, 0);
dec(PEffect16, peffect16_2, 0, 1, 0, 0, 0);
dec(PEffect16, peffect16_3, 1, 1, 0, 0, 0);
dec(PEffect16, peffect16_4, 0, 0, 0, 2, 0);
dec(PEffect16, peffect16_5, 1, 0, 0, 2, 0);
dec(PEffect16, peffect16_6, 0, 0, 2, 2, 0);
dec(PEffect16, peffect16_7, 1, 0, 2, 2, 0);
dec(PEffect16, peffect16_8, 0, 0, 0, 3, 0);
dec(PEffect16, peffect16_9, 1, 0, 0, 3, 0);
dec(PEffect16, peffect16_10, 0, 0, 2, 3, 0);
dec(PEffect16, peffect16_11, 1, 0, 2, 3, 0);
dec(PEffect16, peffect16_12, 0, 0, 0, 4, 0);
dec(PEffect16, peffect16_13, 1, 0, 0, 4, 0);
dec(PEffect16, peffect16_14, 0, 0, 2, 4, 0);
dec(PEffect16, peffect16_15, 1, 0, 2, 4, 0);
dec(PEffect16, peffect16_16, 0, 0, 0, 0, 4);
dec(PEffect16, peffect16_17, 1, 0, 0, 0, 4);
dec(PEffect16, peffect16_18, 0, 0, 0, 2, 4);
dec(PEffect16, peffect16_19, 1, 0, 0, 2, 4);
dec(PEffect16, peffect16_20, 0, 0, 1, 4, 4);
dec(PEffect16, peffect16_21, 1, 0, 2, 4, 4);
#endif

PEffect16::PEffect16(char const *tag, int32 d, int32 s, int32 a, int32 n, int32 t)
{
    m_soft = s;
    m_angle = a;
    m_sections = n;
    SetControl(d, s, 16.0f);
    m_tilex = (int16) t;
    m_tiley = (int16) t;
    if (t == 4)
    {
        m_count = 2;
    }
    m_tag = tag;
}


PEffect16::~PEffect16()
{
}

void    PEffect16::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 240.0f : 256.0f;
    float const a = m_soft ? 8.0f : 0.0f;
    float const a2 = m_angle ? 0.5f : 0.0f;
    float const d2 = m_sections == 0 ? M_2PI_f : M_2PI_f / (float) m_sections;
    for (int32 y = 0; y < h; y++)
    {
        float const yf = ((float) y - (float) h / 2.0f);
        for (int32 x = 0; x < w; x++)
        {
            float const xf = ((float) x - (float) w / 2.0f);
            float f = (float) atan2(xf, -yf) / d2 + a2;
            while (f >= 1.0f)
            {
                f -= 1.0f;
            }
            while (f < 0.0f)
            {
                f += 1.0f;
            }
            uint8 const v = (uint8) floor(m * f + a);
            data[y][x] = v;
        }
    }
}
