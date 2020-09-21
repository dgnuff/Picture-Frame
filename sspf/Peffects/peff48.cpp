// Fan from corners

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect48 : public PEffect
{
public:
    PEffect48(char const *tag, int32 d, int32 s, int32 c);
    virtual ~PEffect48();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_corner;
};

#ifdef NDEBUG
dec(PEffect48, peffect48_0,  0, 0, 0);
dec(PEffect48, peffect48_1,  1, 0, 0);
dec(PEffect48, peffect48_2,  0, 1, 0);
dec(PEffect48, peffect48_3,  1, 1, 0);
dec(PEffect48, peffect48_4,  0, 0, 1);
dec(PEffect48, peffect48_5,  1, 0, 1);
dec(PEffect48, peffect48_6,  0, 1, 1);
dec(PEffect48, peffect48_7,  1, 1, 1);
dec(PEffect48, peffect48_8,  0, 0, 2);
dec(PEffect48, peffect48_9,  1, 0, 2);
dec(PEffect48, peffect48_10, 0, 1, 2);
dec(PEffect48, peffect48_11, 1, 1, 2);
dec(PEffect48, peffect48_12, 0, 0, 3);
dec(PEffect48, peffect48_13, 1, 0, 3);
dec(PEffect48, peffect48_14, 0, 1, 3);
dec(PEffect48, peffect48_15, 1, 1, 3);
#endif

PEffect48::PEffect48(char const *tag, int32 d, int32 s, int32 c)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_corner = c;
    m_tag = tag;
}

PEffect48::~PEffect48()
{
}

void    PEffect48::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    for (int32 y = 0; y < h; y++)
    {
        int const y1 = m_corner & 2 ? h - y - 1 : y;
        float const yf = (float) y;
        for (int32 x = 0; x < w; x++)
        {
            int const x1 = m_corner & 1 ? w - x - 1 : x;
            float const xf = (float) x;
            float f = (atan2f(yf, xf) / M_PI_2_f);
            if (f < -0.999f)
            {
                f = -0.999f;
            }
            if (f > 0.999f)
            {
                f = 0.999f;
            }
            uint8 const v = (uint8) floor(m * f + a);
            data[y1][x1] = v;
        }
    }
}
