// 8 by 4 "brick" wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect45 : public PEffect
{
public:
    PEffect45(char const *tag, int32 d);
    virtual ~PEffect45();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect45, peffect45_0, 0);
dec(PEffect45, peffect45_1, 1);
#endif

PEffect45::PEffect45(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tilex = 4;
    m_tiley = 4;
    m_tag = tag;
}

PEffect45::~PEffect45()
{
}

void    PEffect45::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    int32 const h2 = h / 2;
    for (int32 x = 0; x < w; x++)
    {
        float const f = (float) x / (float) w;
        uint8 const v = (uint8) floor(f * 256.0f);
        uint8 const v2 = v ^ 0x80;
        int32 y;
        for (y = 0; y < h2; y++)
        {
            data[y][x] = v;
        }
        for (; y < h; y++)
        {
            data[y][x] = v2;
        }
    }
}
