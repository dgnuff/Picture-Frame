// V slide transition side to side

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect19 : public PEffect
{
public:
    PEffect19(char const *tag, int32 p, int32 d, int32 s);
    virtual ~PEffect19();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_point;
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect19, peffect19_0, 0, 0, 0);
dec(PEffect19, peffect19_1, 1, 0, 0);
dec(PEffect19, peffect19_2, 0, 1, 0);
dec(PEffect19, peffect19_3, 1, 1, 0);
dec(PEffect19, peffect19_4, 0, 0, 1);
dec(PEffect19, peffect19_5, 1, 0, 1);
dec(PEffect19, peffect19_6, 0, 1, 1);
dec(PEffect19, peffect19_7, 1, 1, 1);
#endif

PEffect19::PEffect19(char const *tag, int32 p, int32 d, int32 s)
{
    m_point = p;
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_tag = tag;
}

PEffect19::~PEffect19()
{
}

void    PEffect19::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const dist = (float) (w + h / 2);
    for (int32 y1 = 0; y1 < (h + 1) / 2; y1++)
    {
        int32 const y2 = h - 1 - y1;
        for (int32 x = 0; x < w; x++)
        {
            int32 const d = m_point ? x + y1 : x + h / 2 - 1 - y1;
            float const f = (float) d / dist;
            uint8 const v = (uint8) floor(m * f + a);
            data_c[y1][x] = v;
            data_c[y2][x] = v;
        }
    }
}
