// Dual fans from centers of side edges

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect7 : public PEffect
{
public:
    PEffect7(char const *tag, int32 d, int32 s, int32 b);
    virtual ~PEffect7();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_both;
};

#ifdef NDEBUG
dec(PEffect7, peffect7_0, 0, 0, 0);
dec(PEffect7, peffect7_1, 1, 0, 0);
dec(PEffect7, peffect7_2, 0, 1, 0);
dec(PEffect7, peffect7_3, 1, 1, 0);
dec(PEffect7, peffect7_4, 0, 0, 1);
dec(PEffect7, peffect7_5, 1, 0, 1);
dec(PEffect7, peffect7_6, 0, 1, 1);
dec(PEffect7, peffect7_7, 1, 1, 1);
#endif

PEffect7::PEffect7(char const *tag, int32 d, int32 s, int32 b)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_both = b;
    m_tag = tag;
}

PEffect7::~PEffect7()
{
}

void    PEffect7::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
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
    int32 const h1 = (h + 1) / 2;
    int32 const w1 = m_both ? (w + 1) / 2 : w;
    for (int32 y = 0; y < h1; y++)
    {
        int32 const y1 = h - y - 1;
        float const yf = (float) (h1 - y - 1);
        for (int32 x = 0; x < w1; x++)
        {
            float const xf = (float) x;
            float f = atan2f(yf, xf) / M_PI_2_f;
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
                int32 const x1 = w - x - 1;
                data_c[y][x] = v;
                data_c[y][x1] = v;
                data_c[y1][x] = v;
                data_c[y1][x1] = v;
            }
            else
            {
                int32 const x1 = edge ? w - x - 1 : x;
                data_c[y][x1] = v;
                data_c[y1][x1] = v;
            }
        }
    }
}
