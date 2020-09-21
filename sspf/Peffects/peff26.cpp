// Split horizontal V

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect26 : public PEffect
{
public:
    PEffect26(char const *tag, int32 d, int32 e);
    virtual ~PEffect26();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect26, peffect26_0, 0, 0);
dec(PEffect26, peffect26_1, 1, 0);
dec(PEffect26, peffect26_2, 0, 1);
dec(PEffect26, peffect26_3, 1, 1);
#endif

PEffect26::PEffect26(char const *tag, int32 d, int32 e)
{
    SetControl(d, 0, 0.0f);
    m_edge = e;
    m_square = 1;
    m_tag = tag;
}

PEffect26::~PEffect26()
{
}

void    PEffect26::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    int32 const h2 = h / 2;
    int32 const t = w * h2;
    float const tf = (float) t;
    for (int32 y = 0; y < h2; y++)
    {
        int32 const y1 = m_edge ? h2 - 1 - y : y;
        int32 const y2 = h - 1 - y1;
        for (int32 x = 0; x < w; x++)
        {
            int32 const c = x * h2 + y * w;
            float const xyf = (float) c / tf;
            float const f = c > t ? 2.0f - xyf : xyf;
            uint8 const v = (uint8) floor(255.0f * f);
            data[y1][x] = v;
            data[y2][x] = v;
        }
    }
}
