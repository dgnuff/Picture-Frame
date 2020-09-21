// Stair step vertical wipe

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::min;

class PEffect23 : public PEffect
{
public:
    PEffect23(char const *tag, int32 d, int32 a);
    virtual ~PEffect23();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_angle;
};

#ifdef NDEBUG
dec(PEffect23, peffect23_0, 0, 0);
dec(PEffect23, peffect23_1, 1, 0);
dec(PEffect23, peffect23_2, 0, 1);
dec(PEffect23, peffect23_3, 1, 1);
dec(PEffect23, peffect23_4, 0, 2);
dec(PEffect23, peffect23_5, 1, 2);
#endif

PEffect23::PEffect23(char const *tag, int32 d, int32 a)
{
    SetControl(d, 0, 0.0f);
    m_angle = a;
    m_square = 1;
    m_tag = tag;
}

PEffect23::~PEffect23()
{
}

void    PEffect23::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint32    permute[8];

    if (m_angle == 2)
    {
        Util::GeneratePermutation(permute, 8);
    }
    else
    {
        for (int32 i = 0; i < 8; i++)
        {
            permute[i] = m_angle ? 7 - i : i;
        }
    }

    int32 const t = w + h;
    for (int32 x = 0; x < 8; x++)
    {
        int32 const xs = x * w / 8;
        int32 const xe = xs + w / 8;
        int32 const xf = permute[x] * w / 8;
        for (int32 y = 0; y < h; y++)
        {
            float const f = (float) min(xf + y, t);
            uint8 const v = (uint8) floor(f * 255.0f / t);
            for (int32 x1 = xs; x1 < xe; x1++)
            {
                data[y][x1] = v;
            }
        }
    }
}
