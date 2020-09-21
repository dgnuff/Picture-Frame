// Horizontal shear wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect54 : public PEffect
{
public:
    PEffect54(char const *tag, int32 d, int32 p, int32 o);
    virtual ~PEffect54();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_parts;
    int32   m_orientation;
};

#ifdef NDEBUG
dec(PEffect54, peffect54_0, 0, 0, 0);
dec(PEffect54, peffect54_1, 0, 0, 1);
dec(PEffect54, peffect54_2, 0, 1, 0);
dec(PEffect54, peffect54_3, 0, 1, 1);
dec(PEffect54, peffect54_4, 1, 0, 0);
dec(PEffect54, peffect54_5, 1, 0, 1);
dec(PEffect54, peffect54_6, 1, 1, 0);
dec(PEffect54, peffect54_7, 1, 1, 1);
#endif

PEffect54::PEffect54(char const *tag, int32 d, int32 p, int32 o)
{
    m_parts = p;
    m_orientation = o;
    SetControl(d, 0, 0.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect54::~PEffect54()
{
}

void    PEffect54::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const wf = (float) w;
    float const hf = (float) h;
    for (int32 y = 0; y < h; y += m_parts ? 2 : 1)
    {
        int32 const y1 = m_parts ? y / 2 : y;
        float yf = (float) (m_orientation ? y : h - y - 1);
        for (int32 x = 0; x < w; x++)
        {
            float const xf = (float) x;
            float const f = xf < yf ? xf / yf * 0.5f : 1.0f - (wf - xf - 1) / (hf - yf - 1) * 0.5f;
            uint8 const v = (uint8) floor(256.0f * f);
            data[y1][x] = v;
            if (m_parts)
            {
                data[h - y1 - 1][x] = v;
            }
        }
    }
}
