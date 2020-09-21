// Side to side sparkle wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect37 : public PEffect
{
public:
    PEffect37(char const *tag, int32 d);
    virtual ~PEffect37();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect37, peffect37_0, 0);
dec(PEffect37, peffect37_1, 1);
#endif

PEffect37::PEffect37(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect37::~PEffect37()
{
}

void    PEffect37::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 x = 0; x < w; x++)
    {
        float const f = (float) x / (float) w;
        uint8 const v = (uint8) floor(192.0f * f + 0.0f);
        int32 r;
        for (int32 y = 0; y < h; y++)
        {
            if ((y & 3) == 0)
            {
                r = rand32();
            }
            data[y][x] = v + (r & 0x3f);
            r = r >> 8;
        }
    }
}
