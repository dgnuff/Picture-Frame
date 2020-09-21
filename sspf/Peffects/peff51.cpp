// Segmented fan wipe from corners

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect51 : public PEffect
{
public:
    PEffect51(char const *tag, int32 d, int32 c);
    virtual ~PEffect51();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_corner;
};

#ifdef NDEBUG
dec(PEffect51, peffect51_0, 0, 0);
dec(PEffect51, peffect51_1, 1, 0);
dec(PEffect51, peffect51_2, 0, 1);
dec(PEffect51, peffect51_3, 1, 1);
dec(PEffect51, peffect51_4, 0, 2);
dec(PEffect51, peffect51_5, 1, 2);
dec(PEffect51, peffect51_6, 0, 3);
dec(PEffect51, peffect51_7, 1, 3);
#endif

PEffect51::PEffect51(char const *tag, int32 d, int32 c)
{
    SetControl(d, 0, 0.0f);
    m_corner = c;
    m_tag = tag;
}

PEffect51::~PEffect51()
{
}

void    PEffect51::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    static int32 const cxf[4] = { 0, 1, 1, 0 };
    static int32 const cyf[4] = { 0, 0, 1, 1 };
    int32 const cx = cxf[m_corner] * w;
    int32 const cy = cyf[m_corner] * h;
    int32 const segments = rand32() & 1 ? 6 : 12;

    for (int32 y = 0; y < h; y++)
    {
        float const dy = (float) (y - cy);
        for (int32 x = 0; x < w; x++)
        {
            float const dx = (float) (x - cx);
            float const at = atan2(dy, dx) * 0.999f;
            float const a = fmod(at, M_PI_2_f);
            float const r = a / M_PI_2_f;
            float const s = floor(r * (float) segments);
            uint8 const v = (uint8) floor(s * 256.0f / segments);
            data[y][x] = v;
        }
    }
}
