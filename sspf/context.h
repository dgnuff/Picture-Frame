// This is a graphics context that can be rendered to, the code to start and stop the thread
// that actually renders, callbacks to handle texture load and effect selection, plus all the
// necessary setup to get the context in a state where we can actually render to it.
#pragma once

#include "sspf.h"

class PEffect;
class SVEffect;
class DVEffect;

// If the screen and image aspect ratios are within this epsilon value of each other, don't bother
// with the letterbox clipping code
#define     AR_EPSILON                  0.01f

#define     LOAD_STATE_STARTFETCH       0x0000
#define     LOAD_STATE_FETCHING         0x0001
#define     LOAD_STATE_RENDERING        0x0002
#define     LOAD_STATE_TRANSITIONING    0x0003
#define     LOAD_STATE_STARTBLACK       0x0004
#define     LOAD_STATE_SHOWBLACK        0x0005
#define     LOAD_STATE_PREPARE          0x0100
#define     LOAD_STATE_TRIGGERED        0x0200
#define     LOAD_STATE_TERMINATE        0x0400
#define     LOAD_STATE_NEWTEXTURE       0x0800
#define     LOAD_STATE_FLAGS            0xff00

class Context final
{
public:
    Context(HWND hWnd, uint32 scrx, uint32 scry, bool fullScreen);
    ~Context();

    static void     InitCallbacks();

    bool    Init();
    bool    InitDevice();
    bool    InitBuffers();
    bool    InitRenderState();
    bool    InitTransform();
    bool    InitTextures();
    bool    InitShaders();

    void    Cleanup();

    void    StartPrepare();
    void    StartTransition();

    static void     SetEffectCallback(Context &context, PEffect *peffect, VEffect *veffect);
    void    SetEffect(PEffect *peffect, VEffect *veffect);
    void    Prepare();

    static void     TextureLoadCallback(Context &context, Texture &texture);
    void    TextureLoad(Texture &texture);
    void    CreateNewTexture();

    void    Render();
    void    RenderPEffect(PEffect const &peffect);
    void    RenderVEffect(VEffect const &veffect);
    void    RenderRectangle(ID3D11ShaderResourceView *textureRV, XMFLOAT4 &stFixup);
    void    RenderMotion(VEffect const &veffect, ID3D11ShaderResourceView *textureRV, XMFLOAT4 &stFixup, bool pass2);

    static DWORD WINAPI RenderThreadWrapper(LPVOID vContext);
    void    RenderThread();

    bool    StartThread();
    void    StopThread();

    static void     SetFrameRate(float frameRate);
    static float    GetFrameRate();

private:
    HWND                        m_hWnd;
    D3D_DRIVER_TYPE             m_driverType;
    D3D_FEATURE_LEVEL           m_featureLevel;
    ID3D11Device                *m_d3dDevice;
    ID3D11DeviceContext         *m_deviceContext;
    IDXGISwapChain              *m_swapChain;
    ID3D11RenderTargetView      *m_renderTargetView;

    ID3D11Texture2D             *m_depthStencil;
    ID3D11DepthStencilView      *m_depthStencilView;
    ID3D11VertexShader          *m_vertexShader;
    ID3D11InputLayout           *m_vertexLayout;
    ID3D11PixelShader           *m_pixelShader;

    ID3D11Buffer                *m_vertexBuffer;
    ID3D11Buffer                *m_identityVertexBuffer;
    ID3D11Buffer                *m_indexBuffer;
    ID3D11Buffer                *m_identityIndexBuffer;
    ID3D11Buffer                *m_vertexBuffer_p2;
    ID3D11Buffer                *m_indexBuffer_p2;
    ID3D11Buffer                *m_matrixBuffer;
    ID3D11Buffer                *m_framesBuffer;
    ID3D11Buffer                *m_depthBiasBuffer;
    ID3D11Buffer                *m_transformsBuffer;
    ID3D11Buffer                *m_identityTransformBuffer;
    ID3D11Buffer                *m_psParamsBuffer;

    Matrices                    m_matrices;
    DepthBias                   m_depthBias;
    Transforms                  m_transforms;
    PSParams                    m_psParams;

    ID3D11Texture2D             *m_oldTexture;
    ID3D11Texture2D             *m_newTexture;
    ID3D11Texture2D             *m_controlTexture;
    ID3D11Texture2D             *m_alphaTexture;
    ID3D11Texture2D             *m_blackControlTexture;
    ID3D11Texture2D             *m_whiteAlphaTexture;

    ID3D11ShaderResourceView    *m_oldTextureRV;
    ID3D11ShaderResourceView    *m_newTextureRV;
    ID3D11ShaderResourceView    *m_controlTextureRV;
    ID3D11ShaderResourceView    *m_alphaTextureRV;
    ID3D11ShaderResourceView    *m_blackControlTextureRV;
    ID3D11ShaderResourceView    *m_whiteAlphaTextureRV;

    ID3D11SamplerState          *m_linearSampler;
    ID3D11SamplerState          *m_pointSampler;

    ID3D11RasterizerState       *m_rasterizerStateNormal;
    ID3D11RasterizerState       *m_rasterizerStateScissors;

    ID3D11BlendState            *m_blendStateNormal;
    ID3D11BlendState            *m_blendStateAlpha;

    XMFLOAT4                    m_oldSTFixup;
    XMFLOAT4                    m_newSTFixup;
    XMFLOAT4                    m_controlSTFixup;

    PEffect                     *m_peffect;
    VEffect                     *m_veffect;

    uint32                      m_scrx;
    uint32                      m_scry;
    uint32                      m_aspect;
    bool                        m_fullScreen;
    bool                        m_firstTime;

    uint32                      m_ctrlTexSize;
    uint8                       *m_ctrlTexData;
    uint8                       **m_ctrlTexPtr;
    uint8                       *m_alphaTexData;
    uint8                       **m_alphaTexPtr;

    float                       m_leadFrameNum;
    float                       m_frameNum;
    int32                       m_mouseDist;

    Texture                     m_textureParams;

    LONG                        m_loadState;

    HANDLE                      m_hThread;

    static XMFLOAT4 const       s_identityTransform[3];
    static float                s_frameRate;
    static D3D11_INPUT_ELEMENT_DESC const   s_vertexLayout[3];
    static float const          s_colorBlack[4];
    static uint32 const         s_blackData[16];
    static uint32 const         s_whiteData[16];

    void                        CreateControlTexture();
    void                        CreateAlphaTexture();
    void                        CreateBlackControlTexture();

    void                        CreateWhiteAlphaTexture();
    void                        CreatMonochromeTexture(ID3D11Texture2D **texture, ID3D11ShaderResourceView **textureRV, void const *data, uint32 w, uint32 h);
    void                        PatternFill();

public:
#ifdef GRAPHICS_DEBUG
    IDXGraphicsAnalysis*        m_pGraphicsAnalysis;
#endif
};
