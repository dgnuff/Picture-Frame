// Segmented fan wipe from edges

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect50 : public PEffect
{
public:
    PEffect50(char const *tag, int32 d, int32 e);
    virtual ~PEffect50();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect50, peffect50_0, 0, 0);
dec(PEffect50, peffect50_1, 1, 0);
dec(PEffect50, peffect50_2, 0, 1);
dec(PEffect50, peffect50_3, 1, 1);
dec(PEffect50, peffect50_4, 0, 2);
dec(PEffect50, peffect50_5, 1, 2);
dec(PEffect50, peffect50_6, 0, 3);
dec(PEffect50, peffect50_7, 1, 3);
#endif

PEffect50::PEffect50(char const *tag, int32 d, int32 e)
{
    SetControl(d, 0, 0.0f);
    m_edge = e;
    m_tag = tag;
}

PEffect50::~PEffect50()
{
}

void    PEffect50::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    static int32 const cxf[4] = { 1, 2, 1, 0 };
    static int32 const cyf[4] = { 0, 1, 2, 1 };
    static float const aaf[4] = { 0.0f, M_PI_2_f, M_PI_f, M_3PI_2_f };
    int32 const cx = (cxf[m_edge] * w) / 2;
    int32 const cy = (cyf[m_edge] * h) / 2;
    float const aa = aaf[m_edge];
    int32 const segments = (rand32() & 8) + 8;

    for (int32 y = 0; y < h; y++)
    {
        float const dy = (float) (y - cy);
        for (int32 x = 0; x < w; x++)
        {
            float const dx = (float) (x - cx);
            float const at = atan2(dy, dx) * 0.999f;
            float const ata = at + aa;
            float const a = fmod(ata, M_PI_f);
            float const r = a / M_PI_f;
            float const s = floor(r * (float) segments);
            uint8 const v = (uint8) floor(s * 256.0f / segments);
            data[y][x] = v;
        }
    }
}
