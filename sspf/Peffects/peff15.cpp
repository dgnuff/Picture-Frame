// 4 by 4 boxes

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect15 : public PEffect
{
public:
    PEffect15(char const *tag, int32 s);
    virtual ~PEffect15();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect15, peffect15_0, 0);
dec(PEffect15, peffect15_1, 1);
#endif

PEffect15::PEffect15(char const *tag, int32 s)
{
    SetControl(0, s, 16.0f);
    m_count = 2;
    m_square = 1;
    m_tag = tag;
}


PEffect15::~PEffect15()
{
}

void    PEffect15::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint32 used[16];
    int32 x;
    int32 y;
    int32 x1;
    int32 y1;

    Util::GeneratePermutation(used, 16);

    int32 const xs = w / 4;
    int32 const ys = h / 4;
    for (int32 i = 0; i < 16; i++)
    {
        x = used[i];
        y = x >> 2 & 0x03;
        x = x & 0x03;
        x = x * xs;
        y = y * ys;
        uint8 const v = (uint8) (i * 16 + 8);
        for (y1 = 0; y1 < ys; y1++)
        {
            for (x1 = 0; x1 < xs; x1++)
            {
                data[y + y1][x + x1] = v;
            }
        }
    }
}
