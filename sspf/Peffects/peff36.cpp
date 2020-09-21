// Diagonal split wipes

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect36 : public PEffect
{
public:
    PEffect36(char const *tag, int32 d, int32 l, int32 w);
    virtual ~PEffect36();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_lean;
    int32   m_which;
};

#ifdef NDEBUG
dec(PEffect36, peffect36_0, 0, 0, 0);
dec(PEffect36, peffect36_1, 1, 0, 0);
dec(PEffect36, peffect36_2, 0, 1, 0);
dec(PEffect36, peffect36_3, 1, 1, 0);
dec(PEffect36, peffect36_4, 0, 0, 1);
dec(PEffect36, peffect36_5, 1, 0, 1);
dec(PEffect36, peffect36_6, 0, 1, 1);
dec(PEffect36, peffect36_7, 1, 1, 1);
#endif

PEffect36::PEffect36(char const *tag, int32 d, int32 l, int32 w)
{
    SetControl(d, 0, 0.0f);
    m_lean = l;
    m_which = w;
    m_square = 1;
    m_tag = tag;
}

PEffect36::~PEffect36()
{
}

void    PEffect36::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 x = 0; x < w; x++)
    {
        int32 const x1 = m_lean ? w - 1 - x : x;
        int32 const x2 = w - 1 - x1;
        float const xf = (float) x;
        for (int32 y = 0; y <= x; y++)
        {
            int32 const y2 = h - 1 - y;
            float const yf = (float) y;
            float f = (float) atan2(yf, xf) / M_PI_4_f;
            uint8 const v = (uint8) Util::Clamp(floor(256.0f * f), 0.0f, 255.0f);
            if (m_which)
            {
                data[x1][y] = v;
                data[x2][y2] = v;
            }
            else
            {
                data[y][x1] = v;
                data[y2][x2] = v;
            }
        }
    }
}
