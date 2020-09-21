// Horizontal split dual sparkle

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::vector;

class PEffect42 : public PEffect
{
public:
    PEffect42(char const *tag, int32 d);
    virtual ~PEffect42();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect42, peffect42_0, 0);
dec(PEffect42, peffect42_1, 1);
#endif

PEffect42::PEffect42(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect42::~PEffect42()
{
}

void    PEffect42::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
#ifdef DEBUG
    vector<Util::SmartArray<uint8>> data_c;
    Util::BuildSmartArray(data_c, data, w, h);
#else
    uint8 **data_c = data;
#endif
    for (int32 y = 0; y < (h + 1) / 2; y++)
    {
        for (int32 x = 0; x < w; x += 2)
        {
            uint32 const r = rand32();
            data_c[y][x] = (uint8) (r & 0x7f);
            data_c[h - y - 1][x] = (uint8) (r >> 8 | 0x80);
            if (x != w - 1)
            {
                data_c[y][x + 1] = (uint8) (r >> 16 & 0x7f);
                data_c[h - y - 1][x + 1] = (uint8) (r >> 24 | 0x80);
            }
        }
    }
}
