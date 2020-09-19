//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
    pGolf = new Golf;
    pPlay = new GolfPlay;
    
    pCamera = new Camera(m_outputWidth, m_outputHeight);

    pCamera->InintializePreSwingCamera(pGolf->GetTeePos(), pGolf->GetTeeDirection());

    if (m_isInDebugMode == false)
    {
        m_currentState = GameState::GAMESTATE_INTROSCREEN;
    }
    else
    {
        m_currentState = GameState::GAMESTATE_GAMEPLAY;
    }
}

Game::~Game()
{
    if (m_audioEngine)
    {
        m_audioEngine->Suspend();
    }
    m_audioMusicStream.reset();
    m_audioEffectStream.reset();

    delete pGolf;
    delete pPlay;
    delete pCamera;
}

void Game::AudioPlayMusic(XACT_WAVEBANK_AUDIOBANK aSFX)
{
    m_audioMusicStream = m_audioBank->CreateStreamInstance(aSFX);

    if (m_audioMusicStream)
    {
        m_audioMusicStream->SetVolume(m_musicVolume);
        m_audioMusicStream->Play(true);
    }
}

void Game::AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK aSFX)
{
    m_audioEffectStream = m_audioBank->CreateStreamInstance(aSFX);
    if (m_audioEffectStream)
    {
        m_audioEffectStream->SetVolume(m_sfxVolume);
        m_audioEffectStream->Play();
    }
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
    ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
    m_world = DirectX::SimpleMath::Matrix::Identity;
    m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
    m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    m_effect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;

    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));

    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(m_d3dContext.Get());
    
    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);
    
    /* // For multisampling rendering
    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE, TRUE); // Multisampling
    */
    DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rastDesc, m_raster.ReleaseAndGetAddressOf()));

    m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"myfile.spritefont");
    m_titleFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"titleFont.spritefont");
    m_bitwiseFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"bitwise24.spritefont");
    m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());

    // Start Texture
    ComPtr<ID3D11Resource> resource;
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarFrame.png", nullptr, m_powerFrameTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarMeter.png", nullptr, m_powerMeterTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarImpact.png", nullptr, m_powerImpactTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarBackswing.png", nullptr, m_powerBackswingTexture.ReleaseAndGetAddressOf()));

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarFrame.png", resource.GetAddressOf(), m_powerFrameTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarMeter.png", resource.GetAddressOf(), m_powerMeterTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarImpact.png", resource.GetAddressOf(), m_powerImpactTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarBackswing.png", resource.GetAddressOf(), m_powerBackswingTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> PowerbarFrame;
    ComPtr<ID3D11Texture2D> PowerbarMeter;
    ComPtr<ID3D11Texture2D> PowerbarImpact;
    ComPtr<ID3D11Texture2D> PowerbarBackswing;
    DX::ThrowIfFailed(resource.As(&PowerbarFrame));
    DX::ThrowIfFailed(resource.As(&PowerbarMeter));
    DX::ThrowIfFailed(resource.As(&PowerbarImpact));
    DX::ThrowIfFailed(resource.As(&PowerbarBackswing));

    CD3D11_TEXTURE2D_DESC PowerbarFrameDesc;
    PowerbarFrame->GetDesc(&PowerbarFrameDesc);
    CD3D11_TEXTURE2D_DESC PowerbarMeterDesc;
    PowerbarMeter->GetDesc(&PowerbarMeterDesc);
    CD3D11_TEXTURE2D_DESC PowerbarImpactDesc;
    PowerbarImpact->GetDesc(&PowerbarImpactDesc);
    CD3D11_TEXTURE2D_DESC PowerbarBackswingDesc;
    PowerbarBackswing->GetDesc(&PowerbarBackswingDesc);

    m_powerBarFrameOrigin.x = float(PowerbarFrameDesc.Width / 2);
    m_powerBarFrameOrigin.y = float(PowerbarFrameDesc.Height / 2);
    m_powerBarMeterOrigin.x = float(PowerbarMeterDesc.Width / 2);
    m_powerBarMeterOrigin.y = float(PowerbarMeterDesc.Height / 2);
    m_powerBarImpactOrigin.x = float(PowerbarImpactDesc.Width / 2);
    m_powerBarImpactOrigin.y = float(PowerbarImpactDesc.Height / 2);
    m_powerBarBackswingOrigin.x = float(PowerbarBackswingDesc.Width / 2);
    m_powerBarBackswingOrigin.y = float(PowerbarBackswingDesc.Height / 2);

    // Character Select Textures
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"ChacterSpriteSheet.png", nullptr, m_characterTexture.ReleaseAndGetAddressOf()));
    m_character = std::make_unique<AnimatedTexture>();
    m_character->Load(m_characterTexture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter0SpriteSheet.png", nullptr, m_character0Texture.ReleaseAndGetAddressOf()));
    m_character0 = std::make_unique<AnimatedTexture>();
    m_character0->Load(m_character0Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter1SpriteSheet.png", nullptr, m_character1Texture.ReleaseAndGetAddressOf()));
    m_character1 = std::make_unique<AnimatedTexture>();
    m_character1->Load(m_character1Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"Chacter2SpriteSheet.png", nullptr, m_character2Texture.ReleaseAndGetAddressOf()));
    m_character2 = std::make_unique<AnimatedTexture>();
    m_character2->Load(m_character2Texture.Get(), 4, 6);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"CharacterBackground.png", resource.GetAddressOf(), m_characterBackgroundTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> characterBackground;
    DX::ThrowIfFailed(resource.As(&characterBackground));
    CD3D11_TEXTURE2D_DESC characterBackgroundDesc;
    characterBackground->GetDesc(&characterBackgroundDesc);
    m_characterBackgroundOrigin.x = float(characterBackgroundDesc.Width / 2);
    m_characterBackgroundOrigin.y = float(characterBackgroundDesc.Height / 2);

    // Environment Select textures
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectCalm.png", nullptr, m_environSelectCalmTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectCalm.png", resource.GetAddressOf(), m_environSelectCalmTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> pEnvironmentSelectCalm;
    DX::ThrowIfFailed(resource.As(&pEnvironmentSelectCalm));
    CD3D11_TEXTURE2D_DESC EnvironmentSelectCalmDesc;
    pEnvironmentSelectCalm->GetDesc(&EnvironmentSelectCalmDesc);
    m_environSelectCalmOrigin.x = float(EnvironmentSelectCalmDesc.Width / 2);
    m_environSelectCalmOrigin.y = float(EnvironmentSelectCalmDesc.Height / 2);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectBreezy.png", nullptr, m_environSelectBreezyTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectBreezy.png", resource.GetAddressOf(), m_environSelectBreezyTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> pEnvironmentSelectBreezy;
    DX::ThrowIfFailed(resource.As(&pEnvironmentSelectBreezy));
    CD3D11_TEXTURE2D_DESC EnvironmentSelectBreezyDesc;
    pEnvironmentSelectBreezy->GetDesc(&EnvironmentSelectBreezyDesc);
    m_environSelectBreezyOrigin.x = float(EnvironmentSelectBreezyDesc.Width / 2);
    m_environSelectBreezyOrigin.y = float(EnvironmentSelectBreezyDesc.Height / 2);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectAlien.jpg", nullptr, m_environSelectAlienTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"EnvironmentSelectAlien.jpg", resource.GetAddressOf(), m_environSelectAlienTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> pEnvironmentSelectAlien;
    DX::ThrowIfFailed(resource.As(&pEnvironmentSelectAlien));
    CD3D11_TEXTURE2D_DESC EnvironmentSelectAlienDesc;
    pEnvironmentSelectAlien->GetDesc(&EnvironmentSelectAlienDesc);
    m_environSelectAlienOrigin.x = float(EnvironmentSelectAlienDesc.Width / 2);
    m_environSelectAlienOrigin.y = float(EnvironmentSelectAlienDesc.Height / 2);

    // Intro screen textures
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"logoBMW.png", nullptr, m_bmwLogoTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"logoBMW.png", resource.GetAddressOf(), m_bmwLogoTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> logoBMW;
    DX::ThrowIfFailed(resource.As(&logoBMW));
    CD3D11_TEXTURE2D_DESC logoBMWDesc;
    logoBMW->GetDesc(&logoBMWDesc);
    m_bmwLogoOrigin.x = float(logoBMWDesc.Width / 2);
    m_bmwLogoOrigin.y = float(logoBMWDesc.Height / 2);

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"logoJI.png", nullptr, m_jiLogoTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"logoJI.png", resource.GetAddressOf(), m_jiLogoTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Texture2D> logoJI;
    DX::ThrowIfFailed(resource.As(&logoJI));
    CD3D11_TEXTURE2D_DESC logoJIDesc;
    logoJI->GetDesc(&logoJIDesc);
    m_jiLogoOrigin.x = float(logoJIDesc.Width / 2);
    m_jiLogoOrigin.y = float(logoJIDesc.Height / 2);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        //swapChainDesc.SampleDesc.Count = 8; // multisampling  RenderTesting
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
        ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    //CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, 8, 0);  // For multisampling rendering

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    //CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);  //for multisampling RenderTesting

    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.

    ////********* WLJ world start ----- deactivate to turn off world spin
    m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
    m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
    // world end

    // UI font positions
    m_fontPos.x = backBufferWidth / 2.f;
    m_fontPos.y = backBufferHeight / 2.f;
    m_fontPos2.x = backBufferWidth - 5.f;
    m_fontPos2.y = 35;
    m_fontPosDebug.x = 480;
    m_fontPosDebug.y = 35;
    m_fontMenuPos.x = backBufferWidth / 2.f;
    m_fontMenuPos.y = 35;
    m_bitwiseFontPos.x = backBufferWidth / 2.f;
    m_bitwiseFontPos.y = backBufferHeight / 2.f;

    // Start swing power bar
    m_powerMeterFrameRect.left = static_cast<long>((backBufferWidth / 2.f) - m_powerBarFrameOrigin.x);
    m_powerMeterFrameRect.right = static_cast<long>((backBufferWidth / 2.f) + m_powerBarFrameOrigin.x);
    m_powerMeterFrameRect.top = static_cast<long>((backBufferHeight / 1.08f) - m_powerBarFrameOrigin.y);
    m_powerMeterFrameRect.bottom = static_cast<long>((backBufferHeight / 1.08f) + m_powerBarFrameOrigin.y);

    m_powerMeterSize = static_cast<float>(m_powerMeterFrameRect.right - m_powerMeterFrameRect.left);

    float powerMeterScale = pPlay->GetMeterLength();
    float impactPointScale = pPlay->GetMeterImpactPoint();
    impactPointScale = impactPointScale * (m_powerMeterSize / powerMeterScale);
    m_powerMeterImpactPoint = m_powerMeterFrameRect.right - impactPointScale;

    m_powerMeterImpactRect.top = m_powerMeterFrameRect.top;
    m_powerMeterImpactRect.bottom = m_powerMeterFrameRect.bottom;
    m_powerMeterImpactRect.right = m_powerMeterFrameRect.right - static_cast<long>(impactPointScale) + 20;
    m_powerMeterImpactRect.left = m_powerMeterFrameRect.right - static_cast<long>(impactPointScale) - 20;

    m_powerMeterBarRect = m_powerMeterFrameRect;
    m_powerMeterBarRect.left = m_powerMeterFrameRect.right - static_cast<long>(impactPointScale);
    m_powerMeterBarRect.right = m_powerMeterFrameRect.right - static_cast<long>(impactPointScale);

    m_powerMeterBackswingRect = m_powerMeterFrameRect;
    m_powerMeterBackswingRect.left = m_powerMeterBarRect.left;
    m_powerMeterBackswingRect.right = m_powerMeterBarRect.right;

    m_powerMeterBarScale = 1.0f - (pPlay->GetMeterImpactPoint() / pPlay->GetMeterLength());

    // Character textures  
    m_characterPos.x = backBufferWidth / 2.f;
    m_characterPos.y = backBufferHeight / 2.f;
    m_character0Pos.x = backBufferWidth / 2.f;
    m_character0Pos.y = backBufferHeight / 2.f;
    m_character1Pos.x = backBufferWidth / 2.f;
    m_character1Pos.y = backBufferHeight / 2.f;
    m_character2Pos.x = backBufferWidth / 2.f;
    m_character2Pos.y = backBufferHeight / 2.f;   
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"CharacterBackground.png", nullptr, m_characterBackgroundTexture.ReleaseAndGetAddressOf()));
    m_characterBackgroundPos.x = backBufferWidth / 2.f;
    m_characterBackgroundPos.y = backBufferHeight / 2.f;

    // Environment select textures
    m_environSelectCalmPos.x = backBufferWidth / 2.f;
    m_environSelectCalmPos.y = backBufferHeight / 2.f;

    m_environSelectBreezyPos.x = backBufferWidth / 2.f;
    m_environSelectBreezyPos.y = backBufferHeight / 2.f;

    m_environSelectAlienPos.x = backBufferWidth / 2.f;
    m_environSelectAlienPos.y = backBufferHeight / 2.f;

    // Intro Sceen textures
    m_bmwLogoPos.x = backBufferWidth / 2.f;
    m_bmwLogoPos.y = backBufferHeight / 2.f;

    m_jiLogoPos.x = backBufferWidth / 2.f;
    m_jiLogoPos.y = backBufferHeight / 2.f;
    // End Texture
}

void Game::DrawCameraFocus()
{
    const float line = .25f;
    DirectX::SimpleMath::Vector3 focalPoint = pCamera->GetTargetPos();
    DirectX::SimpleMath::Vector3 yLine = focalPoint;
    yLine.y += line;
    DirectX::SimpleMath::Vector3 xLine = focalPoint;
    xLine.x += line;
    DirectX::SimpleMath::Vector3 zLine = focalPoint;
    zLine.z += line;

    VertexPositionColor origin(focalPoint, Colors::Yellow);
    VertexPositionColor yOffset(yLine, Colors::Yellow);
    VertexPositionColor xOffset(xLine, Colors::Yellow);
    VertexPositionColor zOffset(zLine, Colors::Yellow);
    m_batch->DrawLine(origin, yOffset);
    m_batch->DrawLine(origin, xOffset);
    m_batch->DrawLine(origin, zOffset);
}

void Game::DrawFlagAndHole()
{
    std::vector<DirectX::VertexPositionColor> holeVert = pGolf->GetHoleVertex();
    
    for (int i = 0; i < holeVert.size() - 1; ++i)
    {
        m_batch->DrawLine(holeVert[i], holeVert[i + 1]);
    }
    
    std::vector<DirectX::VertexPositionColor> flagVert = pGolf->GetFlagVertex();
    if (flagVert.size() == 5)
    {
        m_batch->DrawLine(flagVert[0], flagVert[1]);
        m_batch->DrawTriangle(flagVert[2], flagVert[3], flagVert[4]);
    }
    else
    {
        // error handle vector is of wrong size for needed inputs to draw
    }
}

void Game::DrawHydraShot()
{
    std::vector<DirectX::XMVECTORF32> lineColor;

    lineColor.push_back(DirectX::Colors::AliceBlue);
    lineColor.push_back(DirectX::Colors::CadetBlue);
    lineColor.push_back(DirectX::Colors::CornflowerBlue);

    lineColor.push_back(DirectX::Colors::Red);
    lineColor.push_back(DirectX::Colors::OrangeRed);
    lineColor.push_back(DirectX::Colors::DarkRed);

    lineColor.push_back(DirectX::Colors::Yellow);
    lineColor.push_back(DirectX::Colors::ForestGreen);
    lineColor.push_back(DirectX::Colors::DarkSeaGreen);

    std::vector<DirectX::XMVECTORF32> lineStartColor;
    lineStartColor.push_back(DirectX::Colors::White);
    lineStartColor.push_back(DirectX::Colors::White);
    lineStartColor.push_back(DirectX::Colors::White);

    lineStartColor.push_back(DirectX::Colors::Yellow);
    lineStartColor.push_back(DirectX::Colors::Yellow);
    lineStartColor.push_back(DirectX::Colors::Yellow);

    lineStartColor.push_back(DirectX::Colors::Orange);
    lineStartColor.push_back(DirectX::Colors::Orange);
    lineStartColor.push_back(DirectX::Colors::Orange);

    std::vector<DirectX::XMVECTORF32> lineEndColor;
    lineEndColor.push_back(DirectX::Colors::Red);
    lineEndColor.push_back(DirectX::Colors::Red);
    lineEndColor.push_back(DirectX::Colors::Red);

    lineEndColor.push_back(DirectX::Colors::Blue);
    lineEndColor.push_back(DirectX::Colors::Blue);
    lineEndColor.push_back(DirectX::Colors::Blue);

    lineEndColor.push_back(DirectX::Colors::Yellow);
    lineEndColor.push_back(DirectX::Colors::Yellow);
    lineEndColor.push_back(DirectX::Colors::Yellow);

    std::vector<std::vector<DirectX::SimpleMath::Vector3>> hydraPath = pGolf->GetHydraShotPath();
    //std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();

    if (hydraPath.size() > 1)
    {
        std::vector<std::vector<float>> hydraTimeStep = pGolf->GetHydraTimeStep();
        //GetHydraShotPath
        std::vector<float> shotTimeStep = pGolf->GetShotPathTimeSteps();

        for (int i = 0; i < hydraPath.size(); ++i)
        {
            DirectX::SimpleMath::Vector3 prevPos2 = hydraPath[i][0];
            int ballPosIndex2 = 0;
            for (int j = 0; j < hydraPath[i].size(); ++j)
            {
                DirectX::SimpleMath::Vector3 p1(prevPos2);
                DirectX::SimpleMath::Vector3 p2(hydraPath[i][j]);
                //VertexPositionColor aV(p1, lineColor[i]);
               // VertexPositionColor bV(p2, lineColor[i]);
                VertexPositionColor aV(p1, lineEndColor[i]);
                VertexPositionColor bV(p2, lineEndColor[i]);
                //VertexPositionColor aV(p1, DirectX::Colors::White);
                //VertexPositionColor bV(p2, DirectX::Colors::White);

                if (hydraTimeStep[i][j] < m_projectileTimer)
                {
                    m_batch->DrawLine(aV, bV);
                    ballPosIndex2 = j;
                }
                prevPos2 = hydraPath[i][j];
            }
            //pGolf->SetBallPosition(hydraPath[0][ballPosIndex2]);
        }
        if (pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
        {
            pCamera->SetTargetPos(pGolf->GetBallPosition());
        }
    }
}

void Game::DrawIntroScreen()
{
    float fadeDuration = 1.5f;
    float logoDisplayDuration = 5.f;
    float logoDisplayGap = 1.f;
    float startDelay = 4.2f;  
    float timeStamp = static_cast<float>(m_timer.GetTotalSeconds());
    
    float fadeInStart1 = startDelay;
    float fadeInStart2 = startDelay + logoDisplayDuration + logoDisplayGap;
    float fadeInEnd1 = startDelay + fadeDuration;
    float fadeInEnd2 = startDelay + logoDisplayDuration + logoDisplayGap + fadeDuration;
    float fadeOutStart1 = startDelay + logoDisplayDuration - fadeDuration;
    float fadeOutStart2 = startDelay + logoDisplayDuration + logoDisplayGap + logoDisplayDuration - fadeDuration;
    float fadeOutEnd1 = startDelay + logoDisplayDuration;
    float fadeOutEnd2 = startDelay + logoDisplayDuration + logoDisplayGap + logoDisplayDuration;

    DirectX::XMVECTORF32 fadeColor = DirectX::Colors::White;

    if (timeStamp < fadeInStart1)
    {
        // Render nothing
    }
    else if (timeStamp < fadeOutEnd1)
    {
        std::string textLine = "Proudly Presents";
        float textLinePosX = m_bitwiseFontPos.x;
        float textLinePosY = m_bitwiseFontPos.y + 100;
        DirectX::SimpleMath::Vector2 textLinePos(textLinePosX, textLinePosY);
        DirectX::SimpleMath::Vector2 textLineOrigin = m_bitwiseFont->MeasureString(textLine.c_str()) / 2.f;

        if (timeStamp < fadeInEnd1)  // fade in
        {
            float colorIntensity = (timeStamp - fadeInStart1) / fadeDuration;
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_jiLogoTexture.Get(), m_jiLogoPos, nullptr, fadeColor, 0.f, m_jiLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        else if (timeStamp > fadeOutStart1) // fade out
        {
            float colorIntensity = (fadeOutEnd1 - timeStamp) / (fadeDuration);
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_jiLogoTexture.Get(), m_jiLogoPos, nullptr, fadeColor, 0.f, m_jiLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        else // display at full intesity
        {
            m_spriteBatch->Draw(m_jiLogoTexture.Get(), m_jiLogoPos, nullptr, fadeColor, 0.f, m_jiLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
    }
    else if (timeStamp < fadeInStart2)
    {
        // render nothing
    }
    else if (timeStamp < fadeOutEnd2)
    {
        std::string textLine = "A Baltimore Magic Werks Production";
        float textLinePosX = m_bitwiseFontPos.x;
        float textLinePosY = m_bitwiseFontPos.y + 450;
        DirectX::SimpleMath::Vector2 textLinePos(textLinePosX, textLinePosY);
        DirectX::SimpleMath::Vector2 textLineOrigin = m_bitwiseFont->MeasureString(textLine.c_str()) / 2.f;
        if (timeStamp < fadeInEnd2)  // fade in
        {
            float colorIntensity = (timeStamp - fadeInStart2) / (fadeDuration);
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        else if (timeStamp > fadeOutStart2) // fade out
        {
            float colorIntensity = (fadeOutEnd2 - timeStamp) / (fadeDuration);
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }       
        else
        {
            AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK::XACT_WAVEBANK_AUDIOBANK_COINSFX);
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
    }
    if (timeStamp > fadeOutEnd2 + logoDisplayGap)
    {
        AudioPlayMusic(XACT_WAVEBANK_AUDIOBANK::XACT_WAVEBANK_AUDIOBANK_MUSIC01);
        m_currentState = GameState::GAMESTATE_STARTSCREEN;
    }
}

void Game::DrawMenuCharacterSelect()
{
    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

    std::string menuTitle = "Character Select";
    float menuTitlePosX = m_fontMenuPos.x;
    float menuTitlePosY = m_fontMenuPos.y + 25.f;
    DirectX::SimpleMath::Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    DirectX::SimpleMath::Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(1.f, 1.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);

    float ySpacing = 50.f;
    float posY0 = 250.0f;
    float posX0 = backBufferWidth * .20f;

    std::string menuObj0String = pGolf->GetCharacterName(0);

    DirectX::SimpleMath::Vector2 menuObj0Pos(posX0, posY0);
    DirectX::SimpleMath::Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;

    m_characterBackgroundOrigin = menuObj0Origin;
    float half = m_characterBackgroundOrigin.x / 2.f;

    m_character0Pos.x = posX0 + half + 25.f - menuObj0Origin.x;
    m_character0Pos.y = m_fontMenuPos.y + menuTitlePosY + 20;
    m_characterBackgroundPos.x = posX0 + half + 25.f;
    m_characterBackgroundPos.y = m_character0Pos.y + 10;

    posY0 += ySpacing;
    int i = 0;

    std::string dataString = "Data: ";
    DirectX::SimpleMath::Vector2 dataOrigin = m_bitwiseFont->MeasureString(dataString.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 dataPos;
    dataPos.x = posX0;
    dataPos.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString0 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin0 = m_bitwiseFont->MeasureString(armLengthString0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 armLengthPos0;
    armLengthPos0.x = posX0;
    armLengthPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString0.c_str(), armLengthPos0, Colors::White, 0.f, armLengthOrigin0);

    std::string armMassString0 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin0 = m_bitwiseFont->MeasureString(armMassString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos0;
    posY0 += ySpacing;
    armMassPos0.x = posX0;
    armMassPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString0.c_str(), armMassPos0, Colors::White, 0.f, armMassOrigin0);

    std::string clubLengthModString0 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin0 = m_bitwiseFont->MeasureString(clubLengthModString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos0;
    posY0 += ySpacing;
    clubLengthModPos0.x = posX0;
    clubLengthModPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString0.c_str(), clubLengthModPos0, Colors::White, 0.f, clubLengthModOrigin0);

    std::string armBalancePointString0 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin0 = m_bitwiseFont->MeasureString(armBalancePointString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos0;
    posY0 += ySpacing;
    armBalancePointPos0.x = posX0;
    armBalancePointPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString0.c_str(), armBalancePointPos0, Colors::White, 0.f, armBalancePointOrigin0);

    std::string armMassMoIString0 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin0 = m_bitwiseFont->MeasureString(armMassMoIString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos0;
    posY0 += ySpacing;
    armMassMoIPos0.x = posX0;
    armMassMoIPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString0.c_str(), armMassMoIPos0, Colors::White, 0.f, armMassMoIOrigin0);

    std::string bioString = "Bio:";
    DirectX::SimpleMath::Vector2 bioOrigin = m_bitwiseFont->MeasureString(bioString.c_str()) / 2.f;
    posY0 += ySpacing + ySpacing;
    DirectX::SimpleMath::Vector2 bioPos;
    bioPos.x = posX0;
    bioPos.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String0 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin0 = m_bitwiseFont->MeasureString(bioLine0String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine0Pos0;
    bioLine0Pos0.x = posX0;
    bioLine0Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String0.c_str(), bioLine0Pos0, Colors::White, 0.f, bioLine0Origin0);

    std::string bioLine1String0 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin0 = m_bitwiseFont->MeasureString(bioLine1String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine1Pos0;
    bioLine1Pos0.x = posX0;
    bioLine1Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String0.c_str(), bioLine1Pos0, Colors::White, 0.f, bioLine1Origin0);

    std::string bioLine2String0 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin0 = m_bitwiseFont->MeasureString(bioLine2String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine2Pos0;
    bioLine2Pos0.x = posX0;
    bioLine2Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String0.c_str(), bioLine2Pos0, Colors::White, 0.f, bioLine2Origin0);

    std::string bioLine3String0 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Origin0 = m_bitwiseFont->MeasureString(bioLine3String0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 bioLine3Pos0;
    bioLine3Pos0.x = posX0;
    bioLine3Pos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String0.c_str(), bioLine3Pos0, Colors::White, 0.f, bioLine3Origin0);

    ///////////////////////////////////////////////////////////
    if (m_menuSelect == 0)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character0->Draw(m_spriteBatch.get(), m_character0Pos);

    float posX1 = backBufferWidth / 2.0f;
    float posY1 = 250.0f;
    std::string menuObj1String = pGolf->GetCharacterName(1);

    DirectX::SimpleMath::Vector2 menuObj1Pos(posX1, posY1);
    DirectX::SimpleMath::Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    m_characterBackgroundPos.x = posX1 + half + 10.f;
    m_character1Pos.x = m_characterBackgroundPos.x - menuObj0Origin.x;
    m_character1Pos.y = m_character0Pos.y;

    posY1 += ySpacing + ySpacing;
    ++i;

    dataPos.x = posX1;
    dataPos.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString1 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin1 = m_bitwiseFont->MeasureString(armLengthString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armLengthPos1;
    posY1 += ySpacing;
    armLengthPos1.x = posX1;
    armLengthPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString1.c_str(), armLengthPos1, Colors::White, 0.f, armLengthOrigin1);

    std::string armMassString1 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin1 = m_bitwiseFont->MeasureString(armMassString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos1;
    posY1 += ySpacing;
    armMassPos1.x = posX1;
    armMassPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString1.c_str(), armMassPos1, Colors::White, 0.f, armMassOrigin1);

    std::string clubLengthModString1 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin1 = m_bitwiseFont->MeasureString(clubLengthModString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos1;
    posY1 += ySpacing;
    clubLengthModPos1.x = posX1;
    clubLengthModPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString1.c_str(), clubLengthModPos1, Colors::White, 0.f, clubLengthModOrigin1);

    std::string armBalancePointString1 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin1 = m_bitwiseFont->MeasureString(armBalancePointString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos1;
    posY1 += ySpacing;
    armBalancePointPos1.x = posX1;
    armBalancePointPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString1.c_str(), armBalancePointPos1, Colors::White, 0.f, armBalancePointOrigin1);

    std::string armMassMoIString1 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin1 = m_bitwiseFont->MeasureString(armMassMoIString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos1;
    posY1 += ySpacing;
    armMassMoIPos1.x = posX1;
    armMassMoIPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString1.c_str(), armMassMoIPos1, Colors::White, 0.f, armMassMoIOrigin1);

    posY1 += ySpacing + ySpacing;
    bioPos.x = posX1;
    bioPos.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String1 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin1 = m_bitwiseFont->MeasureString(bioLine0String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine0Pos1;
    posY1 += ySpacing;
    bioLine0Pos1.x = posX1;
    bioLine0Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String1.c_str(), bioLine0Pos1, Colors::White, 0.f, bioLine0Origin1);

    std::string bioLine1String1 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin1 = m_bitwiseFont->MeasureString(bioLine1String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine1Pos1;
    posY1 += ySpacing;
    bioLine1Pos1.x = posX1;
    bioLine1Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String1.c_str(), bioLine1Pos1, Colors::White, 0.f, bioLine1Origin1);

    std::string bioLine2String1 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin1 = m_bitwiseFont->MeasureString(bioLine2String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine2Pos1;
    posY1 += ySpacing;
    bioLine2Pos1.x = posX1;
    bioLine2Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String1.c_str(), bioLine2Pos1, Colors::White, 0.f, bioLine2Origin1);

    std::string bioLine3String1 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Origin1 = m_bitwiseFont->MeasureString(bioLine3String1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine3Pos1;
    posY1 += ySpacing;
    bioLine3Pos1.x = posX1;
    bioLine3Pos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String1.c_str(), bioLine3Pos1, Colors::White, 0.f, bioLine3Origin1);

    if (m_menuSelect == 1)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character1->Draw(m_spriteBatch.get(), m_character1Pos);

    float posX2 = backBufferWidth * .80f;
    float posY2 = 250.0f;

    std::string menuObj2String = pGolf->GetCharacterName(2);

    DirectX::SimpleMath::Vector2 menuObj2Pos(posX2, posY2);
    DirectX::SimpleMath::Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    m_characterBackgroundPos.x = posX2 + half;
    m_character2Pos.x = m_characterBackgroundPos.x - 125.f;;
    m_character2Pos.y = m_character0Pos.y;

    ++i;
    posY2 += ySpacing + ySpacing;

    dataPos.x = posX2;
    dataPos.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    std::string armLengthString2 = pGolf->GetCharacterArmLength(i);
    DirectX::SimpleMath::Vector2 armLengthOrigin2 = m_bitwiseFont->MeasureString(armLengthString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armLengthPos2;
    posY2 += ySpacing;
    armLengthPos2.x = posX2;
    armLengthPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armLengthString2.c_str(), armLengthPos2, Colors::White, 0.f, armLengthOrigin2);

    std::string armMassString2 = pGolf->GetCharacterArmMass(i);
    DirectX::SimpleMath::Vector2 armMassOrigin2 = m_bitwiseFont->MeasureString(armMassString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassPos2;
    posY2 += ySpacing;
    armMassPos2.x = posX2;
    armMassPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassString2.c_str(), armMassPos2, Colors::White, 0.f, armMassOrigin2);

    std::string clubLengthModString2 = pGolf->GetCharacterClubLengthMod(i);
    DirectX::SimpleMath::Vector2 clubLengthModOrigin2 = m_bitwiseFont->MeasureString(clubLengthModString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 clubLengthModPos2;
    posY2 += ySpacing;
    clubLengthModPos2.x = posX2;
    clubLengthModPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), clubLengthModString2.c_str(), clubLengthModPos2, Colors::White, 0.f, clubLengthModOrigin2);

    std::string armBalancePointString2 = pGolf->GetCharacterArmBalancePoint(i);
    DirectX::SimpleMath::Vector2 armBalancePointOrigin2 = m_bitwiseFont->MeasureString(armBalancePointString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armBalancePointPos2;
    posY2 += ySpacing;
    armBalancePointPos2.x = posX2;
    armBalancePointPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armBalancePointString2.c_str(), armBalancePointPos2, Colors::White, 0.f, armBalancePointOrigin2);

    std::string armMassMoIString2 = pGolf->GetCharacterArmMassMoI(i);
    DirectX::SimpleMath::Vector2 armMassMoIOrigin2 = m_bitwiseFont->MeasureString(armMassMoIString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 armMassMoIPos2;
    posY2 += ySpacing;
    armMassMoIPos2.x = posX2;
    armMassMoIPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), armMassMoIString2.c_str(), armMassMoIPos2, Colors::White, 0.f, armMassMoIOrigin2);

    posY2 += ySpacing + ySpacing;
    bioPos.x = posX2;
    bioPos.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioString.c_str(), bioPos, Colors::White, 0.f, bioOrigin);

    std::string bioLine0String2 = pGolf->GetCharacterBioLine0(i);
    DirectX::SimpleMath::Vector2 bioLine0Origin2 = m_bitwiseFont->MeasureString(bioLine0String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine0Pos2;
    posY2 += ySpacing;
    bioLine0Pos2.x = posX2;
    bioLine0Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine0String2.c_str(), bioLine0Pos2, Colors::White, 0.f, bioLine0Origin2);

    std::string bioLine1String2 = pGolf->GetCharacterBioLine1(i);
    DirectX::SimpleMath::Vector2 bioLine1Origin2 = m_bitwiseFont->MeasureString(bioLine1String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine1Pos2;
    posY2 += ySpacing;
    bioLine1Pos2.x = posX2;
    bioLine1Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine1String2.c_str(), bioLine1Pos2, Colors::White, 0.f, bioLine1Origin2);

    std::string bioLine2String2 = pGolf->GetCharacterBioLine2(i);
    DirectX::SimpleMath::Vector2 bioLine2Origin2 = m_bitwiseFont->MeasureString(bioLine2String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine2Pos2;
    posY2 += ySpacing;
    bioLine2Pos2.x = posX2;
    bioLine2Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine2String2.c_str(), bioLine2Pos2, Colors::White, 0.f, bioLine2Origin2);

    std::string bioLine3String2 = pGolf->GetCharacterBioLine3(i);
    DirectX::SimpleMath::Vector2 bioLine3Origin2 = m_bitwiseFont->MeasureString(bioLine2String2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 bioLine3Pos2;

    posY2 += ySpacing;
    bioLine3Pos2.x = posX2;
    bioLine3Pos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), bioLine3String2.c_str(), bioLine3Pos2, Colors::White, 0.f, bioLine3Origin2);

    if (m_menuSelect == 2)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(4.f, 4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(3.f, 3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(2.f, 2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(1.f, 1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + DirectX::SimpleMath::Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character2->Draw(m_spriteBatch.get(), m_character2Pos);

    if (m_menuSelect < 0 || m_menuSelect > 2)
    {
        m_menuSelect = 0;
    }

    // Start Menu Select Highlight
    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
}

void Game::DrawMenuEnvironmentSelect()
{
    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

    std::string menuTitle = "Environment Select";
    float menuTitlePosX = m_fontMenuPos.x;
    float menuTitlePosY = m_fontMenuPos.y + 25;;
    DirectX::SimpleMath::Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    DirectX::SimpleMath::Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(1.f, 1.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);

    std::vector<std::vector<std::string>> environData = pGolf->GetEnvironSelectStrings();

    float ySpacing = 50.f;
    float posX0 = backBufferWidth * .20f;
    float posY0 = 250.0f;
    int i = 0;
    int j = 0;

    std::string menuObj0String = environData[i][j];
    DirectX::SimpleMath::Vector2 menuObj0Pos(posX0, posY0);
    DirectX::SimpleMath::Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;

    std::string dataString = "Data: ";
    DirectX::SimpleMath::Vector2 dataOrigin = m_bitwiseFont->MeasureString(dataString.c_str()) / 2.f;
    posY0 += ySpacing + ySpacing;
    DirectX::SimpleMath::Vector2 dataPos;
    dataPos.x = posX0;
    dataPos.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    ++i;
    std::string airDensityString0 = environData[i][j]; // air density
    DirectX::SimpleMath::Vector2 airDensityOrigin0 = m_bitwiseFont->MeasureString(airDensityString0.c_str()) / 2.f;
    posY0 += ySpacing;
    DirectX::SimpleMath::Vector2 airDensityPos0;
    airDensityPos0.x = posX0;
    airDensityPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), airDensityString0.c_str(), airDensityPos0, Colors::White, 0.f, airDensityOrigin0);

    ++i;
    std::string gravityString0 = environData[i][j]; // gravity 
    DirectX::SimpleMath::Vector2 gravityOrigin0 = m_bitwiseFont->MeasureString(gravityString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 gravityPos0;
    posY0 += ySpacing;
    gravityPos0.x = posX0;
    gravityPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), gravityString0.c_str(), gravityPos0, Colors::White, 0.f, gravityOrigin0);

    ++i;
    std::string windXString0 = environData[i][j]; // wind x
    DirectX::SimpleMath::Vector2 windXOrigin0 = m_bitwiseFont->MeasureString(windXString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windXPos0;
    posY0 += ySpacing;
    windXPos0.x = posX0;
    windXPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windXString0.c_str(), windXPos0, Colors::White, 0.f, windXOrigin0);

    ++i;
    std::string windZString0 = environData[i][j]; // wind z
    DirectX::SimpleMath::Vector2 windZOrigin0 = m_bitwiseFont->MeasureString(windZString0.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windZPos0;
    posY0 += ySpacing;
    windZPos0.x = posX0;
    windZPos0.y = posY0;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windZString0.c_str(), windZPos0, Colors::White, 0.f, windZOrigin0);

    posY0 += ySpacing + ySpacing + ySpacing + ySpacing + ySpacing;
    m_environSelectCalmPos.x = posX0;
    m_environSelectCalmPos.y = posY0;
    m_spriteBatch->Draw(m_environSelectCalmTexture.Get(), m_environSelectCalmPos, nullptr, Colors::White, 0.f, m_environSelectCalmOrigin);

    //////// next environment point

    float posX1 = backBufferWidth / 2.0f;
    float posY1 = 250.0f;

    i = 0;
    ++j;
    std::string menuObj1String = environData[i][j];
    DirectX::SimpleMath::Vector2 menuObj1Pos(posX1, posY1);
    DirectX::SimpleMath::Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    posY1 += ySpacing + ySpacing;
    dataPos.x = posX1;
    dataPos.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    ++i;
    std::string airDensityString1 = environData[i][j]; // air density
    DirectX::SimpleMath::Vector2 airDensityOrigin1 = m_bitwiseFont->MeasureString(airDensityString1.c_str()) / 2.f;
    posY1 += ySpacing;
    DirectX::SimpleMath::Vector2 airDensityPos1;
    airDensityPos1.x = posX1;
    airDensityPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), airDensityString1.c_str(), airDensityPos1, Colors::White, 0.f, airDensityOrigin1);

    ++i;
    std::string gravityString1 = environData[i][j]; // gravity 
    DirectX::SimpleMath::Vector2 gravityOrigin1 = m_bitwiseFont->MeasureString(gravityString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 gravityPos1;
    posY1 += ySpacing;
    gravityPos1.x = posX1;
    gravityPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), gravityString1.c_str(), gravityPos1, Colors::White, 0.f, gravityOrigin1);

    ++i;
    std::string windXString1 = environData[i][j]; // wind x
    DirectX::SimpleMath::Vector2 windXOrigin1 = m_bitwiseFont->MeasureString(windXString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windXPos1;
    posY1 += ySpacing;
    windXPos1.x = posX1;
    windXPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windXString1.c_str(), windXPos1, Colors::White, 0.f, windXOrigin1);

    ++i;
    std::string windZString1 = environData[i][j]; // wind z
    DirectX::SimpleMath::Vector2 windZOrigin1 = m_bitwiseFont->MeasureString(windZString1.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windZPos1;
    posY1 += ySpacing;
    windZPos1.x = posX1;
    windZPos1.y = posY1;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windZString1.c_str(), windZPos1, Colors::White, 0.f, windZOrigin1);

    posY1 += ySpacing + ySpacing + ySpacing + ySpacing + ySpacing;
    m_environSelectBreezyPos.x = posX1;
    m_environSelectBreezyPos.y = posY1;
    m_spriteBatch->Draw(m_environSelectBreezyTexture.Get(), m_environSelectBreezyPos, nullptr, Colors::White, 0.f, m_environSelectBreezyOrigin);

    //////// next environment point

    float posX2 = backBufferWidth * .80f;
    float posY2 = 250.0f;

    i = 0;
    ++j;
    std::string menuObj2String = environData[i][j];

    DirectX::SimpleMath::Vector2 menuObj2Pos(posX2, posY2);
    DirectX::SimpleMath::Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    posY2 += ySpacing + ySpacing;
    dataPos.x = posX2;
    dataPos.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), dataString.c_str(), dataPos, Colors::White, 0.f, dataOrigin);

    ++i;
    std::string airDensityString2 = environData[i][j]; // air density
    DirectX::SimpleMath::Vector2 airDensityOrigin2 = m_bitwiseFont->MeasureString(airDensityString2.c_str()) / 2.f;
    posY2 += ySpacing;
    DirectX::SimpleMath::Vector2 airDensityPos2;
    airDensityPos2.x = posX2;
    airDensityPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), airDensityString2.c_str(), airDensityPos2, Colors::White, 0.f, airDensityOrigin2);

    ++i;
    std::string gravityString2 = environData[i][j]; // gravity 
    DirectX::SimpleMath::Vector2 gravityOrigin2 = m_bitwiseFont->MeasureString(gravityString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 gravityPos2;
    posY2 += ySpacing;
    gravityPos2.x = posX2;
    gravityPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), gravityString2.c_str(), gravityPos2, Colors::White, 0.f, gravityOrigin2);

    ++i;
    std::string windXString2 = environData[i][j]; // wind x
    DirectX::SimpleMath::Vector2 windXOrigin2 = m_bitwiseFont->MeasureString(windXString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windXPos2;
    posY2 += ySpacing;
    windXPos2.x = posX2;
    windXPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windXString2.c_str(), windXPos2, Colors::White, 0.f, windXOrigin2);

    ++i;
    std::string windZString2 = environData[i][j]; // wind z
    DirectX::SimpleMath::Vector2 windZOrigin2 = m_bitwiseFont->MeasureString(windZString2.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 windZPos2;
    posY2 += ySpacing;
    windZPos2.x = posX2;
    windZPos2.y = posY2;
    m_bitwiseFont->DrawString(m_spriteBatch.get(), windZString2.c_str(), windZPos2, Colors::White, 0.f, windZOrigin2);

    posY2 += ySpacing + ySpacing + ySpacing + ySpacing + ySpacing;
    m_environSelectAlienPos.x = posX2;
    m_environSelectAlienPos.y = posY2;
    m_spriteBatch->Draw(m_environSelectAlienTexture.Get(), m_environSelectAlienPos, nullptr, Colors::White, 0.f, m_environSelectAlienOrigin);

    //////// start of menu highlight drawing

    if (m_menuSelect < 0 || m_menuSelect > 2)
    {
        m_menuSelect = 0;
    }

    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
}

void Game::DrawMenuMain()
{
    float lineDrawY = m_fontMenuPos.y + 15;
    float lineDrawSpacingY = 15;
    std::string menuTitle = "Main Menu";
    float menuTitlePosX = m_fontMenuPos.x;
    float menuTitlePosY = lineDrawY;
    DirectX::SimpleMath::Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    DirectX::SimpleMath::Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(1.f, 1.f), Colors::Green, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, menuOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);

    lineDrawY += menuTitlePosY + lineDrawSpacingY;
    std::string menuObj0String = "Play!";
    DirectX::SimpleMath::Vector2 menuObj0Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj1String = "Character Select";
    DirectX::SimpleMath::Vector2 menuObj1Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj2String = "Environment Select";
    DirectX::SimpleMath::Vector2 menuObj2Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    // Hydra Shot Demo
    ////////////////////////////
    lineDrawY += menuObj0Pos.y;
    std::string menuObjHydraString = "Hydra Shot Demo";
    DirectX::SimpleMath::Vector2 menuObjHydraPos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObjHydraOrigin = m_font->MeasureString(menuObjHydraString.c_str()) / 2.f;
    ///////////////////////////

    lineDrawY += menuObj0Pos.y;
    std::string menuObj3String = "Quit";
    DirectX::SimpleMath::Vector2 menuObj3Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj3Origin = m_font->MeasureString(menuObj3String.c_str()) / 2.f;

    //if (m_menuSelect < 0 || m_menuSelect > 3)
    if (m_menuSelect < 0 || m_menuSelect > 4)
    {
        m_menuSelect = 0;
    }
    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }

    if (m_menuSelect == 3)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObjHydraOrigin);

        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObjHydraOrigin);
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObjHydraOrigin);

        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos, Colors::White, 0.f, menuObjHydraOrigin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObjHydraString.c_str(), menuObjHydraPos, Colors::White, 0.f, menuObjHydraOrigin);
    }

    //if (m_menuSelect == 3)
    if (m_menuSelect == 4)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::White, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(4.f, -4.f), Colors::White, 0.f, menuObj3Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj3Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos + DirectX::SimpleMath::Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj3Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos, Colors::White, 0.f, menuObj3Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj3String.c_str(), menuObj3Pos, Colors::White, 0.f, menuObj3Origin);
    }
}

void Game::DrawPowerBarUI()
{
    if (pPlay->GetMeterPower() >= 0.0)
    {
        m_powerMeterBarRect.left = static_cast<long>(m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01f)));
    }
    else
    {
        m_powerMeterBarRect.right = static_cast<long>(m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01f)));
    }
    if (pPlay->GetIsBackswingSet() == false)
    {
        m_powerMeterBackswingRect.left = static_cast<long>(m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01f)));
    }
    else
    {
        m_powerMeterBackswingRect.left = static_cast<long>(m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetBackswingSet() * m_powerMeterBarScale) * 0.01f)));
    }

    m_spriteBatch->Draw(m_powerBackswingTexture.Get(), m_powerMeterBackswingRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerMeterTexture.Get(), m_powerMeterBarRect, nullptr, Colors::White);

    m_spriteBatch->Draw(m_powerFrameTexture.Get(), m_powerMeterFrameRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerImpactTexture.Get(), m_powerMeterImpactRect, nullptr, Colors::White);
}

// working old version prior to impmenting real time match update
void Game::DrawProjectile()
{
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();
    
    if (shotPath.size() > 1)
    {
        int stepCount = (int)shotPath.size();

        if (m_projectilePathStep >= stepCount)
        {
            m_flightStepTimer.ResetElapsedTime();
            m_projectilePathStep = 0;
        }
        pGolf->SetBallPosition(shotPath[m_projectilePathStep]);
        ++m_projectilePathStep;

        DirectX::SimpleMath::Vector3 prevPos = shotPath[0];
        for (int i = 0; i < m_projectilePathStep; ++i)
        {
            DirectX::SimpleMath::Vector3 p1(prevPos);
            DirectX::SimpleMath::Vector3 p2(shotPath[i]);

            VertexPositionColor aV(p1, Colors::White);
            VertexPositionColor bV(p2, Colors::White);

            m_batch->DrawLine(aV, bV);
            prevPos = shotPath[i];
        }
    }
}

void Game::DrawProjectileRealTime()
{
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();

    if (shotPath.size() > 1)
    {
        std::vector<float> shotTimeStep = pGolf->GetShotPathTimeSteps();

        DirectX::SimpleMath::Vector3 prevPos = shotPath[0];
        int ballPosIndex = 0;
        for (int i = 0; i < shotPath.size(); ++i)
        {
            DirectX::SimpleMath::Vector3 p1(prevPos);
            DirectX::SimpleMath::Vector3 p2(shotPath[i]);
            VertexPositionColor aV(p1, Colors::White);
            VertexPositionColor bV(p2, Colors::White);

            if (shotTimeStep[i] < m_projectileTimer)
            {
                m_batch->DrawLine(aV, bV);
                ballPosIndex = i;
            }
            prevPos = shotPath[i];
        }
        pGolf->SetBallPosition(shotPath[ballPosIndex]);

        // Set camera targe on ball position if using projectile tracking camera
        if (pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
        {
            pCamera->SetTargetPos(pGolf->GetBallPosition());
        }
    }
}

void Game::DrawStartScreen()
{
    const std::string title = "GolfGame1989";
    const std::string author = "By Lehr Jackson";
    const std::string startText = "Press Enter to Start";
    float fontTitlePosX = m_fontPos.x;
    float fontTitlePosY = m_fontPos.y / 2.f;
    DirectX::SimpleMath::Vector2 titlePos(fontTitlePosX, fontTitlePosY);
    float fontAuthorPosX = m_fontPos.x;
    float fontAuthorPosY = m_fontPos.y;
    DirectX::SimpleMath::Vector2 authorPos(fontAuthorPosX, fontAuthorPosY);
    DirectX::SimpleMath::Vector2 startTextPos(m_fontPos.x, m_fontPos.y + fontTitlePosY);

    DirectX::SimpleMath::Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 authorOrigin = m_font->MeasureString(author.c_str()) / 2.f;
    DirectX::SimpleMath::Vector2 startTextOrigin = m_font->MeasureString(startText.c_str()) / 2.f;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(7.f, 7.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(6.f, 6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(5.f, 5.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    m_font->DrawString(m_spriteBatch.get(), author.c_str(), authorPos, Colors::White, 0.f, authorOrigin);
    m_font->DrawString(m_spriteBatch.get(), startText.c_str(), startTextPos, Colors::White, 0.f, startTextOrigin);
}

void Game::DrawShotAimArrow()
{
    const float line = .25f;
    const float aimWidth = .02f;
    const float aimHeight = 0.01f;
    const float centerIndent = .15f;
    DirectX::SimpleMath::Vector3 aimLine = DirectX::SimpleMath::Vector3(line, 0.0f, 0.0f);
    DirectX::SimpleMath::Vector3 aimLineLeft = DirectX::SimpleMath::Vector3(0.05f, 0.0f, -aimWidth);
    DirectX::SimpleMath::Vector3 aimLineRight = DirectX::SimpleMath::Vector3(0.05f, 0.0f, aimWidth);
    aimLine = DirectX::SimpleMath::Vector3::Transform(aimLine, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection())));
    aimLineLeft = DirectX::SimpleMath::Vector3::Transform(aimLineLeft, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection())));
    aimLineRight = DirectX::SimpleMath::Vector3::Transform(aimLineRight, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection())));

    //aimLine = DirectX::SimpleMath::Vector3::Transform(aimLine, DirectX::SimpleMath::Matrix::CreateRotationY(pPlay->GetShotDirection()));
    //aimLineLeft = DirectX::SimpleMath::Vector3::Transform(aimLineLeft, DirectX::SimpleMath::Matrix::CreateRotationY(pPlay->GetShotDirection()));
    //aimLineRight = DirectX::SimpleMath::Vector3::Transform(aimLineRight, DirectX::SimpleMath::Matrix::CreateRotationY(pPlay->GetShotDirection()));
        
    aimLine += pGolf->GetShotStartPos();
    aimLineLeft += pGolf->GetShotStartPos();
    aimLineRight += pGolf->GetShotStartPos();

    DirectX::SimpleMath::Vector3 centerBase = DirectX::SimpleMath::Vector3(centerIndent, aimHeight, 0.0f);
    centerBase = DirectX::SimpleMath::Vector3::Transform(centerBase, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection())));
    //centerBase = DirectX::SimpleMath::Vector3::Transform(centerBase, DirectX::SimpleMath::Matrix::CreateRotationY(pPlay->GetShotDirection()));
    centerBase += pGolf->GetShotStartPos();

    VertexPositionColor origin(centerBase, Colors::Red);


    VertexPositionColor aimPoint(aimLine, Colors::Red);
    VertexPositionColor aimBaseLeft(aimLineLeft, Colors::Red);

    VertexPositionColor aimBaseRight(aimLineRight, Colors::DarkRed);

    VertexPositionColor originRight = origin;
    originRight.color.x = 0.5450980663F;
    VertexPositionColor aimPointRight = aimPoint;
    aimPointRight.color.x = 0.5450980663F;


    //origin.color.w = .0f;
    //origin.color.y = 1;
    //originRight.color.w = .0f;

    if (pPlay->GetShotDirection() <= 0.0)
    {
        m_batch->DrawTriangle(aimPoint, origin, aimBaseLeft);
        m_batch->DrawTriangle(aimPointRight, originRight, aimBaseRight);
    }
    else
    {
        m_batch->DrawTriangle(aimPointRight, originRight, aimBaseRight);
        m_batch->DrawTriangle(aimPoint, origin, aimBaseLeft);
    }

}

void Game::DrawShotAimCone()
{
    const float width = .015f;
    const float length = .25f;
    const float indent = .18f;
    DirectX::SimpleMath::Vector3 right = DirectX::SimpleMath::Vector3(length, 0.0f, width);
    DirectX::SimpleMath::Vector3 left = DirectX::SimpleMath::Vector3(length, 0.0f, -width);
    DirectX::SimpleMath::Vector3 center = DirectX::SimpleMath::Vector3(length - indent, 0.0f, 0.0f);
    
    DirectX::SimpleMath::Matrix rotationMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection()));
    right = DirectX::SimpleMath::Vector3::Transform(right, rotationMat);
    left = DirectX::SimpleMath::Vector3::Transform(left, rotationMat);
    center = DirectX::SimpleMath::Vector3::Transform(center, rotationMat);

    right += pGolf->GetShotStartPos();
    left += pGolf->GetShotStartPos();
    center += pGolf->GetShotStartPos();

    VertexPositionColor origin(pGolf->GetShotStartPos(), Colors::White);
    VertexPositionColor rightLine(right, Colors::White);
    VertexPositionColor leftLine(left, Colors::White);
    VertexPositionColor centerLine(center, Colors::Transparent);

    m_batch->DrawTriangle(origin, rightLine, centerLine);
    m_batch->DrawTriangle(origin, leftLine, centerLine);
}

void Game::DrawShotTimerUI()
{
    std::string timerUI = "Timer = " + std::to_string(m_projectileTimer);
    DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(timerUI.c_str());
    m_font->DrawString(m_spriteBatch.get(), timerUI.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
}

void Game::DrawSwing()
{
    std::vector<DirectX::SimpleMath::Vector3> angles = pGolf->GetRawSwingAngles();

    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection()));

    if (angles.size() > 1)
    {
        DirectX::SimpleMath::Vector3 origin = pGolf->GetSwingOriginOffsetPos() + pGolf->GetShotStartPos();

        const float thetaOriginOffsetY = -.02f;
        DirectX::SimpleMath::Vector3 thetaOrigin = DirectX::SimpleMath::Vector3::Zero;
        thetaOrigin.y += thetaOriginOffsetY;

        int swingStepCount = (int)angles.size();
        if (m_swingPathStep >= swingStepCount)
        {
            m_swingPathStep = 0;
        }
        ++m_swingPathStep;

        int impactPoint = pGolf->GetImpactStep();
        DirectX::XMVECTORF32 shoulderColor = DirectX::Colors::Blue;
        DirectX::XMVECTORF32 handColor = DirectX::Colors::White;
        DirectX::XMVECTORF32 clubHeadColor = DirectX::Colors::Red;
        bool isBallHit = false;
        for (int i = 0; i < m_swingPathStep; ++i)
        {
            if (i > impactPoint)
            {
                shoulderColor = DirectX::Colors::Gray;
                handColor = DirectX::Colors::Black;
                clubHeadColor = DirectX::Colors::Green;
                isBallHit = true;
            }
            if (pCamera->GetCameraState() == CameraState::CAMERASTATE_SWINGVIEW)
            {
                if (isBallHit == true)
                {
                    AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK::XACT_WAVEBANK_AUDIOBANK_IMPACTSFX1);
                    pCamera->SetCameraState(CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW);
                    m_projectileTimer = -0.05;  // Creates a slight delay before ball flight starts , removes abruptness of camera turn and looks/feels a little better I think
                }
            }
            DirectX::SimpleMath::Vector3 theta = DirectX::SimpleMath::Vector3::Transform(thetaOrigin, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].z));
            DirectX::SimpleMath::Vector3 beta = DirectX::SimpleMath::Vector3::Transform(theta, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].y));

            // Rotate to point swing draw in direction of swing aim
            theta = DirectX::SimpleMath::Vector3::Transform(theta, rotMat);
            beta = DirectX::SimpleMath::Vector3::Transform(beta, rotMat);

            theta += pGolf->GetSwingOriginOffsetPos() + pGolf->GetShotStartPos();
            beta += theta;
            VertexPositionColor shoulder(origin, shoulderColor);
            VertexPositionColor thetaColor(theta, handColor);
            VertexPositionColor betaColor(beta, clubHeadColor);
            m_batch->DrawLine(shoulder, thetaColor);
            m_batch->DrawLine(thetaColor, betaColor);
        }
    }
}

void Game::DrawSwingUI()
{
    std::vector<std::string> uiString = pPlay->GetDebugData();

    float fontOriginPosY = m_fontPosDebug.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());

        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
        m_fontPosDebug.y += 35;
    }
    m_fontPosDebug.y = fontOriginPosY;
}

void Game::DrawTeeBox()
{
    //draw tee box;
    const DirectX::SimpleMath::Vector3 teeBoxOrigin = pGolf->GetTeePos();
    const float teeBoxLengthScale = 0.05f;
    const float teeBoxHorizontalScale = 0.1f;
    DirectX::SimpleMath::Vector3 t1(teeBoxOrigin.x - teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z - teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t2(teeBoxOrigin.x + teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z - teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t3(teeBoxOrigin.x - teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z + teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t4(teeBoxOrigin.x + teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z + teeBoxHorizontalScale);
    VertexPositionColor vt1(t1, Colors::White);
    VertexPositionColor vt2(t2, Colors::White);
    VertexPositionColor vt3(t3, Colors::White);
    VertexPositionColor vt4(t4, Colors::White);
    m_batch->DrawLine(vt1, vt2);
    m_batch->DrawLine(vt1, vt3);
    m_batch->DrawLine(vt3, vt4);
    m_batch->DrawLine(vt4, vt2);
    // end tee box draw
}

void Game::DrawTree1(const DirectX::SimpleMath::Vector3 aTreePos)
{
    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);
    //viewHorizontal = viewHorizontal / 2;

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;
    /*
    treeRootTop.position.y += branchGap;
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
    branchEndR.y += branchGap;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
    branchEndL.y += branchGap;
    VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
    VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
    m_batch->DrawLine(treeRootTop, leafR);
    m_batch->DrawLine(treeRootTop, leafL);
    */

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        treeRootTop2.position.y += branchGap;
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        branchEndR.y += branchGap;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
        m_batch->DrawLine(treeRootTop2, leafR);
        m_batch->DrawLine(treeRootTop2, leafL);
    }
}

void Game::DrawTree2(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    //DirectX::SimpleMath::Vector3 swayVec = windVector * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds()));
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation)));
    
    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);
    //viewHorizontal = viewHorizontal / 2;

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {

        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        //treeRootTop2.position.y += branchGap;
        //treeRootTop2.position += swayVec;
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;    
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
        m_batch->DrawLine(branchBaseVertex, leafR);
        m_batch->DrawLine(branchBaseVertex, leafL);
    }
}

void Game::DrawTree3(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

    //DirectX::SimpleMath::Vector3 swayVec = windVector * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds()));
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .04;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine) * 1.1;
    //viewHorizontal = viewHorizontal / 2;

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        //treeRootTop2.position.y += branchGap;
        //treeRootTop2.position += swayVec;
        //DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

        branchEndR.y += branchGap;
        branchEndR += swayVec;
        //DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
        m_batch->DrawLine(branchBaseVertex, leafR);
        m_batch->DrawLine(branchBaseVertex, leafL);
    }
}

void Game::DrawTree4(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

    //DirectX::SimpleMath::Vector3 swayVec = windVector * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds()));
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .04;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine) * 1.1;
    //viewHorizontal = viewHorizontal / 2;

    const float branchGap = .01;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    testBaseTop += swayBase;
    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(testBaseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;
    
    //branchEndR.y += branchGap;
    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;
    //branchEndL.y += branchGap;
    branchEndL += swayVec;
    VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
    VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
    m_batch->DrawLine(branchBaseVertex, leafR);
    m_batch->DrawLine(branchBaseVertex, leafL);

    //// increse width
    float widthMod = 1.0;
    int layerHeight = 6;
    for (int i = 0; i < layerHeight * 3; ++i)
    {
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + branchBase;

        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + branchBase;

        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
        VertexPositionColor branchRoot(branchBase, Colors::ForestGreen);
        m_batch->DrawLine(branchRoot, leafR);
        m_batch->DrawLine(branchRoot, leafL);
        branchBase.y += branchGap;
        if (i < layerHeight)
        {
            widthMod += .2;
        }
        if (i >= layerHeight * 2)
        {
            widthMod -= .2;
        }
    }

    branchBaseVertex.position.y += branchGap;  
    leafR.position.y += branchGap;
    leafL.position.y += branchGap;
    //m_batch->DrawLine(branchBaseVertex, leafR);
   // m_batch->DrawLine(branchBaseVertex, leafL);
}

void Game::DrawTree5(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

    //DirectX::SimpleMath::Vector3 swayVec = windVector * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds()));
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .06;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);
    //viewHorizontal = viewHorizontal / 2;

    const float branchGap = .01;
    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;
    testBaseTop.y += branchGap;
    testBaseTop += swayBase;
    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(testBaseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        branchBase += swayBase;
        branchBase.y += branchGap;

        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        
        //treeRootTop2.position.y += branchGap;
        //treeRootTop2.position += swayVec;
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);
        m_batch->DrawLine(branchBaseVertex, leafR);
        m_batch->DrawLine(branchBaseVertex, leafL);
    }
}

void Game::DrawUI()
{
    std::vector<std::string> uiString = pGolf->GetUIstrings();

    std::string output = uiString[0];

    float fontOriginPosY = m_fontPos2.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPos2, Colors::White, 0.f, lineOrigin);
        m_fontPos2.y += 35;
    }

    m_fontPos2.y = fontOriginPosY;
}

void Game::DrawWorld()
{
    // draw world grid
    DirectX::SimpleMath::Vector3 xAxis(2.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 xFarAxis(6.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 zAxis(0.f, 0.f, 2.f);
    DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3::Zero;
    size_t divisions = 50;
    size_t extention = 50;

    DirectX::XMVECTORF32 gridColor = pGolf->GetTerrainColor();
    
    for (size_t i = 0; i <= divisions + extention; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        DirectX::SimpleMath::Vector3 scale = xAxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - zAxis, gridColor);
            VertexPositionColor v2(scale + zAxis, gridColor);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - zAxis, gridColor);
            VertexPositionColor v2(scale + zAxis, gridColor);
            m_batch->DrawLine(v1, v2);
        }
    }
    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        DirectX::SimpleMath::Vector3 scale = zAxis * fPercent + origin;

        if (scale.z == 0.0f)
        {
            VertexPositionColor v1(scale - xAxis, gridColor); // Center line
            VertexPositionColor v2(scale + xFarAxis, gridColor); // Center line
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, gridColor);
            VertexPositionColor v2(scale + xFarAxis, gridColor);
            m_batch->DrawLine(v1, v2);
        }
    }

    DrawTeeBox();
    DrawFlagAndHole();

    // Test Draw Tree
    DirectX::SimpleMath::Vector3 treePos(1.15, 0.0, 0.15);
    DrawTree2(treePos, .3);
    DirectX::SimpleMath::Vector3 treePos2(1.3, 0.0, .3);
    DrawTree2(treePos2, 0.7f);
    DirectX::SimpleMath::Vector3 treePos3(1.45, 0.0, 0.45);
    DrawTree2(treePos3, 5.5f);

    DirectX::SimpleMath::Vector3 treePos4(1.6, 0.0, 0.6);
    DrawTree2(treePos4, .9f);
    DirectX::SimpleMath::Vector3 treePos5(1.75, 0.0, 0.75);
    DrawTree2(treePos5, .1f);
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1600;
    height = 900;
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // WLJ add for mouse and keybord interface
    m_keyboard = std::make_unique<DirectX::Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    // Audio
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
    #ifdef DEBUG  //#ifdef _DEBUG
    eflags |= AudioEngine_Debug;
    #endif
    m_audioEngine = std::make_unique<AudioEngine>(eflags);
    m_retryAudio = false;
    m_audioBank = std::make_unique<WaveBank>(m_audioEngine.get(), L"audioBank.xwb");
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
    m_kbStateTracker.Reset();
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.

}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_raster.Reset(); // anti-aliased lines

    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();
    m_font.reset();
    m_titleFont.reset();
    m_bitwiseFont.reset();
    m_spriteBatch.reset();
    m_kbStateTracker.Reset();

    //Powerbar
    m_powerFrameTexture.Reset();
    m_powerMeterTexture.Reset();
    m_powerImpactTexture.Reset();
    m_powerBackswingTexture.Reset();

    // Charcter Select
    m_character.reset();
    m_characterTexture.Reset();
    m_character0.reset();
    m_character0Texture.Reset();
    m_character1.reset();
    m_character1Texture.Reset();
    m_character2.reset();
    m_character2Texture.Reset();
    m_characterBackgroundTexture.Reset();

    // Environment Select
    m_environSelectCalmTexture.Reset();
    m_environSelectBreezyTexture.Reset();
    m_environSelectAlienTexture.Reset();

    // Intro Screen
    m_bmwLogoTexture.Reset();
    m_jiLogoTexture.Reset();

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();
    CreateResources();
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
    m_audioEngine->Suspend();
}

void Game::OnResuming()
{
    // TODO: Game is being power-resumed (or returning from minimize).
    m_timer.ResetElapsedTime();
    m_kbStateTracker.Reset();
    m_audioEngine->Resume();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
    pCamera->OnResize(m_outputWidth, m_outputHeight);
    m_proj = pCamera->GetProjectionMatrix();
    m_effect->SetProjection(m_proj);   
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // TODO: Add your rendering code here.
    // WLJ start
    m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
    //m_d3dContext->RSSetState(m_states->CullNone());

    //world start
    m_d3dContext->RSSetState(m_raster.Get()); // WLJ anti-aliasing  RenderTesting
    m_effect->SetWorld(m_world);
    //world end

    m_effect->Apply(m_d3dContext.Get());

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();

    if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
    {
        DrawWorld();
        DrawShotAimCone();
        DrawShotAimArrow();

        if(pCamera->GetCameraState() == CameraState::CAMERASTATE_SWINGVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
        {
            DrawSwing();
        }

        if(pCamera->GetCameraState() == CameraState::CAMERASTATE_PRESWINGVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_FIRSTPERSON)
        {         
            m_flightStepTimer.ResetElapsedTime();
            DrawProjectileRealTime();
            DrawHydraShot();
        }
        if (m_isInDebugMode == true)
        {
            //DrawCameraFocus();
        }
    }

    m_batch->End();

    m_spriteBatch->Begin();

    //DrawShotTimerUI();

    if (m_currentState == GameState::GAMESTATE_INTROSCREEN)
    {
        DrawIntroScreen();
    }
    if (m_currentState == GameState::GAMESTATE_STARTSCREEN)
    {
        DrawStartScreen();
    }
    if (m_currentState == GameState::GAMESTATE_MAINMENU)
    {
        DrawMenuMain();
    }
    if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
    {
        DrawMenuCharacterSelect();
    }
    if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
    {
        DrawMenuEnvironmentSelect();
    }
    if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
    {
        //DrawPowerBarUI();
        //DrawSwingUI();
        //DrawUI();
    }

    m_spriteBatch->End();

    Present();
}

void Game::ResetGamePlay()
{
    pGolf->ZeroUIandRenderData();
    pPlay->ResetPlayData();
    ResetPowerMeter();
    m_projectileTimer = 0;
    m_swingPathStep = 0;
    m_projectilePathStep = 0;
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    m_flightStepTimer.Tick([&]()
        {
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& aTimer)
{
    double elapsedTime = double(aTimer.GetElapsedSeconds());
    m_projectileTimer += elapsedTime;
    // TODO: Add your game logic here.

    if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
    {
        if (m_menuSelect == 0)
        {
            m_character0->Update(static_cast<float>(elapsedTime));
        }
        if (m_menuSelect == 1)
        {
            m_character1->Update(static_cast<float>(elapsedTime));
        }
        if (m_menuSelect == 2)
        {
            m_character2->Update(static_cast<float>(elapsedTime));
        }
    }

    if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
    {
        pPlay->Swing();

        if (pPlay->UpdateSwing() == true)
        {
            AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK::XACT_WAVEBANK_AUDIOBANK_IMPACTSFX1);

            pPlay->ResetSwingUpdateReady();
            pGolf->UpdateImpact(pPlay->GetImpactData());
        }
    }

    // audio
    if (m_retryAudio)
    {
        m_retryAudio = false;
        if (m_audioEngine->Reset())
        {
            // ToDo: restart any looped sounds here
            if (m_audioMusicStream)
            {
                m_audioMusicStream->Play(true);
            }
            if (m_audioEffectStream)
            {
                m_audioEffectStream->Play(); // WLJ this could lead to problems and might not be needed, maybe cause unwanted effect to play after reset?
            }
        }
    }
    else if (!m_audioEngine->Update())
    {
        if (m_audioEngine->IsCriticalError())
        {
            m_retryAudio = true;
        }
    }
    
    pCamera->UpdateCamera(aTimer);
    m_effect->SetView(pCamera->GetViewMatrix());
    
    UpdateInput(aTimer);
}

void Game::UpdateInput(DX::StepTimer const& aTimer)
{
    // WLJ add for mouse and keybord interface   
    auto kb = m_keyboard->GetState();
    m_kbStateTracker.Update(kb);

    if (kb.Escape)
    {
        if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
        {
            pCamera->SetCameraState(CameraState::CAMERASTATE_PRESWINGVIEW);
            ResetGamePlay();
        }
        m_currentState = GameState::GAMESTATE_MAINMENU;
    }
    if (m_kbStateTracker.pressed.Enter)
    {
        if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            if (m_menuSelect == 0)
            {
                pGolf->LoadEnvironment(0);
            }
            if (m_menuSelect == 1)
            {
                pGolf->LoadEnvironment(1);
            }
            if (m_menuSelect == 2)
            {
                pGolf->LoadEnvironment(2);
            }
            m_menuSelect = 0;
            m_currentState = GameState::GAMESTATE_STARTSCREEN;
        }
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            if (m_menuSelect == 0)
            {
                pGolf->SetCharacter(0);
            }
            if (m_menuSelect == 1)
            {
                pGolf->SetCharacter(1);
            }
            if (m_menuSelect == 2)
            {
                pGolf->SetCharacter(2);
            }
            m_menuSelect = 0;
            //m_currentState = GameState::GAMESTATE_MAINMENU; // Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
            m_currentState = GameState::GAMESTATE_STARTSCREEN;// Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
        }
        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            if (m_menuSelect == 0) // GoTo Game State
            {
                m_currentState = GameState::GAMESTATE_GAMEPLAY;
            }
            if (m_menuSelect == 1) // GoTo Character Select State
            {
                m_currentState = GameState::GAMESTATE_CHARACTERSELECT;
            }
            if (m_menuSelect == 2) // GoTo Environment Select State
            {
                m_currentState = GameState::GAMESTATE_ENVIRONTMENTSELECT;
            }
            if (m_menuSelect == 3) // GoTo Game State Hydra Shot Style! (aka just goes into game, yea its for show)
            {
                m_currentState = GameState::GAMESTATE_GAMEPLAY;
            }
            //if (m_menuSelect == 3) // Quit Game
            if (m_menuSelect == 4) // Quit Game
            {
                ExitGame();
            }
            m_menuSelect = 0;
        }
        if (m_currentState == GameState::GAMESTATE_STARTSCREEN)
        {
            m_currentState = GameState::GAMESTATE_MAINMENU;
        }
    }
    if (m_kbStateTracker.pressed.Up)
    {
        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            --m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Down)
    {
        if (m_currentState == GameState::GAMESTATE_MAINMENU)
        {
            ++m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            ++m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Left)
    {
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Right)
    {
        if (m_currentState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
        if (m_currentState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            ++m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.D1)
    {
        pGolf->SelectInputClub(1);
    }
    if (m_kbStateTracker.pressed.D2)
    {
        pGolf->SelectInputClub(2);
    }
    if (m_kbStateTracker.pressed.D3)
    {
        pGolf->SelectInputClub(3);
    }
    if (m_kbStateTracker.pressed.D4)
    {
        pGolf->SelectInputClub(4);
    }
    if (m_kbStateTracker.pressed.D5)
    {
        pGolf->SelectInputClub(5);
    }
    if (m_kbStateTracker.pressed.D6)
    {
        pGolf->SelectInputClub(6);
    }
    if (m_kbStateTracker.pressed.D7)
    {
        pGolf->SelectInputClub(7);
    }
    if (m_kbStateTracker.pressed.D8)
    {
        pGolf->SelectInputClub(8);
    }
    if (m_kbStateTracker.pressed.D9)
    {
        pGolf->SelectInputClub(9);
    }
    if (m_kbStateTracker.pressed.D0)
    {
        pGolf->SelectInputClub(10);
    }
    if (m_kbStateTracker.pressed.OemOpenBrackets)
    {
        pGolf->CycleNextClub(false);
    }
    if (m_kbStateTracker.pressed.OemCloseBrackets)
    {
        pGolf->CycleNextClub(true);        
    }
    if (kb.D)
    {
        pCamera->UpdatePos(0.0f + static_cast<float>(aTimer.GetElapsedSeconds()), 0.0f, 0.0f);
    }
    if (kb.S)
    {
        pCamera->UpdatePos(0.0f, 0.0f, 0.0f - static_cast<float>(aTimer.GetElapsedSeconds()));
    }
    if (kb.A)
    {
        pCamera->UpdatePos(0.0f - static_cast<float>(aTimer.GetElapsedSeconds()), 0.0f, 0.0f);
    }
    if (kb.W)
    {
        pCamera->UpdatePos(0.0f, 0.0f, 0.0f + static_cast<float>(aTimer.GetElapsedSeconds()));
    }
    if (kb.Q)
    {
        pCamera->UpdatePitchYaw(0.0f, 0.0f + static_cast<float>(aTimer.GetElapsedSeconds()));
    }
    if (kb.E)
    {
        pCamera->UpdatePitchYaw(0.0f, 0.0f - static_cast<float>(aTimer.GetElapsedSeconds()));
    }
    if (kb.F)
    {
        pCamera->UpdatePos(0.0f, 0.0f + static_cast<float>(aTimer.GetElapsedSeconds()), 0.0f);
    }
    if (kb.C)
    {
        pCamera->UpdatePos(0.0f, 0.0f - static_cast<float>(aTimer.GetElapsedSeconds()), 0.0f);
    }
    if (m_kbStateTracker.pressed.V) // reset ball to tee position and prep for new shot
    {
        pGolf->SetBallPosition(pGolf->GetTeePos());
        pGolf->SetShotStartPos(pGolf->GetBallPosition());
        pCamera->SetCameraStartPos(pCamera->GetPos());
        pCamera->SetCameraEndPos(pCamera->GetPreSwingCamPos(pGolf->GetTeePos(), pGolf->GetTeeDirection()));
        pCamera->SetTargetStartPos(pCamera->GetTargetPos());
        pCamera->SetTargetEndPos(pCamera->GetPreSwingTargPos(pGolf->GetTeePos(), pGolf->GetTeeDirection()));
        pCamera->SetCameraState(CameraState::CAMERASTATE_RESET);
        ResetGamePlay();
    }
    if (m_kbStateTracker.pressed.B) // move cameras to new ball position and prep for next shot
    {
        pGolf->SetShotStartPos(pGolf->GetBallPosition());
        pCamera->SetCameraStartPos(pCamera->GetPos());
        pCamera->SetCameraEndPos(pCamera->GetPreSwingCamPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
        pCamera->SetTargetStartPos(pCamera->GetTargetPos());
        pCamera->SetTargetEndPos(pCamera->GetPreSwingTargPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
        pCamera->SetCameraState(CameraState::CAMERASTATE_TRANSTONEWSHOT);
        ResetGamePlay();
    }
    if (m_kbStateTracker.pressed.Space)
    {
        if (m_currentState == GameState::GAMESTATE_GAMEPLAY)
        {
            if (pPlay->IsSwingStateAtImpact() == true)
            {
                pCamera->SetCameraStartPos(pCamera->GetPos());
                pCamera->SetCameraEndPos(pCamera->GetSwingCamPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
                pCamera->SetTargetStartPos(pCamera->GetTargetPos());
                pCamera->SetTargetEndPos(pCamera->GetSwingTargPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
                pCamera->TurnEndPosAroundPoint(Utility::ToRadians(pPlay->GetShotDirection()), pGolf->GetShotStartPos());
                pCamera->SetCameraState(CameraState::CAMERASTATE_TRANSITION);
            }
            pPlay->UpdateSwingState();
        }
    }
    if (m_kbStateTracker.released.Z)
    {
        m_kbStateTracker.Reset();
        m_projectileTimer = 0.0;
        pPlay->DebugShot();
    }
    if (m_kbStateTracker.released.H)
    {        
        m_kbStateTracker.Reset();
        m_projectileTimer = 0.0;
        pGolf->BuildHyrdraShotData(pPlay->GetShotDirection());
    }

    if (m_kbStateTracker.pressed.T)
    {
        pCamera->SetCameraState(CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW);
    }
    if (m_kbStateTracker.pressed.Y)
    {
        pCamera->SetCameraState(CameraState::CAMERASTATE_FIRSTPERSON);
    }
    if (m_kbStateTracker.pressed.U)
    {
        pCamera->SetCameraState(CameraState::CAMERASTATE_TRANSITION);
    }
    if (m_kbStateTracker.pressed.F1)
    {
        pCamera->SetCameraState(CameraState::CAMERASTATE_SWINGVIEW);
    }
    if (kb.OemPeriod)
    {
        pPlay->TurnShotAim(static_cast<float>(-aTimer.GetElapsedSeconds()), pCamera->GetAimTurnRate());
        pCamera->TurnAroundPoint(static_cast<float>(-aTimer.GetElapsedSeconds()), pGolf->GetShotStartPos());
    }
    if (kb.OemComma)
    {
        pPlay->TurnShotAim(static_cast<float>(aTimer.GetElapsedSeconds()), pCamera->GetAimTurnRate());
        pCamera->TurnAroundPoint(static_cast<float>(aTimer.GetElapsedSeconds()), pGolf->GetShotStartPos());    
    }
    if (m_kbStateTracker.pressed.X) // Debug to turn aim at a set rate
    {
        const double turnAngle = 90.0;
        const float turnRate = 0.0174532862792735;

        pPlay->TurnShotAim(turnAngle, turnRate);
        pCamera->TurnAroundPoint(Utility::ToRadians(turnAngle), pGolf->GetShotStartPos());
    }
    if (m_kbStateTracker.pressed.N)
    {
        pGolf->ZeroUIandRenderData();
        pGolf->ResetBallData();
    }

    auto mouse = m_mouse->GetState();

    if (pCamera->GetCameraState() == CameraState::CAMERASTATE_FIRSTPERSON)
    {
        if (mouse.positionMode == Mouse::MODE_RELATIVE)
        {
            const float ROTATION_GAIN = 0.004f;
            DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(float(mouse.x), float(mouse.y), 0.f) * ROTATION_GAIN;

            float pitch = -delta.y;
            float yaw = -delta.x;

            pCamera->UpdatePitchYaw(pitch, yaw);
        }

        m_mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    }
}
