// Sparkle box

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect39 : public PEffect
{
public:
    PEffect39(char const *tag, int32 d);
    virtual ~PEffect39();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect39, peffect39_0, 0);
dec(PEffect39, peffect39_1, 1);
#endif

PEffect39::PEffect39(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect39::~PEffect39()
{
}

void    PEffect39::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    for (int32 x = 0; x < (w + 1) / 2; x++)
    {
        int32 const x1 = (w + 1) / 2 - x - 1;
        int32 const x2 = w - x1 - 1;
        float const f = ((float) x * 2.0f) / (float) w;
        for (int32 y = 0; y < (h + 1) / 2; y++)
        {
            int32 const y1 = (h + 1) / 2 - y - 1;
            int32 const y2 = h - y1 - 1;
            float const g = ((float) y * 2.0f) / (float) h;
            uint8 const v = (uint8) Util::Clamp(floor(192.0f * (f > g ? f : g)), 0.0f, 191.0f);
            uint32 const r = rand32();
            data_c[y1][x1] = v + (r & 0x3f);
            data_c[y1][x2] = v + (r >> 8 & 0x3f);
            data_c[y2][x1] = v + (r >> 16 & 0x3f);
            data_c[y2][x2] = v + (r >> 24 & 0x3f);
        }
    }
}
