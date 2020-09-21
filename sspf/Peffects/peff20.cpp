// V slide transition up and down

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect20 : public PEffect
{
public:
    PEffect20(char const *tag, int32 p, int32 d, int32 s);
    virtual ~PEffect20();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_point;
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect20, peffect20_0, 0, 0, 0);
dec(PEffect20, peffect20_1, 1, 0, 0);
dec(PEffect20, peffect20_2, 0, 1, 0);
dec(PEffect20, peffect20_3, 1, 1, 0);
dec(PEffect20, peffect20_4, 0, 0, 1);
dec(PEffect20, peffect20_5, 1, 0, 1);
dec(PEffect20, peffect20_6, 0, 1, 1);
dec(PEffect20, peffect20_7, 1, 1, 1);
#endif

PEffect20::PEffect20(char const *tag, int32 p, int32 d, int32 s)
{
    m_point = p;
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_tag = tag;
}

PEffect20::~PEffect20()
{
}

void    PEffect20::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const dist = (float) (w / 2 + h);
    for (int32 x1 = 0; x1 < (w + 1) / 2; x1++)
    {
        int32 const x2 = w - 1 - x1;
        for (int32 y = 0; y < h; y++)
        {
            int32 const d = m_point ? x1 + y : w / 2 - 1 - x1 + y;
            float const f = (float) d / dist;
            uint8 const v = (uint8) floor(m * f + a);
            data_c[y][x1] = v;
            data_c[y][x2] = v;
        }
    }
}
