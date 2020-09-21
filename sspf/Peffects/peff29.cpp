// Vertical split dual fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect29 : public PEffect
{
public:
    PEffect29(char const *tag, int32 d);
    virtual ~PEffect29();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect29, peffect29_0, 0);
dec(PEffect29, peffect29_1, 1);
#endif

PEffect29::PEffect29(char const *tag, int32 d)
{
    SetControl(d, 1, 2.0f);
    m_count = 2;
    m_square = 1;
    m_tag = tag;
}

PEffect29::~PEffect29()
{
}

void    PEffect29::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    memset(&data[0][0], 64, w / 2);
    memset(&data[0][w / 2], 192, w / 2);

    for (int32 y = 1; y < h; y++)
    {
        memcpy(&data[y][0], &data[0][0], w);
    }
}
