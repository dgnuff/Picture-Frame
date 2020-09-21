#include "sspf.h"

#include "pch.h"


#define SIZE        16

class SVEffect5 : public SVEffect
{
public:
    SVEffect5(char const *tag, int o);
    virtual         ~SVEffect5();
    virtual int     Init();
    virtual void    GenerateTrans();
    virtual void    GenerateFrame(int frame, int pass, D3DXVECTOR4 **transforms, unsigned int *numTrans, unsigned int *startIndex, unsigned int *numTri);

private:
    int m_used[SIZE][SIZE];
    unsigned char m_coords[SIZE * SIZE][2];
    int m_permute[SIZE * SIZE];
};

dec(SVEffect5, sveffect5_0, 0);
dec(SVEffect5, sveffect5_1, 1);

SVEffect5::SVEffect5(char const *tag, int o)
{
    m_order = o;
    m_useframe = 0;
    m_passes = 8;
}

SVEffect5::~SVEffect5()
{
}

int     SVEffect5::Init()
{
    memset(m_used, 0xff, SIZE * SIZE * sizeof(int));
    int dx = 1;
    int dy = 0;
    int v = 0;
    int x = 0;
    int y = 0;
    for (int v = 0; v < SIZE * SIZE; v++)
    {
        m_used[y][x] = SIZE * SIZE - 1 - v;
        m_coords[SIZE * SIZE - 1 - v][0] = x;
        m_coords[SIZE * SIZE - 1 - v][1] = y;
        m_permute[v] = y * SIZE + x;
        int x1 = x + dx;
        int y1 = y + dy;
        if (dx == 1 && (x1 > SIZE - 1 || m_used[y1][x1] != -1))
        {
            dx = 0;
            dy = 1;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dy == 1 && (y1 > SIZE - 1 || m_used[y1][x1] != -1))
        {
            dx = -1;
            dy = 0;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dx == -1 && (x1 < 0 || m_used[y1][x1] != -1))
        {
            dx = 0;
            dy = -1;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dy == -1 && (y1 < 0 || m_used[y1][x1] != -1))
        {
            dx = 1;
            dy = 0;
            x1 = x + dx;
            y1 = y + dy;
        }
        if (dx == 1 && (x1 > SIZE - 1 || m_used[y1][x1] != -1))
        {
            break;
        }
        x = x1;
        y = y1;
    }

    static int vdata[4][2] =
    {
        { 0, 1 },
        { 1, 1 },
        { 0, 0 },
        { 1, 0 },
    };

    static Vertex verts[4 * 256];

    m_vertices = verts;
    int m = 0;
    int o = 0;
    float const uvScale = 2.0f / (float) SIZE;
    float const stScale = 1.0f / (float) SIZE;
    for (int y = 0; y < SIZE; y++)
    {
        for (int x = 0; x < SIZE; x++)
        {
            float const u0 = -1.0f + (float) x * uvScale;
            float const u1 = u0 + uvScale;
            float const v0 = -1.0f + (float) y * uvScale;
            float const v1 = v0 + uvScale;
            float const s0 = 0.0f + (float) x * stScale;
            float const s1 = s0 + stScale;
            float const t0 = 1.0f - (float) y * stScale;
            float const t1 = t0 - stScale;
            float const fm = (float) m++;
            for (int r = 0; r < 4; r++)
            {
                verts[o].x = vdata[r][0] ? u1 : u0;
                verts[o].y = vdata[r][1] ? v1 : v0;
                verts[o].z = 0.0f;
                verts[o].w = fm;
                verts[o].tu = vdata[r][0] ? s1 : s0;
                verts[o].tv = vdata[r][1] ? t1 : t0;
                o++;
            }
        }
        if (m == 32)
        {
            m = 0;
        }
    }

    m_numVerts = 4 * SIZE * SIZE;

    static unsigned short idata[6] = { 0, 1, 2, 1, 2, 3, };
    static unsigned short inds[6 * 256];

    m_indices = inds;
    for (int i = 0; i < SIZE * SIZE; i++)
    {
        int const o = i * 4;
        int const t = i * 6;
        for (int j = 0; j < 6; j++)
        {
            inds[t + j] = idata[j] + o;
        }
    }

    m_numIndex = 6 * SIZE * SIZE;

    return OK;
}

void    SVEffect5::GenerateTrans()
{
}

void    SVEffect5::GenerateFrame(int frame, int pass, D3DXVECTOR4 **transforms, unsigned int *numTrans, unsigned int *startIndex, unsigned int *numTri)
{
    frame = (256 - frame) / 1;
    if (pass == 0)
    {
        memset(s_transforms, 0, MAX_PASS * MAX_TRANSFORM * sizeof(AATrans));
        for (int pp = 0; pp < MAX_PASS; pp++)
        {
            for (int tt = 0; tt < MAX_TRANSFORM; tt++)
            {
                s_transforms[pp][tt].translation.x = 20.0f;
                s_transforms[pp][tt].translation.y = 20.0f;
            }
        }
        float const transformScale = 2.0f / (float) SIZE;
        for (int ii = 0; ii < frame; ii++)
        {
            int const x = m_coords[frame - 1 - ii][0];
            int const y = m_coords[frame - 1 - ii][1];
            int const xx = m_coords[SIZE * SIZE - 1 - ii][0];
            int const yy = m_coords[SIZE * SIZE - 1 - ii][1];
            int const t = m_permute[SIZE * SIZE - 1 - m_used[y][x]];
            int const xxxx = t % SIZE;
            int const yyyy = t / SIZE;
            int const xxx = t % 32;
            int const yyy = t / 32;
            s_transforms[yyy][xxx].translation.x = (float) (xx - xxxx) * transformScale;
            s_transforms[yyy][xxx].translation.y = (float) (yy - yyyy) * transformScale;
        }
    }

    if (transforms != NULL)
    {
        *transforms = (D3DXVECTOR4 *) s_transforms[pass];
    }
    if (numTrans != NULL)
    {
        *numTrans = 32;
    }
    if (startIndex != NULL)
    {
        *startIndex = 32 * 2 * 3 * pass;
    }
    if (numTri != NULL)
    {
        *numTri = 32 * 2;
    }
}
