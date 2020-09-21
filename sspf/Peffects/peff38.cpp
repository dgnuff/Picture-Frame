// Top to bottom sparkle wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect38 : public PEffect
{
public:
    PEffect38(char const *tag, int32 d);
    virtual ~PEffect38();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect38, peffect38_0, 0);
dec(PEffect38, peffect38_1, 1);
#endif

PEffect38::PEffect38(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect38::~PEffect38()
{
}

void    PEffect38::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h; y++)
    {
        float const f = (float) y / (float) h;
        uint8 const v = (uint8) floor(192.0f * f + 0.0f);
        int32 r;
        for (int32 x = 0; x < w; x++)
        {
            if ((x & 3) == 0)
            {
                r = rand32();
            }
            data[y][x] = v + (r & 0x3f);
            r = r >> 8;
        }
    }
}
