// split side to side / up and down wipe

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

class PEffect22 : public PEffect
{
public:
    PEffect22(char const *tag, int32 d, int32 a, int32 q, int32 s);
    virtual ~PEffect22();
    virtual void            PreGenerate();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_align;
    int32   m_sequence;
    int32   m_sections;
    uint32  m_permutation[8];
};

#ifdef NDEBUG
dec(PEffect22, peffect22_0, 0, 0, 0, 2);
dec(PEffect22, peffect22_1, 1, 0, 0, 2);
dec(PEffect22, peffect22_2, 0, 1, 0, 2);
dec(PEffect22, peffect22_3, 1, 1, 0, 2);
dec(PEffect22, peffect22_4, 0, 0, 1, 2);
dec(PEffect22, peffect22_5, 1, 0, 1, 2);
dec(PEffect22, peffect22_6, 0, 1, 1, 2);
dec(PEffect22, peffect22_7, 1, 1, 1, 2);
dec(PEffect22, peffect22_8, 0, 0, 0, 4);
dec(PEffect22, peffect22_9, 1, 0, 0, 4);
dec(PEffect22, peffect22_a, 0, 1, 0, 4);
dec(PEffect22, peffect22_b, 1, 1, 0, 4);
dec(PEffect22, peffect22_c, 0, 0, 1, 4);
dec(PEffect22, peffect22_d, 1, 0, 1, 4);
dec(PEffect22, peffect22_e, 0, 1, 1, 4);
dec(PEffect22, peffect22_f, 1, 1, 1, 4);
dec(PEffect22, peffect22_g, 0, 0, 0, 8);
dec(PEffect22, peffect22_h, 1, 0, 0, 8);
dec(PEffect22, peffect22_i, 0, 1, 0, 8);
dec(PEffect22, peffect22_j, 1, 1, 0, 8);
dec(PEffect22, peffect22_k, 0, 0, 1, 8);
dec(PEffect22, peffect22_l, 1, 0, 1, 8);
dec(PEffect22, peffect22_m, 0, 1, 1, 8);
dec(PEffect22, peffect22_n, 1, 1, 1, 8);
#endif

PEffect22::PEffect22(char const *tag, int32 d, int32 a, int32 q, int32 s)
{
    SetControl(d, 0, 0.0f);
    m_align = a;
    m_sequence = q;
    m_sections = s;
    m_square = 1;
    m_tag = tag;
}

PEffect22::~PEffect22()
{
}

void    PEffect22::PreGenerate()
{
    if (m_sequence)
    {
        switch (rand32() & 3)
        {
        case 0:
        case 1:
            Util::GeneratePermutation(m_permutation, m_sections);
            break;
        case 2:
            for (uint16 i = 0; i < m_sections; i++)
            {
                m_permutation[i] = i;
            }
            break;
        case 3:
            for (uint16 i = 0; i < m_sections; i++)
            {
                m_permutation[i] = m_sections - i - 1;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        memset(m_permutation, 0, m_sections * sizeof(uint32));
    }
}

void    PEffect22::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float const m = m_sequence ? 128.0f : 256.0f;
    float const s = m_sequence ? (float) (128 / (m_sections - 1)) : 0.0f;
    if (m_align)
    {
        for (int32 y = 0; y < h; y++)
        {
            float const f = (float) y / (float) h;
            for (int32 x = 0; x < w; x++)
            {
                int32 p = x * m_sections / w;
                uint8 const v = (uint8) floor(f * m + s * (float) m_permutation[p]);
                int32 const y1 = x * m_sections / w & 1 ? y : h - y - 1;
                data[y1][x] = v;
            }
        }
    }
    else
    {
        for (int32 x = 0; x < w; x++)
        {
            float const f = (float) x / (float) w;
            for (int32 y = 0; y < h; y++)
            {
                int32 p = y * m_sections / h;
                uint8 const v = (uint8) floor(f * m + s * (float) m_permutation[p]);
                int32 const x1 = y * m_sections / h & 1 ? x : w - x - 1;
                data[y][x1] = v;
            }
        }
    }
}
