// 16 by 16 random boxes

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect5 : public PEffect
{
public:
    PEffect5(char const *tag);
    virtual ~PEffect5();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect5, peffect5_0);
#endif

PEffect5::PEffect5(char const *tag)
{
    SetControl(0, 0, 0.0f);
    m_count = 4;
    m_square = 1;
    m_tag = tag;
}

PEffect5::~PEffect5()
{
}

void    PEffect5::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint32 used[256];

    Util::GeneratePermutation(used, 256);

    int32 const xs = w / 16;
    int32 const ys = h / 16;
    for (int32 i = 0; i < 256; i++)
    {
        int32 const xa = used[i] & 0x0f;
        int32 const ya = used[i] >> 4 & 0x0f;
        int32 const x = xa * xs;
        int32 const y = ya * ys;
        for (int32 y1 = 0; y1 < ys; y1++)
        {
            for (int32 x1 = 0; x1 < xs; x1++)
            {
                data[y + y1][x + x1] = i;
            }
        }
    }
}
