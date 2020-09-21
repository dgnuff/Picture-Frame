// Horizontal or vertical random lines

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect27 : public PEffect
{
public:
    PEffect27(char const *tag, int32 e);
    virtual ~PEffect27();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;

private:
    int32   m_edge;
};

#ifdef NDEBUG
dec(PEffect27, peffect27_0, 0);
dec(PEffect27, peffect27_1, 1);
#endif

PEffect27::PEffect27(char const *tag, int32 e)
{
    SetControl(0, 0, 32.0f);
    m_edge = e;
    m_count = 2;
    m_tag = tag;
}

PEffect27::~PEffect27()
{
}

void    PEffect27::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    if (m_edge)
    {
        for (int32 y = 0; y < h; y++)
        {
            memset(&data[y][0], rand32() & 0xff, w);
        }
    }
    else
    {
        int32 x; // Can't declare x in the for loop, we need it below
        for (x = 0; x < w - 3; x += 4)
        {
            uint32 const r = rand32();
            memcpy(&data[0][x], &r, 4);
        }
        for (; x < w; x++)
        {
            data[0][x] = rand32() & 0xff;
        }
        for (int32 y = 1; y < h; y++)
        {
            memcpy(&data[y][0], &data[0][0], w);
        }
    }
}
