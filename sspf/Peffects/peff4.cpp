// Top to bottom wipe

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect4 : public PEffect
{
public:
    PEffect4(char const *tag, int32 d, int32 s, int32 t);
    virtual ~PEffect4();
    virtual void            PreGenerate();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_sequence;
    uint32  m_permutation[8];
};

#ifdef NDEBUG
dec(PEffect4, peffect4_0, 0, 0, 1);
dec(PEffect4, peffect4_1, 0, 1, 1);
dec(PEffect4, peffect4_2, 1, 0, 1);
dec(PEffect4, peffect4_3, 1, 1, 1);
dec(PEffect4, peffect4_4, 0, 0, 2);
dec(PEffect4, peffect4_5, 0, 1, 2);
dec(PEffect4, peffect4_6, 1, 0, 2);
dec(PEffect4, peffect4_7, 1, 1, 2);
dec(PEffect4, peffect4_8, 0, 0, 4);
dec(PEffect4, peffect4_9, 0, 1, 4);
dec(PEffect4, peffect4_a, 1, 0, 4);
dec(PEffect4, peffect4_b, 1, 1, 4);
dec(PEffect4, peffect4_c, 0, 0, 8);
dec(PEffect4, peffect4_d, 0, 1, 8);
dec(PEffect4, peffect4_e, 1, 0, 8);
dec(PEffect4, peffect4_f, 1, 1, 8);
#endif

PEffect4::PEffect4(char const *tag, int32 d, int32 s, int32 t)
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

PEffect4::~PEffect4()
{
}

void    PEffect4::PreGenerate()
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

void    PEffect4::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_sequence ? 128.0f : m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const s = m_sequence ? (float) (128 / (m_tiley - 1)) : 0.0f;
    for (int32 y = 0; y < h; y++)
    {
        float const f = (float) y / (float) h;
        uint8 const v = (uint8) floor(m * f + a + s * (float) m_permutation[ty]);
        for (int32 x = 0; x < w; x++)
        {
            data[y][x] = v;
        }
    }
}
