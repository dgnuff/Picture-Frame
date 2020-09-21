// Horizontal curtains

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect14 : public PEffect
{
public:
    PEffect14(char const *tag, int32 s, int32 d, int32 t);
    virtual ~PEffect14();
    virtual void            PreGenerate();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_sequence;
    uint32  m_permutation[4];
};

#ifdef NDEBUG
dec(PEffect14, peffect14_0, 0, 0, 1);
dec(PEffect14, peffect14_1, 0, 1, 1);
dec(PEffect14, peffect14_2, 1, 0, 1);
dec(PEffect14, peffect14_3, 1, 1, 1);
dec(PEffect14, peffect14_4, 0, 0, 2);
dec(PEffect14, peffect14_5, 0, 1, 2);
dec(PEffect14, peffect14_6, 1, 0, 2);
dec(PEffect14, peffect14_7, 1, 1, 2);
dec(PEffect14, peffect14_8, 0, 0, 4);
dec(PEffect14, peffect14_9, 0, 1, 4);
dec(PEffect14, peffect14_a, 1, 0, 4);
dec(PEffect14, peffect14_b, 1, 1, 4);
#endif

PEffect14::PEffect14(char const *tag, int32 s, int32 d, int32 t)
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
    m_tiley = (int16) t;
    m_square = 1;
    m_tag = tag;
}

PEffect14::~PEffect14()
{
}

void    PEffect14::PreGenerate()
{
    if (m_sequence)
    {
        switch (rand32() & 3)
        {
        case 0:
        case 1:
            Util::GeneratePermutation(m_permutation, m_tiley);
            break;
        case 2:
            for (uint16 i = 0; i < m_tiley; i++)
            {
                m_permutation[i] = i;
            }
            break;
        case 3:
            for (uint16 i = 0; i < m_tiley; i++)
            {
                m_permutation[i] = m_tiley - i - 1;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        memset(m_permutation, 0, m_tiley * sizeof(uint32));
    }
}

void    PEffect14::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_sequence ? 128.0f : m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const s = m_sequence ? (float) (128 / (m_tiley - 1)) : 0.0f;
    for (int32 y = 0; y < h / 2; y++)
    {
        float const f = (float) (y * 2.0f) / (float) h;
        uint8 const v = (uint8) floorf(m * f + a + s * (float) m_permutation[ty]);
        for (int32 x = 0; x < w; x++)
        {
            data[y][x] = v;
            data[h - y - 1][x] = v;
        }
    }
}
