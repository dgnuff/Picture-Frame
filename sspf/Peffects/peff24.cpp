// Stair step horizontal wipe

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::min;

class PEffect24 : public PEffect
{
public:
    PEffect24(char const *tag, int32 d, int32 a);
    virtual ~PEffect24();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_angle;
};

#ifdef NDEBUG
dec(PEffect24, peffect24_0, 0, 0);
dec(PEffect24, peffect24_1, 1, 0);
dec(PEffect24, peffect24_2, 0, 1);
dec(PEffect24, peffect24_3, 1, 1);
dec(PEffect24, peffect24_4, 0, 2);
dec(PEffect24, peffect24_5, 1, 2);
#endif

PEffect24::PEffect24(char const *tag, int32 d, int32 a)
{
    SetControl(d, 0, 0.0f);
    m_angle = a;
    m_square = 1;
    m_tag = tag;
}

PEffect24::~PEffect24()
{
}

void    PEffect24::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
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
    for (int32 y = 0; y < 8; y++)
    {
        int32 const ys = y * h / 8;
        int32 const ye = ys + h / 8;
        int32 const yf = permute[y] * w / 8;
        for (int32 x = 0; x < w; x++)
        {
            float const f = (float) min(yf + x, t);
            uint8 const v = (uint8) floor(f * 255.0f / t);
            for (int32 y1 = ys; y1 < ye; y1++)
            {
                data[y1][x] = v;
            }
        }
    }
}
