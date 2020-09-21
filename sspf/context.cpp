#include "pch.h"

#include "sspf.h"
#include "peffect.h"
#include "veffect.h"
#include "loader.h"
#include "util.h"
#include "context.h"

using std::atomic;
using std::max;
using std::swap;

#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedAnd)

#ifdef DEBUG
#include "vertexShaderDebug.hxx"
#include "pixelShaderDebug.hxx"
#else
#include "vertexShaderRelease.hxx"
#include "pixelShaderRelease.hxx"
#endif

#define FPS             50
#define FRAME_PERIOD    (1000 / FPS)
#define RERENDER_DELAY  2000

XMFLOAT4 const  Context::s_identityTransform[3] =
{
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f },
};

float           Context::s_frameRate = 1.0f;

D3D11_INPUT_ELEMENT_DESC const  Context::s_vertexLayout[3] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

float const     Context::s_colorBlack[4] =
{
#ifdef DEBUG
    // clear to magenta for debug.
    1.0f, 0.0f, 1.0f, 1.0f
#else
    1.0f, 0.0f, 0.0f, 1.0f
#endif
};

uint32 const    Context::s_blackData[16] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

uint32 const    Context::s_whiteData[16] =
{
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
};

Context::Context(HWND hWnd, uint32 scrx, uint32 scry, bool fullScreen) :
m_hWnd(hWnd),
m_driverType(D3D_DRIVER_TYPE_NULL),
m_featureLevel(D3D_FEATURE_LEVEL_11_0),
m_d3dDevice(nullptr),
m_deviceContext(nullptr),
m_swapChain(nullptr),
m_renderTargetView(nullptr),
m_depthStencil(nullptr),
m_depthStencilView(nullptr),
m_vertexShader(nullptr),
m_vertexLayout(nullptr),
m_pixelShader(nullptr),
m_vertexBuffer(nullptr),
m_identityVertexBuffer(nullptr),
m_indexBuffer(nullptr),
m_identityIndexBuffer(nullptr),
m_vertexBuffer_p2(nullptr),
m_indexBuffer_p2(nullptr),
m_matrixBuffer(nullptr),
m_framesBuffer(nullptr),
m_depthBiasBuffer(nullptr),
m_transformsBuffer(nullptr),
m_identityTransformBuffer(nullptr),
m_psParamsBuffer(nullptr),
m_oldTexture(nullptr),
m_newTexture(nullptr),
m_controlTexture(nullptr),
m_alphaTexture(nullptr),
m_blackControlTexture(nullptr),
m_whiteAlphaTexture(nullptr),
m_oldTextureRV(nullptr),
m_newTextureRV(nullptr),
m_controlTextureRV(nullptr),
m_alphaTextureRV(nullptr),
m_blackControlTextureRV(nullptr),
m_whiteAlphaTextureRV(nullptr),
m_linearSampler(nullptr),
m_pointSampler(nullptr),
m_rasterizerStateNormal(nullptr),
m_rasterizerStateScissors(nullptr),
m_blendStateNormal(nullptr),
m_blendStateAlpha(nullptr),
m_peffect(nullptr),
m_veffect(nullptr),
m_scrx(scrx),
m_scry(scry),
m_aspect(Util::Aspect(scrx, scry)),
m_fullScreen(fullScreen),
m_firstTime(false),
m_ctrlTexSize(0),
m_ctrlTexData(nullptr),
m_ctrlTexPtr(nullptr),
m_leadFrameNum(0.0f),
m_frameNum(0.0f),
m_mouseDist(1),
m_hThread(nullptr)
#ifdef GRAPHICS_DEBUG
,
m_pGraphicsAnalysis(nullptr)
#endif
{
    InterlockedExchange(&m_loadState, 0);
    memset(&m_oldSTFixup, 0, sizeof(XMFLOAT4));
    memset(&m_newSTFixup, 0, sizeof(XMFLOAT4));
    memset(&m_controlSTFixup, 0, sizeof(XMFLOAT4));

    m_ctrlTexSize = max(scrx, scry);
    // Find next highest power of two courtesy of https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    m_ctrlTexSize--;
    m_ctrlTexSize |= m_ctrlTexSize >> 1;
    m_ctrlTexSize |= m_ctrlTexSize >> 2;
    m_ctrlTexSize |= m_ctrlTexSize >> 4;
    m_ctrlTexSize |= m_ctrlTexSize >> 8;
    m_ctrlTexSize |= m_ctrlTexSize >> 16;
    m_ctrlTexSize++;

    m_ctrlTexData = (uint8 *) malloc(m_ctrlTexSize * m_ctrlTexSize + m_ctrlTexSize * sizeof(uint8 *));
    m_alphaTexData = (uint8 *) malloc(m_ctrlTexSize * m_ctrlTexSize * sizeof(uint8) + m_ctrlTexSize * sizeof(uint8 *));
    if (m_ctrlTexData != nullptr && m_alphaTexData != nullptr)
    {
        m_ctrlTexPtr = (uint8 **)&m_ctrlTexData[m_ctrlTexSize * m_ctrlTexSize];
        m_alphaTexPtr = (uint8 **)&m_alphaTexData[m_ctrlTexSize * m_ctrlTexSize];

        for (uint32 i = 0; i < m_ctrlTexSize; i++)
        {
            m_ctrlTexPtr[i] = &m_ctrlTexData[i * m_ctrlTexSize];
            m_alphaTexPtr[i] = &m_alphaTexData[i * m_ctrlTexSize];
        }
    }
#ifdef GRAPHICS_DEBUG
    HRESULT const getAnalysis = DXGIGetDebugInterface1(0, __uuidof(m_pGraphicsAnalysis), reinterpret_cast<void**>(&m_pGraphicsAnalysis));
    if (FAILED(getAnalysis))
    {
        m_pGraphicsAnalysis = nullptr;
    }
#endif
}

Context::~Context()
{
    free(m_ctrlTexData);
    free(m_alphaTexData);
}

void    Context::InitCallbacks()
{
    Loader::SetCallbacks(SetEffectCallback, TextureLoadCallback);
}

bool    Context::Init()
{
    if (m_ctrlTexData == nullptr || m_alphaTexData == nullptr)
    {
        return false;
    }
    if (!InitDevice())
    {
        return false;
    }
    if (!InitBuffers())
    {
        return false;
    }
    if (!InitRenderState())
    {
        return false;
    }
    if (!InitTransform())
    {
        return false;
    }
    if (!InitTextures())
    {
        return false;
    }
    if (!InitShaders())
    {
        return false;
    }
    return true;
}

bool    Context::InitDevice()
{
    HRESULT hr = S_OK;

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = NUM_ELEMENT(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = NUM_ELEMENT(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(NULL, m_driverType, NULL, 0, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_deviceContext);

        if (hr == E_INVALIDARG)
        {
            return false;
        }

        if (SUCCEEDED(hr))
        {
            break;
        }
    }
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIFactory1* dxgiFactory = NULL;
    IDXGIDevice* dxgiDevice = NULL;
    hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **) &dxgiDevice);
    if (SUCCEEDED(hr))
    {
        IDXGIAdapter* adapter = NULL;
        hr = dxgiDevice->GetAdapter(&adapter);
        if (SUCCEEDED(hr))
        {
            hr = adapter->GetParent(__uuidof(IDXGIFactory1), (void **) &dxgiFactory);
            adapter->Release();
        }
        dxgiDevice->Release();
    }
    if (FAILED(hr))
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_scrx;
    sd.BufferDesc.Height = m_scry;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(m_d3dDevice, &sd, &m_swapChain);

    // Note this program doesn't handle, or need to handle, full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
    {
        return false;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &pBackBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        return false;
    }

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    memset(&descDepth, 0, sizeof(descDepth));
    descDepth.Width = m_scrx;
    descDepth.Height = m_scry;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_d3dDevice->CreateTexture2D(&descDepth, NULL, &m_depthStencil);
    if (FAILED(hr))
    {
        return false;
    }

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
    if (FAILED(hr))
    {
        return false;
    }

    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    return true;
}

bool    Context::InitBuffers()
{
    D3D11_BUFFER_DESC bd;
    memset(&bd, 0, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    bd.ByteWidth = sizeof(Vertex) * (MAX_VERTEX + 1);
    HRESULT hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_vertexBuffer);
    if (FAILED(hr))
    {
        return false;
    }
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_vertexBuffer_p2);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(Vertex) * 4;
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_identityVertexBuffer);
    if (FAILED(hr))
    {
        return false;
    }
    m_deviceContext->UpdateSubresource(m_identityVertexBuffer, 0, NULL, VEffect::GetVertices(), 0, 0);

    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    bd.ByteWidth = sizeof(uint16) * MAX_INDEX;
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_indexBuffer);
    if (FAILED(hr))
    {
        return false;
    }
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_indexBuffer_p2);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(uint16) * 6;
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_identityIndexBuffer);
    if (FAILED(hr))
    {
        return false;
    }
    m_deviceContext->UpdateSubresource(m_identityIndexBuffer, 0, NULL, VEffect::GetIndices(), 0, 0);

    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    bd.ByteWidth = sizeof(Matrices);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_matrixBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(Frames);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_framesBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(DepthBias);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_depthBiasBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(Transforms);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_transformsBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    bd.ByteWidth = sizeof(AATrans);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_identityTransformBuffer);
    if (FAILED(hr))
    {
        return false;
    }
    m_deviceContext->UpdateSubresource(m_identityTransformBuffer, 0, NULL, s_identityTransform, 0, 0);

    bd.ByteWidth = sizeof(PSParams);
    hr = m_d3dDevice->CreateBuffer(&bd, NULL, &m_psParamsBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool    Context::InitRenderState()
{
    HRESULT hr;

    D3D11_SAMPLER_DESC sampDesc;
    memset(&sampDesc, 0, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_d3dDevice->CreateSamplerState(&sampDesc, &m_linearSampler);
    if (FAILED(hr))
    {
        return false;
    }

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = m_d3dDevice->CreateSamplerState(&sampDesc, &m_pointSampler);
    if (FAILED(hr))
    {
        return false;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc;
    memset(&rasterizerDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    hr = m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStateNormal);
    if (FAILED(hr))
    {
        return false;
    }

    rasterizerDesc.ScissorEnable = TRUE;

    hr = m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStateScissors);
    if (FAILED(hr))
    {
        return false;
    }

    m_deviceContext->RSSetState(m_rasterizerStateNormal);

    D3D11_BLEND_DESC blendDesc;
    memset(&blendDesc, 0, sizeof(D3D11_BLEND_DESC));

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_d3dDevice->CreateBlendState(&blendDesc, &m_blendStateNormal);
    if (FAILED(hr))
    {
        return false;
    }

    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_d3dDevice->CreateBlendState(&blendDesc, &m_blendStateAlpha);
    if (FAILED(hr))
    {
        return false;
    }

    m_deviceContext->OMSetBlendState(m_blendStateNormal, NULL, 0xffffffff);

    return true;
}

bool    Context::InitTransform()
{
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(15.0f), (float) m_scrx / (float) m_scry, 1.0f, 100.0f);
    projection.r[0].m128_f32[0] = 4.0f;
    projection.r[1].m128_f32[1] = 4.0f * (float) m_scrx / (float) m_scry;

    // This has the same net result as doing a lookat, with the eye at projection.r[0].m128_f32[0], target at the origin,
    // and up going up the y axis.  By using the top left value from the projection matrix as the eye distance, this has
    // the very convenient effect of exactly filling the viewport with a sqare rendered from -1,-1 to 1,1.  Which in turn means
    // that an identity world matrix places our standard output square in the correct place to fill the viewport.

#ifdef DEBUG
    // For debug, we pull the camera back so that the actual area drawn is smaller than the viewport.  And the reason we multiply
    // the view distance by 4/3 is that it will make the output rectangle fill 3/4 of the linear dimensions of the viewport.  This
    // also requires that we adjust scissors coordinates, see some code in VEffect::ScissorPoint() that adjusts the coordinates
    // by the inverse amount, i.e. 3.0f / 4.0f.
    XMMATRIX const view = XMMatrixTranslation(0.0f, 0.0f, VEffect::s_borderRect ? 4.0f * (4.0f / 3.0f) : 4.0f);
#else
    XMMATRIX const view = XMMatrixTranslation(0.0f, 0.0f, 4.0f);
#endif

    XMMATRIX const vpm = view * projection;
    m_matrices.viewProjectionMatrix = XMMatrixTranspose(vpm);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT) m_scrx;
    vp.Height = (FLOAT) m_scry;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_deviceContext->RSSetViewports(1, &vp);

    return true;
}

template <typename T> void  SafeRelease(T *&object)
{
    if (object != nullptr)
    {
        object->Release();
        object = nullptr;
    }
}

bool    Context::InitTextures()
{
    // Create a black control texture.
    CreateBlackControlTexture();
    SafeRelease(m_oldTexture);
    SafeRelease(m_oldTextureRV);
    swap(m_oldTexture, m_controlTexture);
    swap(m_oldTextureRV, m_controlTextureRV);
    m_oldSTFixup = XMFLOAT4{ 1.0f, 0.0f, 1.0f, 0.0f };
    InterlockedExchange(&m_loadState, LOAD_STATE_STARTFETCH);
    m_firstTime = true;

    CreateBlackControlTexture();
    CreateWhiteAlphaTexture();

    return true;
}

bool    Context::InitShaders()
{
    HRESULT hr = m_d3dDevice->CreateVertexShader(vertexShader, sizeof(vertexShader), NULL, &m_vertexShader);
    if (FAILED(hr))
    {
        return false;
    }

    UINT const vertexLayoutSize = ARRAYSIZE(s_vertexLayout);
    hr = m_d3dDevice->CreateInputLayout(s_vertexLayout, vertexLayoutSize, vertexShader, sizeof(vertexShader), &m_vertexLayout);
    if (FAILED(hr))
    {
        return false;
    }

    m_deviceContext->IASetInputLayout(m_vertexLayout);

    hr = m_d3dDevice->CreatePixelShader(pixelShader, sizeof(pixelShader), NULL , &m_pixelShader);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void    Context::Cleanup()
{
    SafeRelease(m_blendStateAlpha);
    SafeRelease(m_blendStateNormal);
    SafeRelease(m_rasterizerStateNormal);
    SafeRelease(m_rasterizerStateScissors);
    SafeRelease(m_pointSampler);
    SafeRelease(m_linearSampler);
    SafeRelease(m_whiteAlphaTextureRV);
    SafeRelease(m_blackControlTextureRV);
    SafeRelease(m_alphaTextureRV);
    SafeRelease(m_controlTextureRV);
    SafeRelease(m_newTextureRV);
    SafeRelease(m_oldTextureRV);
    SafeRelease(m_whiteAlphaTexture);
    SafeRelease(m_blackControlTexture);
    SafeRelease(m_alphaTexture);
    SafeRelease(m_controlTexture);
    SafeRelease(m_newTexture);
    SafeRelease(m_oldTexture);
    SafeRelease(m_psParamsBuffer);
    SafeRelease(m_identityTransformBuffer);
    SafeRelease(m_transformsBuffer);
    SafeRelease(m_depthBiasBuffer);
    SafeRelease(m_framesBuffer);
    SafeRelease(m_matrixBuffer);
    SafeRelease(m_indexBuffer_p2);
    SafeRelease(m_vertexBuffer_p2);
    SafeRelease(m_identityIndexBuffer);
    SafeRelease(m_indexBuffer);
    SafeRelease(m_identityVertexBuffer);
    SafeRelease(m_vertexBuffer);
    SafeRelease(m_pixelShader);
    SafeRelease(m_vertexLayout);
    SafeRelease(m_vertexShader);
    SafeRelease(m_depthStencilView);
    SafeRelease(m_depthStencil);
    SafeRelease(m_renderTargetView);
    SafeRelease(m_swapChain);
    SafeRelease(m_deviceContext);
    SafeRelease(m_d3dDevice);
#ifdef GRAPHICS_DEBUG
    SafeRelease(m_pGraphicsAnalysis);
#endif
}

void    Context::StartPrepare()
{
    InterlockedOr(&m_loadState, LOAD_STATE_PREPARE);
}

void    Context::StartTransition()
{
    InterlockedOr(&m_loadState, LOAD_STATE_TRIGGERED);
}

void    Context::SetEffectCallback(Context &context, PEffect *peffect, VEffect *veffect)
{
    context.SetEffect(peffect, veffect);
}

void    Context::SetEffect(PEffect *peffect, VEffect *veffect)
{
    m_peffect = peffect;
    m_veffect = veffect;

    char const *tag = peffect != NULL ? peffect->GetTag() : veffect != NULL ? veffect->GetTag() : NULL;

    if (tag != NULL)
    {
        logFile.Report("Context::SetEffect() called, effect: %s", tag);
    }
}

void    Context::Prepare()
{
    if (m_peffect != NULL)
    {
        PatternFill();

        m_peffect->PreGenerate();
        if (m_peffect->GetSquare())
        {
            m_controlSTFixup.x = 1.0f;
            m_controlSTFixup.z = 1.0f;
            m_peffect->Generate(m_ctrlTexPtr, m_ctrlTexSize, m_ctrlTexSize);
        }
        else
        {
            m_controlSTFixup.x = (float) m_scrx / (float) m_ctrlTexSize;
            m_controlSTFixup.z = (float) m_scry / (float) m_ctrlTexSize;
            m_peffect->Generate(m_ctrlTexPtr, m_scrx, m_scry);
        }

        CreateControlTexture();
    }
    else
    {
        m_veffect->GenerateTrans((AATrans *) m_transforms.transforms, m_scrx, m_scry);
        memset(&m_transforms.transforms[MAX_TRANSFORM * 3], 0, sizeof(AATrans));

        Vertex vertices[MAX_VERTEX];
        uint16 indices[MAX_INDEX];
        m_veffect->GetVertexData(vertices, indices);
        m_deviceContext->UpdateSubresource(m_vertexBuffer, 0, NULL, vertices, 0, 0);
        m_deviceContext->UpdateSubresource(m_indexBuffer, 0, NULL, indices, 0, 0);

        m_veffect->GetVertexData_p2(vertices, indices);
        m_deviceContext->UpdateSubresource(m_vertexBuffer_p2, 0, NULL, vertices, 0, 0);
        m_deviceContext->UpdateSubresource(m_indexBuffer_p2, 0, NULL, indices, 0, 0);

        if (m_veffect->GenerateAlpha(m_alphaTexPtr, m_ctrlTexSize, m_ctrlTexSize))
        {
            CreateAlphaTexture();
        }
    }
}

void    Context::TextureLoadCallback(Context &context, Texture &texture)
{
    context.TextureLoad(texture);
}

void    Context::TextureLoad(Texture &texture)
{
    m_textureParams = texture;
    InterlockedOr(&m_loadState, LOAD_STATE_NEWTEXTURE);
}

void    Context::CreateNewTexture()
{
    SafeRelease(m_newTexture);
    SafeRelease(m_newTextureRV);

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = m_textureParams.m_tx;
    textureDesc.Height = m_textureParams.m_ty;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = m_textureParams.m_data;
    subresourceData.SysMemPitch = m_textureParams.m_tx * 4;
    subresourceData.SysMemSlicePitch = 0;

    HRESULT hr;
    hr = m_d3dDevice->CreateTexture2D(&textureDesc, &subresourceData, &m_newTexture);
    hr = m_d3dDevice->CreateShaderResourceView(m_newTexture, NULL, &m_newTextureRV);

    char const *fileName = m_textureParams.m_resource != nullptr ? m_textureParams.m_resource->m_filename.c_str() : "unknown";
    if (prefixi(fileName, "file://"))
    {
        fileName = &fileName[7];
    }
    char const * const sl = strrchr(fileName, '/');
    char const * const bs = strrchr(fileName, '\\');
    char const * const co = strrchr(fileName, ':');

    auto compare = [](char const * const &a, char const * const &b)
    {
        return b == NULL ? false : a == NULL ? true : a < b;
    };
    char const * const delim = max({ sl, bs, co }, compare);

    if (delim != nullptr)
    {
        fileName = &delim[1];
    }

    logFile.Report("Context::CreateNewTexture() creating texture from file: %s, buffer: %p, size: %d, original size: %d", fileName, m_textureParams.m_data, m_textureParams.m_size, m_textureParams.m_originalSize);

    float const texAspectRatio = (float) m_textureParams.m_w / (float) m_textureParams.m_h;
    float const scrAspectRatio = (float) m_scrx / (float) m_scry;

    // If the texture and screen aspect ratios don't match, fiddle with the x, y, w, h values
    // to pull the largest rectangle we can from the center of the available rectangle
    if (texAspectRatio > scrAspectRatio + AR_EPSILON)
    {
        uint32 const i = (uint32) ((float) m_textureParams.m_h * scrAspectRatio);
        m_textureParams.m_x += (m_textureParams.m_w - i) / 2;
        m_textureParams.m_w = i;
    }
    else if (texAspectRatio < scrAspectRatio - AR_EPSILON)
    {
        uint32 const i = (uint32) ((float) m_textureParams.m_w / scrAspectRatio);
        m_textureParams.m_y += (m_textureParams.m_h - i) / 2;
        m_textureParams.m_h = i;
    }

    m_newSTFixup.x = (float) m_textureParams.m_w / (float) m_textureParams.m_tx;
    m_newSTFixup.y = (float) m_textureParams.m_x / (float) m_textureParams.m_tx;
    m_newSTFixup.z = (float) m_textureParams.m_h / (float) m_textureParams.m_ty;
    m_newSTFixup.w = (float) m_textureParams.m_y / (float) m_textureParams.m_ty;

    if (m_firstTime)
    {
        PostMessage(m_hWnd, WM_FIRSTLOAD, 0, 0);
        m_firstTime = false;
    }
    InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS);
    InterlockedOr(&m_loadState, LOAD_STATE_RENDERING);
}

void    Context::Render()
{
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, s_colorBlack);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    RenderRectangle(m_oldTextureRV, m_oldSTFixup);

    m_swapChain->Present(0, 0);
}

void    Context::RenderPEffect(PEffect const &peffect)
{
#ifdef GRAPHICS_DEBUG
    if (m_pGraphicsAnalysis != NULL && (m_frameNum >= 254.0f || m_frameNum <= 2.0f))
    {
        m_pGraphicsAnalysis->BeginCapture();
    }
#endif

    m_deviceContext->ClearRenderTargetView(m_renderTargetView, s_colorBlack);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_deviceContext->UpdateSubresource(m_matrixBuffer, 0, NULL, &m_matrices, 0, 0);

    Frames frames;
    frames.frames[0].x = 0.0f;
    m_deviceContext->UpdateSubresource(m_framesBuffer, 0, NULL, &frames, 0, 0);

    m_depthBias.depthBias = XMFLOAT4{ 0.0f, 0.0f, 0.5f, 0.0f };
    m_depthBias.depthBias.w = (float) m_scry / (float) m_scrx;
    m_deviceContext->UpdateSubresource(m_depthBiasBuffer, 0, NULL, &m_depthBias, 0, 0);

    m_psParams.oldSTFixup = m_oldSTFixup;
    m_psParams.newSTFixup = m_newSTFixup;
    m_psParams.ctrlSTFixup = m_controlSTFixup;

    float const frameNum = m_frameNum / 256.0f;
    m_psParams.frame = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };
    m_psParams.frame.x = frameNum;

    XMFLOAT4 const * const control = peffect.GetControl();
    m_psParams.control = *control;

    m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, &m_identityVertexBuffer, &stride, &offset);

    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_deviceContext->IASetIndexBuffer(m_identityIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
    m_deviceContext->VSSetConstantBuffers(1, 1, &m_framesBuffer);
    m_deviceContext->VSSetConstantBuffers(2, 1, &m_depthBiasBuffer);
    m_deviceContext->VSSetConstantBuffers(3, 1, &m_identityTransformBuffer);
    m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    m_psParams.frame.y = 1.0f;
    m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);
    m_deviceContext->PSSetConstantBuffers(4, 1, &m_psParamsBuffer);
    m_deviceContext->PSSetShaderResources(0, 1, &m_oldTextureRV);
    m_deviceContext->PSSetShaderResources(1, 1, &m_newTextureRV);
    m_deviceContext->PSSetShaderResources(2, 1, &m_controlTextureRV);
    m_deviceContext->PSSetShaderResources(3, 1, &m_whiteAlphaTextureRV);
    m_deviceContext->PSSetSamplers(0, 1, &m_linearSampler);
    m_deviceContext->PSSetSamplers(1, 1, &m_pointSampler);
    m_deviceContext->DrawIndexed(6, 0, 0);

    m_swapChain->Present(0, 0);

    return;
}

void    Context::RenderVEffect(VEffect const &veffect)
{
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, s_colorBlack);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    ID3D11ShaderResourceView *textureRV = veffect.GetOrder() ? m_oldTextureRV : m_newTextureRV;
    XMFLOAT4 stFixup = veffect.GetOrder() ? m_oldSTFixup : m_newSTFixup;

    if (veffect.GetUsages() & IS_DOUBLE)
    {
        RenderMotion(veffect, textureRV, stFixup, true);
    }
    else
    {
        RenderRectangle(textureRV, stFixup);
    }

    textureRV = veffect.GetOrder() ? m_newTextureRV : m_oldTextureRV;
    stFixup = veffect.GetOrder() ? m_newSTFixup : m_oldSTFixup;

    RenderMotion(veffect, textureRV, stFixup, false);

    m_swapChain->Present(0, 0);
}

void    Context::RenderRectangle(ID3D11ShaderResourceView *textureRV, XMFLOAT4 &stFixup)
{
    m_deviceContext->UpdateSubresource(m_matrixBuffer, 0, NULL, &m_matrices, 0, 0);

    Frames frames;
    frames.frames[0].x = 0.0f;
    m_deviceContext->UpdateSubresource(m_framesBuffer, 0, NULL, &frames, 0, 0);

    m_depthBias.depthBias = XMFLOAT4{ 0.0f, 0.0f, 0.5f, 0.0f };
    m_depthBias.depthBias.w = (float) m_scry / (float) m_scrx;
    m_deviceContext->UpdateSubresource(m_depthBiasBuffer, 0, NULL, &m_depthBias, 0, 0);

    m_psParams.oldSTFixup = stFixup;
    m_psParams.newSTFixup = stFixup;
    m_psParams.ctrlSTFixup = m_controlSTFixup;
    m_psParams.frame = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };
    m_psParams.control = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };

    m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, &m_identityVertexBuffer, &stride, &offset);

    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_deviceContext->IASetIndexBuffer(m_identityIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
    m_deviceContext->VSSetConstantBuffers(1, 1, &m_framesBuffer);
    m_deviceContext->VSSetConstantBuffers(2, 1, &m_depthBiasBuffer);
    m_deviceContext->VSSetConstantBuffers(3, 1, &m_identityTransformBuffer);
    m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    m_psParams.frame.y = 1.0f;
    m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);
    m_deviceContext->PSSetConstantBuffers(4, 1, &m_psParamsBuffer);
    m_deviceContext->PSSetShaderResources(0, 1, &textureRV);
    m_deviceContext->PSSetShaderResources(1, 1, &textureRV);
    m_deviceContext->PSSetShaderResources(2, 1, &m_blackControlTextureRV);
    m_deviceContext->PSSetShaderResources(3, 1, &m_whiteAlphaTextureRV);
    m_deviceContext->PSSetSamplers(0, 1, &m_linearSampler);
    m_deviceContext->PSSetSamplers(1, 1, &m_pointSampler);
    m_deviceContext->DrawIndexed(6, 0, 0);
}

void    Context::RenderMotion(VEffect const &veffect, ID3D11ShaderResourceView *textureRV, XMFLOAT4 &stFixup, bool pass2)
{
    float const frameNum = veffect.GetOrder() ? 256.0f - m_frameNum : m_frameNum;
    Frames frames_p1;
    Frames frames_p2;
    veffect.GenerateFrames(frameNum, (float *) &frames_p1.frames[0].x, (float *) &frames_p2.frames[0].x);
    Frames *frames = pass2 ? &frames_p2 : &frames_p1;
    m_deviceContext->UpdateSubresource(m_framesBuffer, 0, NULL, frames, 0, 0);

    m_depthBias.depthBias = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };
    float const depthBias = 0.1f + (pass2 ? m_veffect->GetDB_p2() : 0.0f);
    m_depthBias.depthBias.z = depthBias;
    m_depthBias.depthBias.w = (float) m_scry / (float) m_scrx;

    m_psParams.oldSTFixup = stFixup;
    m_psParams.newSTFixup = stFixup;

    m_psParams.ctrlSTFixup = XMFLOAT4{ 1.0f, 0.0f, 1.0f, 0.0f };
    m_psParams.frame = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };
    m_psParams.control = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };

    m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);

    ID3D11Buffer * const *vertexBuffer = pass2 ? &m_vertexBuffer_p2 : &m_vertexBuffer;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, vertexBuffer, &stride, &offset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer *indexBuffer = pass2 ? m_indexBuffer_p2 : m_indexBuffer;
    m_deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_deviceContext->UpdateSubresource(m_transformsBuffer, 0, NULL, &m_transforms, 0, 0);

    m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
    m_deviceContext->VSSetConstantBuffers(1, 1, &m_framesBuffer);
    m_deviceContext->VSSetConstantBuffers(3, 1, &m_transformsBuffer);
    m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
    m_deviceContext->PSSetConstantBuffers(4, 1, &m_psParamsBuffer);

    m_deviceContext->PSSetShaderResources(0, 1, &textureRV);
    m_deviceContext->PSSetShaderResources(1, 1, &textureRV);

    m_deviceContext->PSSetShaderResources(2, 1, &m_blackControlTextureRV);
    m_deviceContext->PSSetSamplers(0, 1, &m_linearSampler);
    m_deviceContext->PSSetSamplers(1, 1, &m_pointSampler);

    uint32 const usages = veffect.GetUsages();
    uint32 const scissorsCheck = pass2 ? USE_SCISSORS_P2 : USE_SCISSORS_P1;
    uint32 const stencilCheck = pass2 ? USE_STENCIL_P2 : USE_STENCIL_P1;
    bool const useScissors = (usages & scissorsCheck) != 0;
    bool const useStencil = (usages & stencilCheck) != 0;

    int32 const passes = pass2 ? veffect.GetPasses_p2() : veffect.GetPasses();
    for (int32 pass = 0; pass < passes; pass++)
    {
        uint32 baseTrans = 0;
        uint32 startIndex = 0;
        uint32 numTri = 2;
        uint32 numTri_p2 = 2;
        D3D11_RECT const *scissor = NULL;
        uint8 alpha = 255;

        veffect.GetTransData(pass, frameNum, &baseTrans, &startIndex, &numTri, &numTri_p2, &alpha, &scissor);
        if (pass2)
        {
            numTri = numTri_p2;
        }

        if (scissor != NULL && useScissors)
        {
            m_deviceContext->RSSetScissorRects(1, scissor);
            m_deviceContext->RSSetState(m_rasterizerStateScissors);
        }

        if (useStencil)
        {
            m_psParams.frame.y = (float) alpha / 255.0f;
            m_depthBias.depthBias.z = depthBias - (float) pass * 0.005f;
            m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);
            m_deviceContext->PSSetConstantBuffers(4, 1, &m_psParamsBuffer);
            m_deviceContext->PSSetShaderResources(3, 1, &m_alphaTextureRV);
            m_deviceContext->OMSetBlendState(m_blendStateAlpha, NULL, 0xffffffff);
        }
        else
        {
            m_psParams.frame.y = 1.0f;
            m_deviceContext->UpdateSubresource(m_psParamsBuffer, 0, NULL, &m_psParams, 0, 0);
            m_deviceContext->PSSetConstantBuffers(4, 1, &m_psParamsBuffer);
            m_deviceContext->PSSetShaderResources(3, 1, &m_whiteAlphaTextureRV);
            m_deviceContext->OMSetBlendState(m_blendStateNormal, NULL, 0xffffffff);
        }

        m_depthBias.depthBias.x = (float) baseTrans;
        m_deviceContext->UpdateSubresource(m_depthBiasBuffer, 0, NULL, &m_depthBias, 0, 0);
        m_deviceContext->VSSetConstantBuffers(2, 1, &m_depthBiasBuffer);

        if (numTri > 0)
        {
            m_deviceContext->DrawIndexed(numTri * 3, startIndex, 0);
        }

        if (scissor != NULL && useScissors)
        {
            m_deviceContext->RSSetState(m_rasterizerStateNormal);
        }
    }
}

DWORD WINAPI Context::RenderThreadWrapper(LPVOID vContext)
{
    if (vContext != NULL)
    {
        Context *context = (Context *) vContext;
        context->RenderThread();
    }
    return 0;
}

void    Context::RenderThread()
{
    bool showBlack = false;
    long now = (long) timeGetTime();
    long lastRenderTick = now;
    long reRenderBase = now + 1000000;

    for (;;)
    {
        int32 loadState = (int32) InterlockedAdd(&m_loadState, 0);
        for (;;)
        {
            if (loadState & LOAD_STATE_TERMINATE)
            {
                Cleanup();
                return;
            }
            if (loadState & LOAD_STATE_NEWTEXTURE)
            {
                CreateNewTexture();

                // m_textureParams.data is one of two things.  Either allocated with malloc, and holding a
                // decoded image, or Loader::n_dummyTexture.  In the former case, all alpha values will be
                // 0xff, as returned by the read_???() routines in image.lib.  In the latter case, we drop
                // a special snowflake value of 0 in the first alpha slot.  The pixel shader ignores
                // texture alpha, so this won't affect the render, but we can use that value to determine
                // whether we need to free() the texture data.
                if (m_textureParams.m_data[3] == 0xff)
                {
                    free(m_textureParams.m_data);
                }
                InterlockedAnd(&m_loadState, ~LOAD_STATE_NEWTEXTURE);
            }
            now = (long) timeGetTime();
            if (now - lastRenderTick >= FRAME_PERIOD)
            {
                lastRenderTick = now;
                break;
            }
            int32 const loadValue = loadState & ~LOAD_STATE_FLAGS;
            uint32 sleepTime = loadValue == LOAD_STATE_TRANSITIONING ? 1 : loadValue == LOAD_STATE_SHOWBLACK ? 30000 : 1000;
            Sleep(sleepTime);
            loadState = (int32) InterlockedAdd(&m_loadState, 0);
        }

        switch (loadState & ~LOAD_STATE_FLAGS)
        {
        case LOAD_STATE_STARTFETCH:
            Render();
            showBlack = Util::ShowBlack();
            reRenderBase = now;
            Loader::SubmitJob(this, m_scrx, m_scry, m_firstTime, showBlack);
            InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS);
            InterlockedOr(&m_loadState, LOAD_STATE_FETCHING);
            break;

        case LOAD_STATE_FETCHING:
            now = timeGetTime();
            if (now - reRenderBase >= RERENDER_DELAY)
            {
                Render();
                reRenderBase = now + 1000000;
            }
            break;

        case LOAD_STATE_RENDERING:
            now = timeGetTime();
            if (now - reRenderBase >= RERENDER_DELAY)
            {
                Render();
                reRenderBase = now + 1000000;
            }

            if (loadState & LOAD_STATE_PREPARE)
            {
                Prepare();
                PostMessage(m_hWnd, WM_HEARTBEAT, 0, 0);
                InterlockedAnd(&m_loadState, ~LOAD_STATE_PREPARE);
            }

            if (loadState & LOAD_STATE_TRIGGERED)
            {
                Render();
                m_leadFrameNum = 0.0f;
                if (!m_fullScreen)
                {
                    char const *tag = m_peffect != NULL ? m_peffect->GetTag() : m_veffect != NULL ? m_veffect->GetTag() : "";
                    static char title[256];
                    snprintf(title, 256, "Digital Picture Frame%s%s", tag[0] != 0 ? " - " : "", tag);
                    PostMessage(m_hWnd, WM_NEWTITLE, 0, (LPARAM) title);
                }

                InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS & ~LOAD_STATE_TRIGGERED);
                InterlockedOr(&m_loadState, LOAD_STATE_TRANSITIONING);
            }
            break;

        case LOAD_STATE_TRANSITIONING:
            m_frameNum = max(m_leadFrameNum - 32.0f, 0.0f);
            if (m_peffect != NULL)
            {
                RenderPEffect(*m_peffect);
            }
            else
            {
                RenderVEffect(*m_veffect);
            }
            if (m_leadFrameNum >= 256.0f + 32.0f)
            {
                InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS);
                InterlockedOr(&m_loadState, showBlack ? LOAD_STATE_STARTBLACK : LOAD_STATE_STARTFETCH);

                SafeRelease(m_oldTexture);
                SafeRelease(m_oldTextureRV);
                swap(m_oldTexture, m_newTexture);
                swap(m_oldTextureRV, m_newTextureRV);

                m_oldSTFixup = m_newSTFixup;
                if (!m_fullScreen)
                {
                    PostMessage(m_hWnd, WM_NEWTITLE, 0, (LPARAM) "Digital Picture Frame");
                }
                CreateBlackControlTexture();
                CreateWhiteAlphaTexture();
                PostMessage(m_hWnd, WM_HEARTBEAT, 0, 0);
            }
            m_leadFrameNum += s_frameRate;
            break;

        case LOAD_STATE_STARTBLACK:
            Render();
            reRenderBase = now;
            InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS);
            InterlockedOr(&m_loadState, LOAD_STATE_SHOWBLACK);
            PostMessage(m_hWnd, WM_GONEBLACK, 0, 0);
            break;

        case LOAD_STATE_SHOWBLACK:
            now = timeGetTime();
            if (now - reRenderBase >= RERENDER_DELAY)
            {
                Render();
                reRenderBase = now + 2000000;
            }
            if (!Util::ShowBlack())
            {
                InterlockedAnd(&m_loadState, LOAD_STATE_FLAGS);
                InterlockedOr(&m_loadState, LOAD_STATE_STARTFETCH);
                Util::SetIgnoreMouse(2000);
                PostMessage(m_hWnd, WM_SCREENON, 0, 0);
            }
            break;

        default:
            break;
        }
    }
}

bool    Context::StartThread()
{
    InterlockedExchange(&m_loadState, LOAD_STATE_STARTFETCH);
    m_hThread = CreateThread(NULL, 0, RenderThreadWrapper, (LPVOID) this, 0, NULL);
    if (m_hThread == NULL)
    {
        return false;
    }
    return true;
}

void    Context::StopThread()
{
    // Set the terminate flag.  Since the thread is free running, it will pick this up without any further action on our part.
    InterlockedOr(&m_loadState, LOAD_STATE_TERMINATE);
    // Give the thread 100ms to exit
    DWORD const result = WaitForSingleObject(m_hThread, 100);
    if (result != WAIT_OBJECT_0)
    {
        // Thread did not terminate in a timely manner.  It's marginally safer to suspend it than it is
        // to terminate it.  Stopping it one way or another is definitely desirable since the DirectX
        // reources that it uses are about to be released.  We're shutting down the program at this point,
        // so the thread will be removed when the program exits.
        SuspendThread(m_hThread);
    }
}

void    Context::SetFrameRate(float frameRate)
{
    s_frameRate = frameRate >= 0.1f && frameRate <= 4.0f ? frameRate : 1.0f;
}

float   Context::GetFrameRate()
{
    return s_frameRate;
}

void    Context::CreateControlTexture()
{
    CreatMonochromeTexture(&m_controlTexture, &m_controlTextureRV, m_ctrlTexData, m_ctrlTexSize, m_ctrlTexSize);
}

void    Context::CreateAlphaTexture()
{
    CreatMonochromeTexture(&m_alphaTexture, &m_alphaTextureRV, m_alphaTexData, m_ctrlTexSize, m_ctrlTexSize);
}

void    Context::CreateBlackControlTexture()
{
    CreatMonochromeTexture(&m_blackControlTexture, &m_blackControlTextureRV, s_blackData, 8, 8);
}

void    Context::CreateWhiteAlphaTexture()
{
    CreatMonochromeTexture(&m_whiteAlphaTexture, &m_whiteAlphaTextureRV, s_whiteData, 8, 8);
}

void    Context::CreatMonochromeTexture(ID3D11Texture2D **texture,  ID3D11ShaderResourceView **textureRV, void const *data, uint32 w, uint32 h)
{
    if (*texture != NULL)
    {
        (*texture)->Release();
        *texture = NULL;
    }
    if (*textureRV != NULL)
    {
        (*textureRV)->Release();
        *textureRV = NULL;
    }

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = w;
    textureDesc.Height = h;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;

    subresourceData.pSysMem = data;
    subresourceData.SysMemPitch = w;
    subresourceData.SysMemSlicePitch = 0;

    m_d3dDevice->CreateTexture2D(&textureDesc, &subresourceData, texture);
    m_d3dDevice->CreateShaderResourceView(*texture, NULL, textureRV);
}

void    Context::PatternFill()
{
    for (uint32 y = 0; y < m_ctrlTexSize; y += 8)
    {
        for (uint32 x = 0; x < m_ctrlTexSize; x += 8)
        {
            uint8 const v = (x ^ y) & 8 ? 255 : 0;
            for (uint32 y1 = 0; y1 < 8; y1++)
            {
                for (uint32 x1 = 0; x1 < 8; x1++)
                {
                    m_ctrlTexPtr[y + y1][x + x1] = v;
                }
            }
        }
    }
}
