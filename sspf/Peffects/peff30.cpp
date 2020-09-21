// 4 by 4 checkerboard dual fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect30 : public PEffect
{
public:
    PEffect30(char const *tag, int32 d);
    virtual ~PEffect30();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect30, peffect30_0, 0);
dec(PEffect30, peffect30_1, 1);
#endif

PEffect30::PEffect30(char const *tag, int32 d)
{
    SetControl(d, 1, 2.0f);
    m_count = 2;
    m_square = 1;
    m_tag = tag;
}

PEffect30::~PEffect30()
{
}

void    PEffect30::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < 4; y++)
    {
        int32 const ys = y * h / 4;
        int32 const ye = ys + h / 4;
        for (int32 x = 0; x < 4; x++)
        {
            int32 const xs = x * w / 4;
            int32 const xw = w / 4;
            for (int32 y1 = ys; y1 < ye; y1++)
            {
                memset(&data[y1][xs], 64 + (((x ^ y) & 1) << 7), xw);
            }
        }
    }
}
