#include "pch.h"

#include "sspf.h"
#include "util.h"
#include "peffect.h"

using std::function;
using std::vector;

safevector<PEffect *> peffects;

PEffect::PEffect() :
m_tilex(1),
m_tiley(1),
m_count(1),
m_square(0),
m_control(0.0f, 0.0f, 0.0f, 0.0f),
m_tag(""),
m_previewOK(false)
{
    peffects.push_back(this);
}

PEffect::~PEffect()
{
}

int32   PEffect::GetCount() const
{
    return m_count;
}

int32   PEffect::GetSquare() const
{
    return m_square;
}

char const  *PEffect::GetTag() const
{
    return m_tag;
}

XMFLOAT4 const   *PEffect::GetControl() const
{
    return &m_control;
}

void    PEffect::MakeSegments(TileSegment *segments, uint32 size, uint32 count) const
{
    for (uint32 x = 0; x < count; x++)
    {
        float const wfactor = (float) size / (float) count;
        int32 const xs = (int32) ((float) x * wfactor);
        int32 const xe = (int32) ((float) (x + 1) * wfactor);
        int32 const xw = xe - xs;
        segments[x].start = xs;
        segments[x].size = xw;
    }
}

void    PEffect::Generate(uint8 **data, int32 w, int32 h) const
{
    if ((m_tilex == 1 || m_tilex == 2 || m_tilex == 4 || m_tilex == 8 || m_tilex == 16) &&
                (m_tiley == 1 || m_tiley == 2 || m_tiley == 4 || m_tiley == 8 || m_tiley == 16))
    {
        TileSegment xSegments[16];
        TileSegment ySegments[16];

        MakeSegments(xSegments, w, m_tilex);
        MakeSegments(ySegments, h, m_tiley);

        uint8 **data1 = (uint8 **) alloca(h * sizeof(uint8 *));
        if (data1 == NULL)
        {
            GenerateOne(data, w, h, 0, 0);
            return;
        }
        for (int32 x = 0; x < m_tilex; x++)
        {
            for (int32 y = 0; y < h; y++)
            {
                data1[y] = &data[y][xSegments[x].start];
            }
            for (int32 y = 0; y < m_tiley; y++)
            {
                GenerateOne(&data1[ySegments[y].start], xSegments[x].size, ySegments[y].size, x, y);
            }
        }
    }
    else
    {
        GenerateOne(data, w, h, 0, 0);
    }
}

void    PEffect::PreGenerate()
{
}

void    PEffect::SetControl(int32 d, int32 s, float n)
{
    m_control.x = d ? 1.0f : -1.0f;
    m_control.y = d ? -1.0f : 0.0f;
    m_control.z = s ? n : 1024.0f;
    m_control.w = s ? 0.5f : 0.0f;
}

void    PEffect::InitPEffects(bool previewMode)
{
    int32 const numfx = (int32) peffects.size();
    for (int32 i = 0; i < numfx; i++)
    {
        PEffect *peffect = peffects[i];
        if (previewMode && !peffect->m_previewOK)
        {
            continue;
        }
        int32 const count = Util::Clamp(peffect->GetCount(), 1, 8);
        for (int32 j = 1; j < count; j++)
        {
            peffects.push_back(peffect);
        }
    }
}
