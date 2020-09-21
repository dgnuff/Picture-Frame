// Random sequential corner boxes

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect10 : public PEffect
{
public:
    PEffect10(char const *tag, int32 d, int32 p);
    virtual ~PEffect10();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_param;
};

#ifdef NDEBUG
dec(PEffect10, peffect10_0, 0, 0);
dec(PEffect10, peffect10_1, 1, 0);
dec(PEffect10, peffect10_2, 0, 1);
dec(PEffect10, peffect10_3, 1, 1);
#endif

PEffect10::PEffect10(char const *tag, int32 d, int32 p)
{
    m_param = p;
    SetControl(d, 0, 0.0f);
    m_square = 1;
    m_tag = tag;
}

PEffect10::~PEffect10()
{
}

void    PEffect10::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint32 o[4];

    Util::GeneratePermutation(o, 4);

    for (int32 i = 0; i < 4; i++)
    {
        o[i] *= 64;
    }

    for (int32 x = 0; x < w / 2; x++)
    {
        int32 const x1 = m_param ? x : w / 2 - x - 1;
        int32 const x2 = w - x1 - 1;
        float const f = ((float) x * 2.0f) / (float) w;
        for (int32 y = 0; y < h / 2; y++)
        {
            float const g = ((float) y * 2.0f) / (float) h;
            uint8 const v = (uint8) floor(64.0f * (f > g ? f : g));
            int32 const y1 = m_param ? y : h / 2 - y - 1;
            int32 const y2 = h - y1 - 1;
            data[y1][x1] = v + (uint8) o[0];
            data[y1][x2] = v + (uint8) o[1];
            data[y2][x1] = v + (uint8) o[2];
            data[y2][x2] = v + (uint8) o[3];
        }
    }
}
