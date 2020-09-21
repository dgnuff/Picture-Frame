// Dual fans from centers of top and bottom edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect8 : public PEffect
{
public:
    PEffect8(char const *tag, int32 d, int32 s, int32 b);
    virtual ~PEffect8();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_both;
};

#ifdef NDEBUG
dec(PEffect8, peffect8_0, 0, 0, 0);
dec(PEffect8, peffect8_1, 1, 0, 0);
dec(PEffect8, peffect8_2, 0, 1, 0);
dec(PEffect8, peffect8_3, 1, 1, 0);
dec(PEffect8, peffect8_4, 0, 0, 1);
dec(PEffect8, peffect8_5, 1, 0, 1);
dec(PEffect8, peffect8_6, 0, 1, 1);
dec(PEffect8, peffect8_7, 1, 1, 1);
#endif

PEffect8::PEffect8(char const *tag, int32 d, int32 s, int32 b)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_both = b;
    m_tag = tag;
}

PEffect8::~PEffect8()
{
}

void    PEffect8::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    bool const edge = (rand32() & 1) != 0;
    int32 const w1 = (w + 1) / 2;
    int32 const h1 = m_both ? (h + 1) / 2 : h;
    for (int32 x = 0; x < w1; x++)
    {
        int32 const x1 = w - x - 1;
        float const xf = (float) (w1 - x - 1);
        for (int32 y = 0; y < h1; y++)
        {
            float const yf = (float) y;
            float f = atan2f(xf, yf) / M_PI_2_f;
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            if (f > 0.999f)
            {
                f = 0.999f;
            }
            uint8 const v = (uint8) floor(m * f + a);
            if (m_both)
            {
                int32 const y1 = h - y - 1;
                data_c[y][x] = v;
                data_c[y][x1] = v;
                data_c[y1][x] = v;
                data_c[y1][x1] = v;
            }
            else
            {
                int32 const y1 = edge ? h - y - 1 : y;
                data_c[y1][x] = v;
                data_c[y1][x1] = v;
            }
        }
    }
}
