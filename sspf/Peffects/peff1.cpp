// Fade

#include "pch.h"

#include "sspf.h"
#include "peffect.h"

class PEffect1 : public PEffect
{
public:
    PEffect1(char const *tag);
    virtual ~PEffect1();
    virtual void            GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const;
};

dec(PEffect1, peffect1_0);

PEffect1::PEffect1(char const *tag)
{
    m_control.z = 1.0f;
#ifdef NDEBUG
    m_count = 4;
#endif
    m_tag = tag;
    m_previewOK = true;
}

PEffect1::~PEffect1()
{
}

void    PEffect1::GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const
{
}
