// Concurrent corner boxes

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect9 : public PEffect
{
public:
    PEffect9(char const *tag, int32 d, int32 p, int32 s, int32 a, int32 t);
    virtual ~PEffect9();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_param;
    int32   m_soft;
    int32   m_alt;
};

#ifdef NDEBUG
dec(PEffect9, peffect9_0, 0, 0, 0, 0, 1);
dec(PEffect9, peffect9_1, 1, 0, 0, 0, 1);
dec(PEffect9, peffect9_2, 0, 1, 0, 0, 1);
dec(PEffect9, peffect9_3, 1, 1, 0, 0, 1);
dec(PEffect9, peffect9_4, 0, 0, 1, 0, 1);
dec(PEffect9, peffect9_5, 1, 0, 1, 0, 1);
dec(PEffect9, peffect9_6, 0, 1, 1, 0, 1);
dec(PEffect9, peffect9_7, 1, 1, 1, 0, 1);
dec(PEffect9, peffect9_8, 0, 0, 0, 0, 4);
dec(PEffect9, peffect9_9, 1, 0, 0, 0, 4);
dec(PEffect9, peffect9_10, 0, 1, 0, 0, 4);
dec(PEffect9, peffect9_11, 1, 1, 0, 0, 4);
dec(PEffect9, peffect9_12, 0, 0, 0, 1, 1);
dec(PEffect9, peffect9_13, 1, 0, 0, 1, 1);
dec(PEffect9, peffect9_14, 0, 1, 0, 1, 1);
dec(PEffect9, peffect9_15, 1, 1, 0, 1, 1);
dec(PEffect9, peffect9_16, 0, 0, 1, 1, 1);
dec(PEffect9, peffect9_17, 1, 0, 1, 1, 1);
dec(PEffect9, peffect9_18, 0, 1, 1, 1, 1);
dec(PEffect9, peffect9_19, 1, 1, 1, 1, 1);
#endif

PEffect9::PEffect9(char const *tag, int32 d, int32 p, int32 s, int32 a, int32 t)
{
    m_param = p;
    m_soft = s;
    m_alt = a;
    SetControl(d, s, 32.0f);
    m_tilex = (int16) t;
    m_tiley = (int16) t;
    m_square = 1;
    m_tag = tag;
    if (s == 0 && a == 0 && p == 0 && t == 1)
    {
        m_previewOK = true;
    }
}

PEffect9::~PEffect9()
{
}

void    PEffect9::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    for (int32 x = 0; x < w / 2; x++)
    {
        int32 const xa = w / 2 - x - 1;
        int32 const x1 = m_param ? x : xa;
        int32 const x2 = w - x1 - 1;
        float const f = ((float) (m_alt ? xa : x) * 2.0f) / (float) w;
        for (int32 y = 0; y < h / 2; y++)
        {
            float const g = ((float) y * 2.0f) / (float) h;
            uint8 const v = (uint8) floor(m * (f > g ? f : g) + a);
            int32 const y1 = m_param ? y : h / 2 - y - 1;
            int32 const y2 = h - y1 - 1;
            data[y1][x1] = v;
            data[y1][x2] = v;
            data[y2][x1] = v;
            data[y2][x2] = v;
        }
    }
}
