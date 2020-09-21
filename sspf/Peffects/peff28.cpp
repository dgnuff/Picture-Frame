// Horizontal split dual fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect28 : public PEffect
{
public:
    PEffect28(char const *tag, int32 d);
    virtual ~PEffect28();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

#ifdef NDEBUG
dec(PEffect28, peffect28_0, 0);
dec(PEffect28, peffect28_1, 1);
#endif

PEffect28::PEffect28(char const *tag, int32 d)
{
    SetControl(d, 1, 2.0f);
    m_count = 2;
    m_square = 1;
    m_tag = tag;
}

PEffect28::~PEffect28()
{
}

void    PEffect28::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
    for (int32 y = 0; y < h / 2; y++)
    {
        memset(&data[y][0], 64, w);
        memset(&data[y + h / 2][0], 192, w);
    }
}
