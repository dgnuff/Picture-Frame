// Side to side wipe

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect3 : public PEffect
{
public:
    PEffect3(char const *tag, int32 d, int32 s, int32 t);
    virtual ~PEffect3();
    virtual void            PreGenerate();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_soft;
    int32   m_sequence;
    uint32  m_permutation[8];
};

#ifdef NDEBUG
dec(PEffect3, peffect3_0, 0, 0, 1);
dec(PEffect3, peffect3_1, 0, 1, 1);
dec(PEffect3, peffect3_2, 1, 0, 1);
dec(PEffect3, peffect3_3, 1, 1, 1);
dec(PEffect3, peffect3_4, 0, 0, 2);
dec(PEffect3, peffect3_5, 0, 1, 2);
dec(PEffect3, peffect3_6, 1, 0, 2);
dec(PEffect3, peffect3_7, 1, 1, 2);
dec(PEffect3, peffect3_8, 0, 0, 4);
dec(PEffect3, peffect3_9, 0, 1, 4);
dec(PEffect3, peffect3_a, 1, 0, 4);
dec(PEffect3, peffect3_b, 1, 1, 4);
dec(PEffect3, peffect3_c, 0, 0, 8);
dec(PEffect3, peffect3_d, 0, 1, 8);
dec(PEffect3, peffect3_e, 1, 0, 8);
dec(PEffect3, peffect3_f, 1, 1, 8);
#endif

PEffect3::PEffect3(char const *tag, int32 d, int32 s, int32 t)
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

    if (s == 0 && t == 1)
    {
        m_previewOK = true;
    }
}

PEffect3::~PEffect3()
{
}

void    PEffect3::PreGenerate()
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

void    PEffect3::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_sequence ? 128.0f : m_soft ? 224.0f : 256.0f;
    float const a = m_soft ? 16.0f : 0.0f;
    float const s = m_sequence ? (float) (128 / (m_tilex - 1)) : 0.0f;
    for (int32 x = 0; x < w; x++)
    {
        float const f = (float) x / (float) w;
        uint8 const v = (uint8) floor(m * f + a + s * (float) m_permutation[tx]);
        for (int32 y = 0; y < h; y++)
        {
            data[y][x] = v;
        }
    }
}
