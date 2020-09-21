// Fan from centers of side edges

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect46 : public PEffect
{
public:
    PEffect46(char const *tag, int32 d, int32 s, int32 e);
    virtual ~PEffect46();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect46, peffect46_0, 0, 0, 0);
dec(PEffect46, peffect46_1, 1, 0, 0);
dec(PEffect46, peffect46_2, 0, 1, 0);
dec(PEffect46, peffect46_3, 1, 1, 0);
dec(PEffect46, peffect46_4, 0, 0, 1);
dec(PEffect46, peffect46_5, 1, 0, 1);
dec(PEffect46, peffect46_6, 0, 1, 1);
dec(PEffect46, peffect46_7, 1, 1, 1);
#endif

PEffect46::PEffect46(char const *tag, int32 d, int32 s, int32 e)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_edge = e;
    m_tag = tag;
}

PEffect46::~PEffect46()
{
}

void    PEffect46::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    int32 const h1 = h / 2;
    for (int32 y = 0; y < h; y++)
    {
        float const yf = (float) (y - h1);
        for (int32 x = 0; x < w; x++)
        {
            float const xf = (float) x;
            float f = (atan2f(yf, xf) / M_PI_2_f + 1.0f) * 0.5f;
            if (f < -0.999f)
            {
                f = -0.999f;
            }
            if (f > 0.999f)
            {
                f = 0.999f;
            }
            uint8 const v = (uint8) floor(m * f + a);
            int32 const x1 = m_edge != 0 ? w - x - 1 : x;
            data[y][x1] = v;
        }
    }
}
