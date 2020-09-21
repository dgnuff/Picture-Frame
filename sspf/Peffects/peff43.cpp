// Vertical split dual sparkle

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect43 : public PEffect
{
public:
    PEffect43(char const *tag, int32 d);
    virtual ~PEffect43();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect43, peffect43_0, 0);
dec(PEffect43, peffect43_1, 1);
#endif

PEffect43::PEffect43(char const *tag, int32 d)
{
    SetControl(d, 0, 0.0f);
    m_count = 2;
    m_tag = tag;
}

PEffect43::~PEffect43()
{
}

void    PEffect43::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h; y++)
    {
        uint32 r;
        for (int32 x = 0; x < w; x++)
        {
            if ((x & 3) == 0)
            {
                r = rand32();
            }
            data[y][x] = (uint8) (x < w / 2 ? r & 0x7f : r | 0x80);
            r = r >> 8;
        }
    }
}
