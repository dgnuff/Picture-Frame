// Segmented fan wipe from center

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect52 : public PEffect
{
public:
    PEffect52(char const *tag, int32 d, int32 s);
    virtual ~PEffect52();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_segments;
};

#ifdef NDEBUG
dec(PEffect52, peffect52_0, 0, 16);
dec(PEffect52, peffect52_1, 1, 16);
dec(PEffect52, peffect52_2, 0, 32);
dec(PEffect52, peffect52_3, 1, 32);
#endif

PEffect52::PEffect52(char const *tag, int32 d, int32 s)
{
    SetControl(d, 0, 0.0f);
    m_segments = s;
    m_tag = tag;
}

PEffect52::~PEffect52()
{
}

void    PEffect52::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    static float const aaf[4] = { 0.0f, M_PI_2_f, M_PI_f, M_3PI_2_f };
    int32 const cx = w / 2;
    int32 const cy = h / 2;
    float const aa = aaf[rand32() & 3];

    for (int32 y = 0; y < h; y++)
    {
        float const dy = (float) (y - cy);
        for (int32 x = 0; x < w; x++)
        {
            float const dx = (float) (x - cx);
            float const at = atan2(dy, dx) * 0.999f;
            float const ata = at + aa;
            float const a = fmod(ata, M_2PI_f);
            float const r = a / M_2PI_f;
            float const s = floor(r * (float) m_segments);
            uint8 const v = (uint8) floor(s * 256.0f / m_segments);
            data[y][x] = v;
        }
    }
}
