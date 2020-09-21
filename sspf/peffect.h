#pragma once

#include "sspf.h"
#include "safevector.h"

struct TileSegment
{
    uint32 start;
    uint32 size;
};

class PEffect
{
public:
    PEffect();
    virtual ~PEffect();
    int32               GetCount() const;
    int32               GetSquare() const;
    char const          *GetTag() const;
    XMFLOAT4 const      *GetControl() const;
    void                Generate(uint8 **data, int32 w, int32 h) const;
    virtual void        PreGenerate();
    virtual void        GenerateOne(uint8 **data, int32 w, int32 h, int32 tx, int32 ty) const = 0;
    static void         InitPEffects(bool previewMode);

protected:
    void                SetControl(int32 d, int32 s, float n);

    int16               m_tilex;
    int16               m_tiley;
    int16               m_count;
    int16               m_square;
    XMFLOAT4            m_control;
    char const          *m_tag;
    bool                m_previewOK;
    void                MakeSegments(TileSegment *segments, uint32 size, uint32 count) const;
};

extern safevector<PEffect *> peffects;
