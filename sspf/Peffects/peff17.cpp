// Circular iris

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect17 : public PEffect
{
public:
    PEffect17(char const *tag, int32 s, int32 d, int32 t);
    virtual ~PEffect17();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect17, peffect17_0, 0, 0, 0);
dec(PEffect17, peffect17_1, 0, 1, 0);
dec(PEffect17, peffect17_2, 1, 0, 0);
dec(PEffect17, peffect17_3, 1, 1, 0);
dec(PEffect17, peffect17_6, 0, 0, 4);
dec(PEffect17, peffect17_7, 0, 1, 4);
#endif

PEffect17::PEffect17(char const *tag, int32 s, int32 d, int32 t)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_tilex = (int16) t;
    m_tiley = (int16) t;
    if (t == 4)
    {
        m_count = 2;
    }
    m_tag = tag;
}

PEffect17::~PEffect17()
{
}

void    PEffect17::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const t = sqrtf((float) (w * w + h * h));
    for (int32 x = 0; x < (w + 1) / 2; x++)
    {
        int32 const x1 = (w + 1) / 2 - x - 1;
        int32 const x2 = w - x1 - 1;
        for (int32 y = 0; y < (h + 1) / 2; y++)
        {
            int32 const y1 = (h + 1) / 2 - y - 1;
            int32 const y2 = h - y1 - 1;
            float const c = sqrtf((float) (x * x + y * y));
            float const f = c / t;
            uint8 const v = (uint8) floor(m * f + a);
            data_c[y1][x1] = v;
            data_c[y1][x2] = v;
            data_c[y2][x1] = v;
            data_c[y2][x2] = v;
        }
    }
}
