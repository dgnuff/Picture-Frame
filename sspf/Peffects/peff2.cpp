// Random pixels

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect2 : public PEffect
{
public:
    PEffect2(char const *tag);
    virtual ~PEffect2();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect2, peffect2_0);
#endif

PEffect2::PEffect2(char const *tag)
{
    SetControl(0, 0, 0.0f);
    m_count = 4;
    m_tag = tag;
}

PEffect2::~PEffect2()
{
}

void    PEffect2::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h; y++)
    {
        int32 x; // Can't declare x in the for loop, we need it below
        for (x = 0; x < w - 3; x += 4)
        {
            uint32 r = rand32();
            memcpy(&data[y][x], &r, 4);
        }
        for (; x < w; x++)
        {
            data[y][x] = rand32() & 0xff;
        }
    }
}
