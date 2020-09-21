// Vertical curtains

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect13 : public PEffect
{
public:
    PEffect13(char const *tag, int32 s, int32 d, int32 t);
    virtual ~PEffect13();
    virtual void            PreGenerate();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_sequence;
    uint32  m_permutation[4];
};

#ifdef NDEBUG
dec(PEffect13, peffect13_0, 0, 0, 1);
dec(PEffect13, peffect13_1, 0, 1, 1);
dec(PEffect13, peffect13_2, 1, 0, 1);
dec(PEffect13, peffect13_3, 1, 1, 1);
dec(PEffect13, peffect13_4, 0, 0, 2);
dec(PEffect13, peffect13_5, 0, 1, 2);
dec(PEffect13, peffect13_6, 1, 0, 2);
dec(PEffect13, peffect13_7, 1, 1, 2);
dec(PEffect13, peffect13_8, 0, 0, 4);
dec(PEffect13, peffect13_9, 0, 1, 4);
dec(PEffect13, peffect13_a, 1, 0, 4);
dec(PEffect13, peffect13_b, 1, 1, 4);
#endif

PEffect13::PEffect13(char const *tag, int32 s, int32 d, int32 t)
{
    if (t == 1)
    {
        m_soft = s;
        SetControl(d, s, 32.0f);
    }
    else
    {
        m_sequence = s;
        SetControl(d, 0, 0.0f);
    }
    m_tilex = (int16) t;
    m_square = 1;
    m_tag = tag;
}

PEffect13::~PEffect13()
{
}

void    PEffect13::PreGenerate()
{
    if (m_sequence)
    {
        switch (rand32() & 3)
        {
        case 0:
        case 1:
            Util::GeneratePermutation(m_permutation, m_tilex);
            break;
        case 2:
            for (uint16 i = 0; i < m_tilex; i++)
            {
                m_permutation[i] = i;
            }
            break;
        case 3:
            for (uint16 i = 0; i < m_tilex; i++)
            {
                m_permutation[i] = m_tilex - i - 1;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        memset(m_permutation, 0, m_tilex * sizeof(uint32));
    }
}

void    PEffect13::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_sequence ? 128.0f : m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const s = m_sequence ? (float) (128 / (m_tilex - 1)) : 0.0f;
    for (int32 x = 0; x < w / 2; x++)
    {
        float const f = (float) (x * 2.0f) / (float) w;
        uint8 const v = (uint8) floor(m * f + a + s * (float) m_permutation[tx]);
        for (int32 y = 0; y < h; y++)
        {
            data[y][x] = v;
            data[y][w - x - 1] = v;
        }
    }
}
