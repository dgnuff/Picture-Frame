// Circular iris sparkle

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect40 : public PEffect
{
public:
    PEffect40(char const *tag, int32 d);
    virtual ~PEffect40();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
};

#ifdef NDEBUG
dec(PEffect40, peffect40_0, 0);
dec(PEffect40, peffect40_1, 0);
#endif

PEffect40::PEffect40(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect40::~PEffect40()
{
}

void    PEffect40::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
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
            uint8 const v = (uint8) floor(224.0f * f);
            uint32 const r = rand32();
            data_c[y1][x1] = v + (r & 0x1f);
            data_c[y1][x2] = v + (r >> 8 & 0x1f);
            data_c[y2][x1] = v + (r >> 16 & 0x1f);
            data_c[y2][x2] = v + (r >> 24 & 0x1f);
        }
    }
}
