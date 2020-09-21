// Split vertical V

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect25 : public PEffect
{
public:
    PEffect25(char const *tag, int32 d, int32 e);
    virtual ~PEffect25();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect25, peffect25_0, 0, 0);
dec(PEffect25, peffect25_1, 1, 0);
dec(PEffect25, peffect25_2, 0, 1);
dec(PEffect25, peffect25_3, 1, 1);
#endif

PEffect25::PEffect25(char const *tag, int32 d, int32 e)
{
    SetControl(d, 0, 0.0f);
    m_edge = e;
    m_square = 1;
    m_tag = tag;
}

PEffect25::~PEffect25()
{
}

void    PEffect25::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    int32 const w2 = w / 2;
    int32 const t = w2 * h;
    float const tf = (float) t;
    for (int32 x = 0; x < w2; x++)
    {
        int32 const x1 = m_edge ? w2 - 1 - x : x;
        int32 const x2 = w - 1 - x1;
        for (int32 y = 0; y < h; y++)
        {
            int32 const c = x * h + y * w2;
            float const xyf = (float) c / tf;
            float const f = c > t ? 2.0f - xyf : xyf;
            uint8 const v = (uint8) floor(255.0f * f);
            data[y][x1] = v;
            data[y][x2] = v;
        }
    }
}
