// Vertical shear wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect55 : public PEffect
{
public:
    PEffect55(char const *tag, int32 d, int32 p, int32 o);
    virtual ~PEffect55();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_parts;
    int32   m_orientation;
};

#ifdef NDEBUG
dec(PEffect55, peffect55_0, 0, 0, 0);
dec(PEffect55, peffect55_1, 0, 0, 1);
dec(PEffect55, peffect55_2, 0, 1, 0);
dec(PEffect55, peffect55_3, 0, 1, 1);
dec(PEffect55, peffect55_4, 1, 0, 0);
dec(PEffect55, peffect55_5, 1, 0, 1);
dec(PEffect55, peffect55_6, 1, 1, 0);
dec(PEffect55, peffect55_7, 1, 1, 1);
#endif

PEffect55::PEffect55(char const *tag, int32 d, int32 p, int32 o)
{
    m_parts = p;
    m_orientation = o;
    SetControl(d, 0, 0.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect55::~PEffect55()
{
}

void    PEffect55::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const wf = (float) w;
    float const hf = (float) h;
    for (int32 x = 0; x < w; x += m_parts ? 2 : 1)
    {
        int32 const x1 = m_parts ? x / 2 : x;
        float xf = (float) (m_orientation ? x : w - x - 1);
        for (int32 y = 0; y < h; y++)
        {
            float const yf = (float) y;
            float const f = yf < xf ? yf / xf * 0.5f : 1.0f - (hf - yf - 1) / (wf - xf - 1) * 0.5f;
            uint8 const v = (uint8) floor(256.0f * f);
            data[y][x1] = v;
            if (m_parts)
            {
                data[y][w - x1 - 1] = v;
            }
        }
    }
}
