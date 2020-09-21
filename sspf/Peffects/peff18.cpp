// Bowtie

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect18 : public PEffect
{
public:
    PEffect18(char const *tag, int32 s, int32 d);
    virtual ~PEffect18();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect18, peffect18_0, 0, 0);
dec(PEffect18, peffect18_1, 0, 1);
dec(PEffect18, peffect18_2, 1, 0);
dec(PEffect18, peffect18_3, 1, 1);
#endif

PEffect18::PEffect18(char const *tag, int32 s, int32 d)
{
    m_soft = s;
    SetControl(d, s, 32.0f);
    m_tag = tag;
}

PEffect18::~PEffect18()
{
}

void    PEffect18::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const dist = (float) ((w + h) / 2);
    for (int32 y1 = 0; y1 < (h + 1) / 2; y1++)
    {
        int32 const y2 = h - 1 - y1;
        for (int32 x1 = 0; x1 < (w + 1) / 2; x1++)
        {
            int32 const x2 = w - 1 - x1;
            int32 const d = x1 + h / 2 - 1 - y1;
            float const f = (float) d / dist;
            uint8 const v = (uint8) floor(m * f + a);
            data_c[y1][x1] = v;
            data_c[y1][x2] = v;
            data_c[y2][x1] = v;
            data_c[y2][x2] = v;
        }
    }
}
