// Horizontal or vertical random lines sparkle wipe

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect44 : public PEffect
{
public:
    PEffect44(char const *tag, int32 d, int32 e);
    virtual ~PEffect44();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect44, peffect44_0, 0, 0);
dec(PEffect44, peffect44_1, 1, 0);
dec(PEffect44, peffect44_2, 0, 1);
dec(PEffect44, peffect44_3, 1, 1);
#endif

PEffect44::PEffect44(char const *tag, int32 d, int32 e)
{
    SetControl(d, 0, 0.0f);
    m_edge = e;
    m_count = 2;
    m_tag = tag;
}

PEffect44::~PEffect44()
{
}

void    PEffect44::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    uint32 r;
    if (m_edge)
    {
        for (int32 y = 0; y < h; y++)
        {
            float const f = (float) y / (float) h;
            if ((y & 3) == 0)
            {
                r = rand32();
            }
            uint32 const v = (uint32) floor(192.0f * f) + (r & 0x3f);
            r = r >> 8;
            memset(&data[y][0], v, w);
        }
    }
    else
    {
        for (int32 x = 0; x < w; x++)
        {
            float const f = (float) x / (float) w;
            if ((x & 3) == 0)
            {
                r = rand32();
            }
            uint8 const v = (uint8) ((uint32) floor(192.0f * f) + (r & 0x3f));
            r = r >> 8;
            data[0][x] = v;
        }
        for (int32 y = 1; y < h; y++)
        {
            memcpy(&data[y][0], &data[0][0], w);
        }
    }
}
