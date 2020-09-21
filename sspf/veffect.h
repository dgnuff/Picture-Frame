#pragma once

#include "sspf.h"
#include "safevector.h"

#define                     USE_SCISSORS_P1             1
#define                     USE_SCISSORS_P2             2
#define                     USE_STENCIL_P1              4
#define                     USE_STENCIL_P2              8
#define                     IS_DOUBLE                   16

class VEffect
{
public:
    VEffect();
    virtual ~VEffect();
    virtual void            Init() = 0;
    int32                   GetOrder() const;
    int32                   GetPasses() const;
    int32                   GetPasses_p2() const;
    int32                   GetCount() const;
    uint32                  GetUsages() const;
    float                   GetDB_p2() const;
    char const              *GetTag() const;
    virtual void            GetVertexData(Vertex *vertices, uint16 *indices) const;
    virtual void            GetVertexData_p2(Vertex *vertices, uint16 *indices) const;
    virtual void            GenerateTrans(AATrans *transforms, uint32 w, uint32 h) = 0;
    virtual bool            GenerateAlpha (uint8 **data, uint32 width, uint32 height);
    virtual void            GenerateFrames(float frameNum, float *frames, float *frames_p2) const;
    virtual void            GetTransData(int32 pass, float frameNum, uint32 *baseTrans, uint32 *startIndex, uint32 *numTri, uint32 *numTri_p2, uint8 *alpha, D3D11_RECT const **scissor) const;
    static Vertex const     *GetVertices();
    static uint16 const     *GetIndices();
    static void             InitVEffects(bool previewMode);

protected:
    int16                   m_passes;
    int16                   m_passes_p2;
    int16                   m_count;
    int8                    m_order;
    uint8                   m_usages;
    float                   m_db_p2;
    Vertex const            *m_vertices;
    uint32                  m_numVerts;
    Vertex const            *m_vertices_p2;
    uint32                  m_numVerts_p2;
    uint16 const            *m_indices;
    uint32                  m_numIndex;
    uint16 const            *m_indices_p2;
    uint32                  m_numIndex_p2;
    char const              *m_tag;
    AATrans                 *m_transforms;

    virtual void            GenerateFrames(float frameNum, float *frames) const;
    void                    GenerateVertex(Vertex *vertices, int32 i, float x, float y, int32 f, int32 m, float d = 0.0f, float c = 0.0f);
    void                    GenerateRectangles(Vertex *vertices, uint32 xcount, uint32 ycount, D3D11_RECT *rects = NULL, uint32 width = 1024, uint32 height = 1024, float *xFactors = NULL, float *yFactors = NULL);
    void                    GenerateIndices(uint16 *indices, uint32 size);
    void                    GenerateRandomTrans(AATrans *transform, uint32 scrx, uint32 scry, float preX, float preY, float postX, float postY, float spread, float scale) const;
    void                    GenerateShapeAlpha(uint8 **data, uint32 width, uint32 height, uint32 split);
    void                    GenerateRectangleAlpha(uint8 **data, uint32 width, uint32 height, uint32 xcount, uint32 ycount);

private:
    float                   MultFactor(float base, float offset) const;
    void                    ScissorPoint(float &fval, uint32 &ival, uint32 pos, uint32 count, uint32 size) const;

protected:
    static Vertex const     s_vertices[4];
    static Vertex const     s_vertices_p2[4];
    static uint16 const     s_indices[6];

public:
    static bool             s_borderRect;
};

extern safevector<VEffect *> veffects;
