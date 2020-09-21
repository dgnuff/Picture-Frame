// Shaped wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect49 : public PEffect
{
public:
    PEffect49(char const *tag, int32 d, int32 e, int32 m);
    virtual ~PEffect49();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
    int32   m_mode;

    float Triangle(int32 value, int32 limit) const;
    float Sin(int32 value, int32 limit) const;
};

#ifdef NDEBUG
dec(PEffect49, peffect49_0, 0, 0, 0);
dec(PEffect49, peffect49_1, 1, 0, 0);
dec(PEffect49, peffect49_2, 0, 1, 0);
dec(PEffect49, peffect49_3, 1, 1, 0);
dec(PEffect49, peffect49_4, 0, 0, 1);
dec(PEffect49, peffect49_5, 1, 0, 1);
dec(PEffect49, peffect49_6, 0, 1, 1);
dec(PEffect49, peffect49_7, 1, 1, 1);
#endif

PEffect49::PEffect49(char const *tag, int32 d, int32 e, int32 m)
{
    SetControl(d, 0, 0.0f);
    m_edge = e;
    m_mode = m;
    m_square = 1;
    m_tag = tag;
}

PEffect49::~PEffect49()
{
}

float PEffect49::Triangle(int32 value, int32 limit) const
{
    float const fv = (float) ((value + limit / 16) % (limit / 4));
    float const fl = (float) (limit / 4);
    float const fl2 = fl / 2.0f;
    float const fl4 = fl / 4.0f;

    if (fv < fl4)
    {
        return fv / fl4;
    }
    if (fv < fl4 * 3.0f)
    {
        return (fl2 - fv) / fl4;
    }
    return (fv - fl) / fl4;
}

float PEffect49::Sin(int32 value, int32 limit) const
{
    float const fv = (float) (value % (limit / 4));
    float const fl = (float) (limit / 4);

    return cos(fv / fl * M_2PI_f);
}

void    PEffect49::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    if (m_edge)
    {
        float const mult = rand32() & 1 ? 0.0625f : -0.0625f;
        float *line = (float *) alloca(h * sizeof(float));
        for (int32 y = 0; y < h; y++)
        {
            line[y] = m_mode ? Sin(y, h) : Triangle(y, h);

        }
        for (int32 x = 0; x < w; x++)
        {
            float const xf = ((float) x / (float) w) * 224.0f - 112.0f;
            for (int32 y = 0; y < h; y++)
            {
                float const c = line[y] * mult + 0.5f;
                float const f = 256.0f * c + xf;
                uint8 const v = (uint8) floor(f);
                data[y][x] = v;
            }
        }
    }
    else
    {
        float const mult = rand32() & 1 ? 0.125f : -0.125f;
        float *line = (float *) alloca(w * sizeof(float));
        for (int32 x = 0; x < w; x++)
        {
            line[x] = m_mode ? Sin(x, w) : Triangle(x, w);
        }
        for (int32 y = 0; y < h; y++)
        {
            float const yf = ((float) y / (float) h) * 192.0f - 96.0f;
            for (int32 x = 0; x < w; x++)
            {
                float const c = line[x] * mult + 0.5f;
                float const f = 256.0f * c + yf;
                uint8 const v = (uint8) floor(f);
                data[y][x] = v;
            }
        }
    }
}
