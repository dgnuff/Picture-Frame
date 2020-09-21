// Horizontal fold up strips

#include "pch.h"

#include "sspf.h"

class SVEffect10 : public SVEffect
{
public:
    SVEffect10(char const *tag, int o);
    virtual         ~SVEffect10();
    virtual int     Init();
    virtual void    GenerateTrans();
    virtual void    GenerateFrames(int framenum, float *frames);
    virtual void    GetTransData(int pass, D3DXVECTOR4 **transforms, unsigned int *numTrans, unsigned int *startIndex, unsigned int *numTri);

private:
    int             m_edge;
    int             m_left;
};

#if 1
dec(SVEffect10, sveffect10_0, 0);
dec(SVEffect10, sveffect10_1, 1);
#endif

SVEffect10::SVEffect10(char const *tag, int o)
{
    m_order = o;
    m_passes = 1;
    //m_edge = e;
    //m_left = l;
    m_tag = tag;
}

SVEffect10::~SVEffect10()
{
}

int     SVEffect10::Init()
{
    static Vertex verts[4 * 8 * 32];
    m_vertices = verts;
    GenerateSquareVertices(32, 8);
    int i = 0;
    for (int y = 0; y < 8; y++)
    {
        float f = 0.0;

        for (int x = 0; x < 32; x++)
        {
            m_vertices[i].z = f;
            m_vertices[i++].w = 0.0f;
            m_vertices[i].z = f + 1.0f;
            m_vertices[i++].w = 0.0f;
            m_vertices[i].z = f;
            m_vertices[i++].w = 0.0f;
            m_vertices[i].z = f + 1.0f;
            m_vertices[i++].w = 0.0f;
            f += 1.0f;
        }

        y++;

        for (int x = 0; x < 32; x++)
        {
            m_vertices[i].z = f;
            m_vertices[i++].w = 1.0f;
            m_vertices[i].z = f - 1.0f;
            m_vertices[i++].w = 1.0f;
            m_vertices[i].z = f;
            m_vertices[i++].w = 1.0f;
            m_vertices[i].z = f - 1.0f;
            m_vertices[i++].w = 1.0f;
            f -= 1.0f;
        }
    }

    m_numVerts = 4 * 8 * 32;

    static unsigned short indices[6 * 8 * 32];
    m_indices = indices;
    GenerateIndices(8 * 32);

    m_numIndex = 6 * 8 * 32;

    return OK;
}

void    SVEffect10::GenerateTrans()
{
    memset(&s_transforms[0][0], 0, 2 * sizeof(AATrans));
    s_transforms[0][0].postTrans.x = 2.0f / 128.0f;
    s_transforms[0][0].postTrans.z = -0.7f / 128.0f;
    s_transforms[0][1].postTrans.x = -2.0f / 128.0f;
    s_transforms[0][1].postTrans.z = -0.7f / 128.0f;
}

void    SVEffect10::GenerateFrames(int framenum, float *frames)
{
    if (frames != NULL)
    {
        memset(frames, 0, sizeof(float) * 66);
        int iframe = m_order ? 256 - framenum : framenum;
        for (int i = 0; i < 66; i++)
        {
            frames[i] = (float) iframe;
            i++;
            frames[i] = (float) iframe;
            iframe -= 8;
            if (iframe < 0)
            {
                iframe = 0;
            }
        }
    }
}

void    SVEffect10::GetTransData(int pass, D3DXVECTOR4 **transforms, unsigned int *numTrans, unsigned int *startIndex, unsigned int *numTri)
{
    if (transforms != NULL)
    {
        *transforms = (D3DXVECTOR4 *) s_transforms[0];
    }
    if (numTrans != NULL)
    {
        *numTrans = 2;
    }
    if (startIndex != NULL)
    {
        *startIndex = 0; // 6 * 32 * 4;
    }
    if (numTri != NULL)
    {
        *numTri = 2 * 32 * 8;
    }
}
