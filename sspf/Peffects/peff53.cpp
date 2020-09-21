// Grid plasma transition

#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::max;
using std::min;

class PEffect53 : public PEffect
{
public:
    PEffect53(char const *tag, int32 w, int32 s, int32 n);
    virtual ~PEffect53();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_width;
    int32   m_factor;
};

#ifdef NDEBUG
dec(PEffect53, PEffect53_0, 16, 0, 0);
dec(PEffect53, PEffect53_1, 16, 1, 32);
dec(PEffect53, PEffect53_2, 48, 0, 0);
dec(PEffect53, PEffect53_3, 48, 1, 16);
dec(PEffect53, PEffect53_4, 80, 0, 0);
dec(PEffect53, PEffect53_5, 80, 1, 8);
#endif

PEffect53::PEffect53(char const *tag, int32 w, int32 s, int32 n)
{
    SetControl(0, s, (float) n);
    m_width = w;
    m_factor = n;
    m_tag = tag;
}

PEffect53::~PEffect53()
{
}

void    PEffect53::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    float points[46][81];
    float filtered[46][81];
    TileSegment xSegments[80];
    TileSegment ySegments[45];

    // Box filter taken from https://homepages.inf.ed.ac.uk/rbf/HIPR2/gsmooth.htm
    static float const filter[5][5] =
    {
        {  1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f,  1.0f / 273.0f },
        {  4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f,  4.0f / 273.0f },
        {  7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f,  7.0f / 273.0f },
        {  4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f,  4.0f / 273.0f },
        {  1.0f / 273.0f,  4.0f / 273.0f,  7.0f / 273.0f,  4.0f / 273.0f,  1.0f / 273.0f }
    };

    int32 const height = (m_width / 16) * 9;
    float const m = 256.0f -(float)  m_factor;
    float const a = (float) m_factor * 0.5f;

    for (int32 y = 0; y < height + 1; y++)
    {
        for (int32 x = 0; x < m_width + 1; x++)
        {
            points[y][x] = sin(rndf32() * M_2PI_f);
        }
    }

    float top = -1000.0f;
    float bottom = 1000.0f;
    for (int32 y = 0; y < height + 1; y++)
    {
        for (int32 x = 0; x < m_width + 1; x++)
        {
            float s = 0.0f;
            for (int32 dy = -2; dy <= 2; dy++)
            {
                int yc = y + dy;
                if (yc < 0)
                {
                    yc += height + 1;
                }
                else if (yc >= height + 1)
                {
                    yc -= height + 1;
                }
                for (int dx = -2; dx <= 2; dx++)
                {
                    int xc = x + dx;
                    if (xc < 0)
                    {
                        xc += m_width + 1;
                    }
                    else if (xc >= m_width + 1)
                    {
                        xc -= m_width + 1;
                    }
                    float const f = filter[dy + 2][dx + 2];
                    s += points[yc][xc] * f;
                }
            }
            filtered[y][x] = s;
            top = max(top, s);
            bottom = min(bottom, s);
        }
    }
    MakeSegments(xSegments, w, m_width);
    MakeSegments(ySegments, h, height);

    float const m0 = 0.999f / (top - bottom);

    for (int32 ys = 0; ys < height; ys++)
    {
        for (int32 xs = 0; xs < m_width; xs++)
        {
            float const p0 = (filtered[ys][xs] - bottom) * m0;
            float const p1 = (filtered[ys][xs + 1] - bottom) * m0;
            float const p2 = (filtered[ys + 1][xs] - bottom) * m0;
            float const p3 = (filtered[ys + 1][xs + 1] - bottom) * m0;

            for (uint32 yp = 0; yp < ySegments[ys].size; yp++)
            {
                float const y = (float) yp / (float) ySegments[ys].size;
                int32 const yc = ySegments[ys].start + yp;

                float const l0 = Util::Lerp(p0, p2, y);
                float const l1 = Util::Lerp(p1, p3, y);

                for (uint32 xp = 0; xp < xSegments[xs].size; xp++)
                {
                    float const x = (float) xp / (float) xSegments[xs].size;
                    int32 const xc = xSegments[xs].start + xp;
                    float const f = Util::Lerp(l0, l1, x);
                    float const vv = m * f + a;
                    uint8 const v = (uint8) floor(m * f + a);
                    data[yc][xc] = v;
                }
            }
        }
    }
}
