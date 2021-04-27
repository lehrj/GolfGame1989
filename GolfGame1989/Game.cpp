//
// Game.cpp
//
#include <stdlib.h>  // for random numbers
#include <time.h>    // timer for random numbers

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
    srand(time(NULL));
    pGolf = new Golf;
    pPlay = new GolfPlay;

    pCamera = new Camera(m_outputWidth, m_outputHeight);
    pCamera->InintializePreSwingCamera(pGolf->GetTeePos(), pGolf->GetTeeDirection());
    pLighting = new Lighting();
    pLighting->SetLighting(Lighting::LightingState::LIGHTINGSTATE_NULL);
    //pLighting->SetLighting(Lighting::LightingState::LIGHTINGSTATE_TESTSUNMOVE);

    if (m_isInDebugMode == false)
    {
        m_currentGameState = GameState::GAMESTATE_INTROSCREEN;
    }
    else
    {
        m_currentGameState = GameState::GAMESTATE_GAMEPLAY;
    }

    //m_currentGameState = GameState::GAMESTATE_INTROSCREEN;
    m_currentUiState = UiState::UISTATE_SWING;
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
    delete pLighting;

    delete[] m_terrainVertexArray;
    m_terrainVertexArray = 0;
    delete[] m_terrainVertexArrayBase;
    m_terrainVertexArrayBase = 0;

    delete[] m_terrainVertexArray2;
    m_terrainVertexArray2 = 0;
    delete[] m_terrainVertexArrayBase2;
    m_terrainVertexArrayBase2 = 0;
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

    /*
    // Lighting effect and batch
    m_effectNormColorLighting = std::make_unique<DirectX::BasicEffect>(m_d3dDevice.Get());
    //m_effectNormColorLighting = std::make_unique<DirectX::NormalMapEffect>(m_d3dDevice.Get());
    m_effectNormColorLighting->SetVertexColorEnabled(true);

    void const* shaderByteCodeLighting;
    size_t byteCodeLengthLighting;

    m_effectNormColorLighting->GetVertexShaderBytecode(&shaderByteCodeLighting, &byteCodeLengthLighting);
    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexTypeLighting::InputElements, VertexTypeLighting::InputElementCount, shaderByteCodeLighting, byteCodeLengthLighting, m_inputLayout.ReleaseAndGetAddressOf()));
    m_batchNormColorLighting = std::make_unique<PrimitiveBatch<VertexTypeLighting>>(m_d3dContext.Get());
    */

    m_effect2 = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    
    //m_effect2->SetLightingEnabled(true);
    m_effect2->SetLightEnabled(0, true);
    m_effect2->SetLightDiffuseColor(0, Colors::White);
    m_effect2->SetLightDirection(0, -DirectX::SimpleMath::Vector3::UnitY);
    m_effect2->SetVertexColorEnabled(true);
    m_effect2->EnableDefaultLighting();
    m_effect2->SetLightDiffuseColor(0, Colors::Gray);
    


    void const* shaderByteCode2;
    size_t byteCodeLength2;
    m_effect2->GetVertexShaderBytecode(&shaderByteCode2, &byteCodeLength2);
    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType2::InputElements, VertexType2::InputElementCount, shaderByteCode2, byteCodeLength2, m_inputLayout.ReleaseAndGetAddressOf()));
    m_batch2 = std::make_unique<PrimitiveBatch<VertexType2>>(m_d3dContext.Get());




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

    // Shape for skydome
    //m_shape = GeometricPrimitive::CreateSphere(m_d3dContext.Get());
    //m_shape = GeometricPrimitive::CreateSphere(m_d3dContext.Get(), 100, false);
    //m_shape = GeometricPrimitive::CreateSphere(m_d3dContext.Get(), 3, true, true);
    //m_shape = GeometricPrimitive::CreateBox(m_d3dContext.Get(), XMFLOAT3(10, 10, 10), false, false);

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

    ////********* WLJ world start ----- 
    m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);

    // Hook values into the camera class variables for uniform view field when switching window size/fullscreen
    const float viewPlaneNear = 0.0001f;  
    const float viewPlaneFar = 9.0f;
    m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), viewPlaneNear, viewPlaneFar);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);

    m_effect2->SetView(m_view);
    m_effect2->SetProjection(m_proj);
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

    // heightmap load
    //pTerrain->Initialize(m_d3dDevice.Get(), "heightmap01.bmp");
    //pTerrain->Initialize(m_d3dDevice.Get(), "setup.txt");
    //pZone->Initialize(m_d3dDevice.Get());
}

void Game::DrawBridge(const DirectX::SimpleMath::Vector3 aPos, const float aRotation)
{
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(aRotation);
    DirectX::SimpleMath::Vector3 origin = aPos;
    origin.y -= .04;
    DirectX::XMVECTORF32 bridgeColor1 = DirectX::Colors::LightSlateGray;
    DirectX::XMVECTORF32 bridgeColor2 = DirectX::Colors::DarkSlateGray;
    DirectX::XMVECTORF32 bridgeColor3 = DirectX::Colors::White;
    DirectX::XMVECTORF32 bridgeColor4 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 bridgeColor5 = DirectX::Colors::Red;

    const float width = .06;
    const float length = .6;
    const float halfLength = length * .5;
    const float quarterlength = halfLength * .6;
    const float height = .08;
    const float quarterHeight = height * .7;
    const float frontBaseLength = quarterlength * .6;
    const float secondBaseLength = halfLength - (quarterlength * .3);
    const float thridBaseLength = halfLength + (quarterlength * .3);
    const float backBaseLength = length - (quarterlength * .6);

    DirectX::SimpleMath::Vector3 nw(length, 0.0, -width);
    nw = DirectX::SimpleMath::Vector3::Transform(nw, rotMat) + origin;
    DirectX::SimpleMath::Vector3 ne(length, 0.0, width);
    ne = DirectX::SimpleMath::Vector3::Transform(ne, rotMat) + origin;
    DirectX::SimpleMath::Vector3 se(.0, 0.0, width);
    se = DirectX::SimpleMath::Vector3::Transform(se, rotMat) + origin;
    DirectX::SimpleMath::Vector3 sw(.0, 0.0, -width);
    sw = DirectX::SimpleMath::Vector3::Transform(sw, rotMat) + origin;

    DirectX::SimpleMath::Vector3 midLeftTop(halfLength, height, -width);
    midLeftTop = DirectX::SimpleMath::Vector3::Transform(midLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 midRightTop(halfLength, height, width);
    midRightTop = DirectX::SimpleMath::Vector3::Transform(midRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 quarterLeftTop(quarterlength, quarterHeight, -width);
    quarterLeftTop = DirectX::SimpleMath::Vector3::Transform(quarterLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 quarterRightTop(quarterlength, quarterHeight, width);
    quarterRightTop = DirectX::SimpleMath::Vector3::Transform(quarterRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 backQuarterLeftTop(length - quarterlength, quarterHeight, -width);
    backQuarterLeftTop = DirectX::SimpleMath::Vector3::Transform(backQuarterLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 backQuarterRightTop(length - quarterlength, quarterHeight, width);
    backQuarterRightTop = DirectX::SimpleMath::Vector3::Transform(backQuarterRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 frontBaseLeft(frontBaseLength, 0.0, -width);
    frontBaseLeft = DirectX::SimpleMath::Vector3::Transform(frontBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 frontBaseRight(frontBaseLength, 0.0, width);
    frontBaseRight = DirectX::SimpleMath::Vector3::Transform(frontBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 secondBaseLeft(secondBaseLength, 0.0, -width);
    secondBaseLeft = DirectX::SimpleMath::Vector3::Transform(secondBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 secondBaseRight(secondBaseLength, 0.0, width);
    secondBaseRight = DirectX::SimpleMath::Vector3::Transform(secondBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 thirdBaseLeft(thridBaseLength, 0.0, -width);
    thirdBaseLeft = DirectX::SimpleMath::Vector3::Transform(thirdBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 thirdBaseRight(thridBaseLength, 0.0, width);
    thirdBaseRight = DirectX::SimpleMath::Vector3::Transform(thirdBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 backBaseLeft(backBaseLength, 0.0, -width);
    backBaseLeft = DirectX::SimpleMath::Vector3::Transform(backBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 backBaseRight(backBaseLength, 0.0, width);
    backBaseRight = DirectX::SimpleMath::Vector3::Transform(backBaseRight, rotMat) + origin;

    VertexPositionColor endLeft(nw, bridgeColor1);
    VertexPositionColor endRight(ne, bridgeColor1);
    VertexPositionColor lowerRight(se, bridgeColor1);
    VertexPositionColor lowerLeft(sw, bridgeColor1);

    VertexPositionColor halfTopLeft(midLeftTop, bridgeColor1);
    VertexPositionColor halfTopRight(midRightTop, bridgeColor1);
    // under
    VertexPositionColor underHalfTopLeft(midLeftTop, bridgeColor4);
    VertexPositionColor underHalfTopRight(midRightTop, bridgeColor4);

    VertexPositionColor quarterTopLeft(quarterLeftTop, bridgeColor1);
    VertexPositionColor quarterTopRight(quarterRightTop, bridgeColor1);
    // under
    VertexPositionColor underQuarterTopLeft(quarterLeftTop, bridgeColor4);
    VertexPositionColor underQuarterTopRight(quarterRightTop, bridgeColor4);

    VertexPositionColor backQuarterTopLeft(backQuarterLeftTop, bridgeColor1);
    VertexPositionColor backQuarterTopRight(backQuarterRightTop, bridgeColor1);
    // under
    VertexPositionColor underBackQuarterTopLeft(backQuarterLeftTop, bridgeColor4);
    VertexPositionColor underBackQuarterTopRight(backQuarterRightTop, bridgeColor4);

    VertexPositionColor frontLeftBase(frontBaseLeft, bridgeColor1);
    VertexPositionColor frontRightBase(frontBaseRight, bridgeColor1);
    // under
    VertexPositionColor underFrontLeftBase(frontBaseLeft, bridgeColor2);
    VertexPositionColor underFrontRightBase(frontBaseRight, bridgeColor2);

    VertexPositionColor secondLeftBase(secondBaseLeft, bridgeColor1);
    VertexPositionColor secondRightBase(secondBaseRight, bridgeColor1);
    //under
    VertexPositionColor underSecondLeftBase(secondBaseLeft, bridgeColor2);
    VertexPositionColor underSecondRightBase(secondBaseRight, bridgeColor2);

    VertexPositionColor thirdLeftBase(thirdBaseLeft, bridgeColor1);
    VertexPositionColor thirdRightBase(thirdBaseRight, bridgeColor1);
    //under
    VertexPositionColor underThirdLeftBase(thirdBaseLeft, bridgeColor2);
    VertexPositionColor UnderThirdRightBase(thirdBaseRight, bridgeColor2);

    VertexPositionColor backLeftBase(backBaseLeft, bridgeColor1);
    VertexPositionColor backRightBase(backBaseRight, bridgeColor1);
    //under
    VertexPositionColor underBackLeftBase(backBaseLeft, bridgeColor2);
    VertexPositionColor underBackRightBase(backBaseRight, bridgeColor2);

    // railings
    VertexPositionColor leftRailing1(sw, bridgeColor3);
    VertexPositionColor leftRailing2(quarterLeftTop, bridgeColor3);
    VertexPositionColor leftRailing3(midLeftTop, bridgeColor3);
    VertexPositionColor leftRailing4(backQuarterLeftTop, bridgeColor3);
    VertexPositionColor leftRailing5(nw, bridgeColor3);

    VertexPositionColor rightRailing1(se, bridgeColor3);
    VertexPositionColor rightRailing2(quarterRightTop, bridgeColor3);
    VertexPositionColor rightRailing3(midRightTop, bridgeColor3);
    VertexPositionColor rightRailing4(backQuarterRightTop, bridgeColor3);
    VertexPositionColor rightRailing5(ne, bridgeColor3);

    // draw undersides
    m_batch->DrawQuad(underFrontLeftBase, underFrontRightBase, underQuarterTopRight, underQuarterTopLeft);
    m_batch->DrawQuad(underSecondLeftBase, underSecondRightBase, underQuarterTopRight, underQuarterTopLeft);
    m_batch->DrawQuad(underSecondLeftBase, underSecondRightBase, underHalfTopRight, underHalfTopLeft);
    m_batch->DrawQuad(underThirdLeftBase, UnderThirdRightBase, underHalfTopRight, underHalfTopLeft);

    m_batch->DrawQuad(underThirdLeftBase, UnderThirdRightBase, underBackQuarterTopRight, underBackQuarterTopLeft);
    m_batch->DrawQuad(underBackLeftBase, underBackRightBase, underBackQuarterTopRight, underBackQuarterTopLeft);

    m_batch->DrawQuad(lowerLeft, lowerRight, quarterTopRight, quarterTopLeft);
    m_batch->DrawQuad(quarterTopLeft, quarterTopRight, halfTopRight, halfTopLeft);
    m_batch->DrawQuad(halfTopLeft, halfTopRight, backQuarterTopRight, backQuarterTopLeft);
    m_batch->DrawQuad(endLeft, endRight, backQuarterTopRight, backQuarterTopLeft);

    // sides;
    m_batch->DrawTriangle(lowerLeft, frontLeftBase, quarterTopLeft);
    m_batch->DrawTriangle(lowerRight, frontRightBase, quarterTopRight);

    m_batch->DrawTriangle(secondLeftBase, halfTopLeft, quarterTopLeft);
    m_batch->DrawTriangle(secondRightBase, halfTopRight, quarterTopRight);

    m_batch->DrawTriangle(thirdLeftBase, halfTopLeft, backQuarterTopLeft);
    m_batch->DrawTriangle(thirdRightBase, halfTopRight, backQuarterTopRight);

    m_batch->DrawTriangle(backLeftBase, backQuarterTopLeft, endLeft);
    m_batch->DrawTriangle(backRightBase, backQuarterTopRight, endRight);

    // railings
    m_batch->DrawLine(leftRailing1, leftRailing2);
    m_batch->DrawLine(leftRailing2, leftRailing3);
    m_batch->DrawLine(leftRailing3, leftRailing4);
    m_batch->DrawLine(leftRailing4, leftRailing5);

    m_batch->DrawLine(rightRailing1, rightRailing2);
    m_batch->DrawLine(rightRailing2, rightRailing3);
    m_batch->DrawLine(rightRailing3, rightRailing4);
    m_batch->DrawLine(rightRailing4, rightRailing5);
}

void Game::DrawBridgeTest2(const DirectX::SimpleMath::Vector3 aPos, const float aRotation)
{
    ////////// testing angle func

    DirectX::SimpleMath::Vector3 tempNorm = DirectX::SimpleMath::Vector3::UnitY;

    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(aRotation);
    DirectX::SimpleMath::Matrix rotMat2 = DirectX::SimpleMath::Matrix::CreateRotationY(aRotation + Utility::ToRadians(180.0));
    DirectX::SimpleMath::Vector3 origin = aPos;
    origin.y -= .04;
    DirectX::XMVECTORF32 bridgeColor1 = DirectX::Colors::LightSlateGray;
    DirectX::XMVECTORF32 bridgeColor2 = DirectX::Colors::DarkSlateGray;
    DirectX::XMVECTORF32 bridgeColor3 = DirectX::Colors::White;
    DirectX::XMVECTORF32 bridgeColor4 = DirectX::Colors::Black;
    bridgeColor4 = bridgeColor2;
    DirectX::XMVECTORF32 bridgeColor5 = DirectX::Colors::Red;
    DirectX::XMVECTORF32 bridgeColorTop = DirectX::Colors::Green;

    const float width = .06;
    const float length = .6;
    const float halfLength = length * .5;
    const float quarterlength = halfLength * .6;
    const float height = .08;
    const float quarterHeight = height * .7;
    const float frontBaseLength = quarterlength * .6;
    const float secondBaseLength = halfLength - (quarterlength * .3);
    const float thridBaseLength = halfLength + (quarterlength * .3);
    const float backBaseLength = length - (quarterlength * .6);

    DirectX::SimpleMath::Vector3 nw(length, 0.0, -width);
    nw = DirectX::SimpleMath::Vector3::Transform(nw, rotMat) + origin;
    DirectX::SimpleMath::Vector3 ne(length, 0.0, width);
    ne = DirectX::SimpleMath::Vector3::Transform(ne, rotMat) + origin;
    DirectX::SimpleMath::Vector3 se(.0, 0.0, width);
    se = DirectX::SimpleMath::Vector3::Transform(se, rotMat) + origin;
    DirectX::SimpleMath::Vector3 sw(.0, 0.0, -width);
    sw = DirectX::SimpleMath::Vector3::Transform(sw, rotMat) + origin;

    DirectX::SimpleMath::Vector3 midLeftTop(halfLength, height, -width);
    midLeftTop = DirectX::SimpleMath::Vector3::Transform(midLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 midRightTop(halfLength, height, width);
    midRightTop = DirectX::SimpleMath::Vector3::Transform(midRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 quarterLeftTop(quarterlength, quarterHeight, -width);
    quarterLeftTop = DirectX::SimpleMath::Vector3::Transform(quarterLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 quarterRightTop(quarterlength, quarterHeight, width);
    quarterRightTop = DirectX::SimpleMath::Vector3::Transform(quarterRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 backQuarterLeftTop(length - quarterlength, quarterHeight, -width);
    backQuarterLeftTop = DirectX::SimpleMath::Vector3::Transform(backQuarterLeftTop, rotMat) + origin;
    DirectX::SimpleMath::Vector3 backQuarterRightTop(length - quarterlength, quarterHeight, width);
    backQuarterRightTop = DirectX::SimpleMath::Vector3::Transform(backQuarterRightTop, rotMat) + origin;

    DirectX::SimpleMath::Vector3 frontBaseLeft(frontBaseLength, 0.0, -width);
    frontBaseLeft = DirectX::SimpleMath::Vector3::Transform(frontBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 frontBaseRight(frontBaseLength, 0.0, width);
    frontBaseRight = DirectX::SimpleMath::Vector3::Transform(frontBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 secondBaseLeft(secondBaseLength, 0.0, -width);
    secondBaseLeft = DirectX::SimpleMath::Vector3::Transform(secondBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 secondBaseRight(secondBaseLength, 0.0, width);
    secondBaseRight = DirectX::SimpleMath::Vector3::Transform(secondBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 thirdBaseLeft(thridBaseLength, 0.0, -width);
    thirdBaseLeft = DirectX::SimpleMath::Vector3::Transform(thirdBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 thirdBaseRight(thridBaseLength, 0.0, width);
    thirdBaseRight = DirectX::SimpleMath::Vector3::Transform(thirdBaseRight, rotMat) + origin;

    DirectX::SimpleMath::Vector3 backBaseLeft(backBaseLength, 0.0, -width);
    backBaseLeft = DirectX::SimpleMath::Vector3::Transform(backBaseLeft, rotMat) + origin;
    DirectX::SimpleMath::Vector3 backBaseRight(backBaseLength, 0.0, width);
    backBaseRight = DirectX::SimpleMath::Vector3::Transform(backBaseRight, rotMat) + origin;

    // Setup normals
    DirectX::SimpleMath::Vector3 normUndersidePos = DirectX::SimpleMath::Vector3::UnitX;
    normUndersidePos = DirectX::SimpleMath::Vector3::Transform(normUndersidePos, rotMat2);
    DirectX::SimpleMath::Vector3 normUndersideNeg = -DirectX::SimpleMath::Vector3::UnitX;
    normUndersideNeg = DirectX::SimpleMath::Vector3::Transform(normUndersideNeg, rotMat2);
    DirectX::SimpleMath::Vector3 normUndersideDown = -DirectX::SimpleMath::Vector3::UnitY;

    DirectX::SimpleMath::Vector3 normSideLeft = DirectX::SimpleMath::Vector3::UnitZ;
    normSideLeft = DirectX::SimpleMath::Vector3::Transform(normSideLeft, rotMat2);
    DirectX::SimpleMath::Vector3 normSideRight = - DirectX::SimpleMath::Vector3::UnitZ;
    normSideRight = DirectX::SimpleMath::Vector3::Transform(normSideRight, rotMat2);

    //DirectX::SimpleMath::Vector3 normQuarterPos = DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3 normQuarterPos(quarterlength, quarterHeight, 0.0);
    normQuarterPos = DirectX::SimpleMath::Vector3::Transform(normQuarterPos, rotMat2);
    normQuarterPos.Normalize();
    DirectX::SimpleMath::Vector3 normHalfPos(halfLength - quarterlength, height - quarterHeight, 0.0);
    normHalfPos = DirectX::SimpleMath::Vector3::Transform(normHalfPos, rotMat2);
    normHalfPos.Normalize();

    DirectX::SimpleMath::Matrix flipRotMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::GetPi());
    DirectX::SimpleMath::Vector3 normQuarterNeg = DirectX::SimpleMath::Vector3::Transform(normQuarterPos, rotMat);
    DirectX::SimpleMath::Vector3 normHalfNeg = DirectX::SimpleMath::Vector3::Transform(normHalfPos, rotMat);

    DirectX::SimpleMath::Vector3 testSurfaceNorm = GetSurfaceNormal(sw, se, quarterRightTop);
    DirectX::SimpleMath::Vector3 testSurfaceNorm2 = GetSurfaceNormal(quarterRightTop, se, sw);
    normQuarterPos = testSurfaceNorm2;

    DirectX::SimpleMath::Vector3 testSurfaceNorm3 = GetSurfaceNormal(midRightTop, quarterLeftTop, midRightTop);
    DirectX::SimpleMath::Vector3 testSurfaceNorm3a = GetSurfaceNormal(midRightTop, quarterLeftTop, quarterRightTop);
    normHalfPos = testSurfaceNorm3a;
    normHalfPos = DirectX::SimpleMath::Vector3::UnitY;

    DirectX::SimpleMath::Matrix halfSpinMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::GetPi());



    DirectX::SimpleMath::Vector3 testSurfaceNorm4 = GetSurfaceNormal(midRightTop, midLeftTop, backQuarterRightTop);
    normHalfNeg = testSurfaceNorm4;


    normHalfPos = DirectX::SimpleMath::Vector3::Transform(normHalfNeg, rotMat);



    DirectX::SimpleMath::Vector3 testSurfaceNorm5 = GetSurfaceNormal(backQuarterRightTop, backQuarterLeftTop, nw);
    normQuarterNeg = testSurfaceNorm5;

    VertexPositionNormalColor endLeft(nw, normQuarterNeg, bridgeColorTop);
    VertexPositionNormalColor endRight(ne, normQuarterNeg, bridgeColorTop);
    VertexPositionNormalColor endLeftSide(nw, normSideLeft, bridgeColor1);
    VertexPositionNormalColor endRightSide(ne, normSideRight, bridgeColor1);
    VertexPositionNormalColor lowerLeft(sw, normQuarterPos, bridgeColorTop);
    VertexPositionNormalColor lowerRight(se, normQuarterPos, bridgeColorTop);
    VertexPositionNormalColor lowerLeftSide(sw, normSideLeft, bridgeColor1);
    VertexPositionNormalColor lowerRightSide(se, normSideRight, bridgeColor1);

    VertexPositionNormalColor halfTopLeft(midLeftTop, normHalfPos, bridgeColorTop);
    VertexPositionNormalColor halfTopRight(midRightTop, normHalfPos, bridgeColorTop);
    VertexPositionNormalColor halfTopLeftSide(midLeftTop, normSideLeft, bridgeColor1);
    VertexPositionNormalColor halfTopRightSide(midRightTop, normSideRight, bridgeColor1);
    // under
    VertexPositionNormalColor underHalfTopLeft(midLeftTop, normUndersideDown, bridgeColor4);
    VertexPositionNormalColor underHalfTopRight(midRightTop, normUndersideDown, bridgeColor4);

    VertexPositionNormalColor quarterTopLeft(quarterLeftTop, normQuarterPos, bridgeColorTop);
    VertexPositionNormalColor quarterTopRight(quarterRightTop, normQuarterPos, bridgeColorTop);
    VertexPositionNormalColor quarterTopLeftSide(quarterLeftTop, normSideLeft, bridgeColor1);
    VertexPositionNormalColor quarterTopRightSide(quarterRightTop, normSideRight, bridgeColor1);
    // under
    VertexPositionNormalColor underQuarterTopLeft(quarterLeftTop, normUndersideDown, bridgeColor4);
    VertexPositionNormalColor underQuarterTopRight(quarterRightTop, normUndersideDown, bridgeColor4);

    VertexPositionNormalColor backQuarterTopLeft(backQuarterLeftTop, normHalfNeg, bridgeColorTop);
    VertexPositionNormalColor backQuarterTopRight(backQuarterRightTop, normHalfNeg, bridgeColorTop);
    VertexPositionNormalColor backQuarterTopLeftSide(backQuarterLeftTop, normSideLeft, bridgeColor1);
    VertexPositionNormalColor backQuarterTopRightSide(backQuarterRightTop, normSideRight, bridgeColor1);
    // under
    VertexPositionNormalColor underBackQuarterTopLeft(backQuarterLeftTop, normUndersideDown, bridgeColor4);
    VertexPositionNormalColor underBackQuarterTopRight(backQuarterRightTop, normUndersideDown, bridgeColor4);

    VertexPositionNormalColor frontLeftBase(frontBaseLeft, tempNorm, bridgeColorTop);
    VertexPositionNormalColor frontRightBase(frontBaseRight, tempNorm, bridgeColorTop);
    VertexPositionNormalColor frontLeftBaseSide(frontBaseLeft, normSideLeft, bridgeColor1);
    VertexPositionNormalColor frontRightBaseSide(frontBaseRight, normSideRight, bridgeColor1);
    // under
    VertexPositionNormalColor underFrontLeftBase(frontBaseLeft, normUndersideNeg, bridgeColor2);
    VertexPositionNormalColor underFrontRightBase(frontBaseRight, normUndersideNeg, bridgeColor2);
    VertexPositionNormalColor underFrontLeftBaseNeg(frontBaseLeft, normUndersidePos, bridgeColor5);
    VertexPositionNormalColor underFrontRightBaseNeg(frontBaseRight, normUndersidePos, bridgeColor5);

    VertexPositionNormalColor secondLeftBase(secondBaseLeft, normUndersidePos, bridgeColor1);
    VertexPositionNormalColor secondRightBase(secondBaseRight, normUndersidePos, bridgeColor1);
    VertexPositionNormalColor secondLeftBaseSide(secondBaseLeft, normSideLeft, bridgeColor1);
    VertexPositionNormalColor secondRightBaseSide(secondBaseRight, normSideRight, bridgeColor1);
    //under
    VertexPositionNormalColor underSecondLeftBase(secondBaseLeft, normUndersidePos, bridgeColor2);
    VertexPositionNormalColor underSecondRightBase(secondBaseRight, normUndersidePos, bridgeColor2);
    VertexPositionNormalColor underSecondLeftBaseNeg(secondBaseLeft, normUndersideNeg, bridgeColor2);
    VertexPositionNormalColor underSecondRightBaseNeg(secondBaseRight, normUndersideNeg, bridgeColor2);

    VertexPositionNormalColor thirdLeftBase(thirdBaseLeft, tempNorm, bridgeColor1);
    VertexPositionNormalColor thirdRightBase(thirdBaseRight, tempNorm, bridgeColor1);
    VertexPositionNormalColor thirdLeftBaseSide(thirdBaseLeft, normSideLeft, bridgeColor1);
    VertexPositionNormalColor thirdRightBaseSide(thirdBaseRight, normSideRight, bridgeColor1);
    //under
    VertexPositionNormalColor underThirdLeftBase(thirdBaseLeft, normUndersideNeg, bridgeColor2);
    VertexPositionNormalColor UnderThirdRightBase(thirdBaseRight, normUndersideNeg, bridgeColor2);
    VertexPositionNormalColor underThirdLeftBasePos(thirdBaseLeft, normUndersidePos, bridgeColor2);
    VertexPositionNormalColor UnderThirdRightBasePos(thirdBaseRight, normUndersidePos, bridgeColor2);


    VertexPositionNormalColor backLeftBase(backBaseLeft, tempNorm, bridgeColor1);
    VertexPositionNormalColor backRightBase(backBaseRight, tempNorm, bridgeColor1);
    VertexPositionNormalColor backLeftBaseSide(backBaseLeft, normSideLeft, bridgeColor1);
    VertexPositionNormalColor backRightBaseSide(backBaseRight, normSideRight, bridgeColor1);
    //under
    VertexPositionNormalColor underBackLeftBase(backBaseLeft, normUndersidePos, bridgeColor2);
    VertexPositionNormalColor underBackRightBase(backBaseRight, normUndersidePos, bridgeColor2);

    // railings
    /*
    VertexPositionNormalColor leftRailing1(sw, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor leftRailing2(quarterLeftTop, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor leftRailing3(midLeftTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor leftRailing4(backQuarterLeftTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor leftRailing5(nw, normQuarterNeg, bridgeColor3);
    */
    VertexPositionNormalColor leftRailing1(sw, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor leftRailing2(quarterLeftTop, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor leftRailing3(quarterLeftTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor leftRailing4(midLeftTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor leftRailing5(midLeftTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor leftRailing6(backQuarterLeftTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor leftRailing7(backQuarterLeftTop, normQuarterNeg, bridgeColor3);
    VertexPositionNormalColor leftRailing8(nw, normQuarterNeg, bridgeColor3);

    /*
    VertexPositionNormalColor rightRailing1(se, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor rightRailing2(quarterRightTop, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor rightRailing3(midRightTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor rightRailing4(backQuarterRightTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor rightRailing5(ne, normQuarterNeg, bridgeColor3);
    */
    VertexPositionNormalColor rightRailing1(se, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor rightRailing2(quarterRightTop, normQuarterPos, bridgeColor3);
    VertexPositionNormalColor rightRailing3(quarterRightTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor rightRailing4(midRightTop, normHalfPos, bridgeColor3);
    VertexPositionNormalColor rightRailing5(midRightTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor rightRailing6(backQuarterRightTop, normHalfNeg, bridgeColor3);
    VertexPositionNormalColor rightRailing7(backQuarterRightTop, normQuarterNeg, bridgeColor3);
    VertexPositionNormalColor rightRailing8(ne, normQuarterNeg, bridgeColor3);

    // draw undersides
    m_batch2->DrawQuad(underFrontLeftBase, underFrontRightBase, underQuarterTopRight, underQuarterTopLeft);
    
    m_batch2->DrawQuad(underSecondLeftBase, underSecondRightBase, underQuarterTopRight, underQuarterTopLeft);
    m_batch2->DrawQuad(underSecondLeftBaseNeg, underSecondRightBaseNeg, underHalfTopRight, underHalfTopLeft);
    m_batch2->DrawQuad(underThirdLeftBasePos, UnderThirdRightBasePos, underHalfTopRight, underHalfTopLeft);
    
    m_batch2->DrawQuad(underThirdLeftBase, UnderThirdRightBase, underBackQuarterTopRight, underBackQuarterTopLeft);
    m_batch2->DrawQuad(underBackLeftBase, underBackRightBase, underBackQuarterTopRight, underBackQuarterTopLeft);

    
    m_batch2->DrawQuad(lowerLeft, lowerRight, quarterTopRight, quarterTopLeft);
    //quarterTopLeft.normal = normHalfPos;
    //quarterTopRight.normal = normHalfPos;
    m_batch2->DrawQuad(quarterTopLeft, quarterTopRight, halfTopRight, halfTopLeft);
    //halfTopRight.normal = normHalfNeg;
    //halfTopLeft.normal = normHalfNeg;
    m_batch2->DrawQuad(halfTopLeft, halfTopRight, backQuarterTopRight, backQuarterTopLeft);
    //backQuarterTopRight.normal = normQuarterNeg;
    //backQuarterTopLeft.normal = normQuarterNeg;
    m_batch2->DrawQuad(endLeft, endRight, backQuarterTopRight, backQuarterTopLeft);
    

    // sides;
    //normSideLeft = - DirectX::SimpleMath::Vector3::UnitY;
    /*
    lowerLeft.normal = normSideLeft;
    frontLeftBase.normal = normSideLeft;
    quarterTopLeft.normal = normSideLeft;
    secondLeftBase.normal = normSideLeft;
    halfTopLeft.normal = normSideLeft;
    thirdLeftBase.normal = normSideLeft;
    backQuarterTopLeft.normal = normSideLeft;
    backLeftBase.normal = normSideLeft;
    endLeft.normal = normSideLeft;

    lowerRight.normal = normSideRight;
    frontRightBase.normal = normSideRight;
    quarterTopRight.normal = normSideRight;
    secondRightBase.normal = normSideRight;
    halfTopRight.normal = normSideRight;
    thirdRightBase.normal = normSideRight;
    backQuarterTopRight.normal = normSideRight;
    backRightBase.normal = normSideRight;
    endRight.normal = normSideRight;
    */
    m_batch2->DrawTriangle(lowerLeftSide, frontLeftBaseSide, quarterTopLeftSide);
    m_batch2->DrawTriangle(lowerRightSide, frontRightBaseSide, quarterTopRightSide);

    m_batch2->DrawTriangle(secondLeftBaseSide, halfTopLeftSide, quarterTopLeftSide);
    m_batch2->DrawTriangle(secondRightBaseSide, halfTopRightSide, quarterTopRightSide);

    m_batch2->DrawTriangle(thirdLeftBaseSide, halfTopLeftSide, backQuarterTopLeftSide);
    m_batch2->DrawTriangle(thirdRightBaseSide, halfTopRightSide, backQuarterTopRightSide);

    m_batch2->DrawTriangle(backLeftBaseSide, backQuarterTopLeftSide, endLeftSide);
    m_batch2->DrawTriangle(backRightBaseSide, backQuarterTopRightSide, endRightSide);

    // railings
    /*
    m_batch2->DrawLine(leftRailing1, leftRailing2);
    leftRailing2.normal = normHalfPos;
    m_batch2->DrawLine(leftRailing2, leftRailing3);
    leftRailing3.normal = normHalfNeg;
    m_batch2->DrawLine(leftRailing3, leftRailing4);
    leftRailing4.normal = normQuarterNeg;
    m_batch2->DrawLine(leftRailing4, leftRailing5);
    */
    m_batch2->DrawLine(leftRailing1, leftRailing2);
    m_batch2->DrawLine(leftRailing3, leftRailing4);
    m_batch2->DrawLine(leftRailing5, leftRailing6);
    m_batch2->DrawLine(leftRailing7, leftRailing8);

    /*
    m_batch2->DrawLine(rightRailing1, rightRailing2);
    rightRailing2.normal = normHalfPos;
    m_batch2->DrawLine(rightRailing2, rightRailing3);
    rightRailing3.normal = normHalfNeg;
    m_batch2->DrawLine(rightRailing3, rightRailing4);
    rightRailing4.normal = normQuarterNeg;
    m_batch2->DrawLine(rightRailing4, rightRailing5);
    */
    m_batch2->DrawLine(rightRailing1, rightRailing2);
    m_batch2->DrawLine(rightRailing3, rightRailing4);
    m_batch2->DrawLine(rightRailing5, rightRailing6);
    m_batch2->DrawLine(rightRailing7, rightRailing8);

}

void Game::DrawCameraFocus()
{
    const float line = .25f;
    DirectX::SimpleMath::Vector3 focalPoint = pCamera->GetTargetPos();

    //pGolf->SetPosToTerrain(focalPoint);
    float height = pGolf->GetTerrainHeight(focalPoint);
    //height += .1;
    focalPoint.y = height;
    DirectX::SimpleMath::Vector3 yLine = focalPoint;
    yLine.y += line;
    DirectX::SimpleMath::Vector3 xLine = focalPoint;
    xLine.x += line;
    DirectX::SimpleMath::Vector3 zLine = focalPoint;
    zLine.z += line;

    DirectX::SimpleMath::Vector3 negZLine = focalPoint;
    negZLine.z -= line;
    DirectX::SimpleMath::Vector3 negXLine = focalPoint;
    negXLine.x -= line;

    VertexPositionColor origin(focalPoint, Colors::Yellow);
    VertexPositionColor yOffset(yLine, Colors::Yellow);
    VertexPositionColor xOffset(xLine, Colors::Yellow);
    VertexPositionColor zOffset(zLine, Colors::Yellow);

    VertexPositionColor negXOffset(negXLine, Colors::Yellow);
    VertexPositionColor negZOffset(negZLine, Colors::Yellow);

    m_debugHeight = focalPoint.y;
    m_debugXpoint = focalPoint.x;
    m_debugZpoint = focalPoint.z;

    m_batch->DrawLine(origin, yOffset);
    m_batch->DrawLine(origin, xOffset);
    m_batch->DrawLine(origin, zOffset);
    m_batch->DrawLine(origin, negZOffset);
    m_batch->DrawLine(origin, negXOffset);
}

void Game::DrawDebugLines()
{
    std::vector<std::pair<DirectX::VertexPositionColor, DirectX::VertexPositionColor>> debugLines = pGolf->GetBallDebugLines();

    for (int i = 0; i < debugLines.size(); ++i)
    {
        m_batch->DrawLine(debugLines[i].first, debugLines[i].second);
    }
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

void Game::DrawFlagHoleFixture(const DirectX::SimpleMath::Vector3 aPos, const float aVariation)
{
    const float poleHeight = 0.1;
    const float flagWidth = .02;
    const float flagHeight = .01;
    const DirectX::XMVECTORF32 flagColor = DirectX::Colors::Red;
    const DirectX::XMVECTORF32 poleColor = DirectX::Colors::White;

    //DirectX::SimpleMath::Vector3 poleBase = aPos;
    DirectX::SimpleMath::Vector3 poleBase = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 poleTop = poleBase;
    poleTop.y += poleHeight;
    DirectX::SimpleMath::Vector3 flagTip = poleTop;
    flagTip.y -= flagHeight;
    flagTip.x -= flagWidth;
    flagTip.z -= flagWidth;

    float windDirection = pGolf->GetWindDirectionRad();
    DirectX::SimpleMath::Vector3 windNormalized = pGolf->GetEnvironWindVector();
    float windSpeed = windNormalized.Length() * .3;
    windNormalized.Normalize();

    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    windDirection = windDirection + (cosf(static_cast<float>(m_timer.GetTotalSeconds() * windSpeed)) * 0.1);
    flagTip = DirectX::SimpleMath::Vector3::Transform(flagTip, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(windDirection)));

    poleTop += swayBase;
    DirectX::SimpleMath::Vector3 flagBottom = poleTop;
    flagBottom.y -= flagHeight + flagHeight;

    DirectX::VertexPositionColor poleBaseVertex(poleBase + aPos, poleColor);
    DirectX::VertexPositionColor poleTopVertex(poleTop + aPos, poleColor);
    DirectX::VertexPositionColor flagTopVertex(poleTop + aPos, flagColor);
    DirectX::VertexPositionColor flagTipVertex(flagTip + aPos, flagColor);
    DirectX::VertexPositionColor flagBottomVertex(flagBottom + aPos, flagColor);

    m_batch->DrawLine(poleBaseVertex, poleTopVertex);
    m_batch->DrawTriangle(flagTopVertex, flagTipVertex, flagBottomVertex);

    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<DirectX::VertexPositionColor> holeVert = pGolf->GetHoleVertex();

    for (int i = 0; i < holeVert.size() - 1; ++i)
    {
        m_batch->DrawLine(holeVert[i], holeVert[i + 1]);
    }

    /*
    std::vector<DirectX::VertexPositionColor> flagVert = pGolf->GetFlagVertex();
    if (flagVert.size() == 5)
    {
        //m_batch->DrawLine(flagVert[0], flagVert[1]);
        //m_batch->DrawTriangle(flagVert[2], flagVert[3], flagVert[4]);
    }
    else
    {
        // error handle vector is of wrong size for needed inputs to draw
    }
    */
}

void Game::DrawFlagHoleFixtureTest1(const DirectX::SimpleMath::Vector3 aPos, const float aVariation)
{
    const float poleHeight = 0.1;
    const float flagWidth = .02;
    const float flagHeight = .01;
    const DirectX::XMVECTORF32 flagColor = DirectX::Colors::Red;
    const DirectX::XMVECTORF32 poleColor = DirectX::Colors::White;

    DirectX::SimpleMath::Vector3 poleBase = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 poleTop = poleBase;
    poleTop.y += poleHeight;
    DirectX::SimpleMath::Vector3 flagTip = poleTop;
    flagTip.y -= flagHeight;
    flagTip.x -= flagWidth;
    flagTip.z -= flagWidth;

    float windDirection = pGolf->GetWindDirectionRad();
    DirectX::SimpleMath::Vector3 windNormalized = pGolf->GetEnvironWindVector();
    float windSpeed = windNormalized.Length() * .3;
    windNormalized.Normalize();

    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    windDirection = windDirection + (cosf(static_cast<float>(m_timer.GetTotalSeconds() * windSpeed)) * 0.1);
    flagTip = DirectX::SimpleMath::Vector3::Transform(flagTip, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(windDirection)));

    poleTop += swayBase;
    DirectX::SimpleMath::Vector3 flagBottom = poleTop;
    flagBottom.y -= flagHeight + flagHeight;

    DirectX::VertexPositionNormalColor poleBaseVertex(poleBase + aPos, DirectX::SimpleMath::Vector3::UnitY, poleColor);
    DirectX::VertexPositionNormalColor poleTopVertex(poleTop + aPos, DirectX::SimpleMath::Vector3::UnitY, poleColor);
    DirectX::VertexPositionNormalColor flagTopVertex(poleTop + aPos, DirectX::SimpleMath::Vector3::UnitY, flagColor);
    DirectX::VertexPositionNormalColor flagTipVertex(flagTip + aPos, DirectX::SimpleMath::Vector3::UnitY, flagColor);
    DirectX::VertexPositionNormalColor flagBottomVertex(flagBottom + aPos, DirectX::SimpleMath::Vector3::UnitY, flagColor);

    m_batch2->DrawLine(poleBaseVertex, poleTopVertex);
    m_batch2->DrawTriangle(flagTopVertex, flagTipVertex, flagBottomVertex);

    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<DirectX::VertexPositionNormalColor> holeVert = pGolf->GetHoleVertexTest1();

    for (int i = 0; i < holeVert.size() - 1; ++i)
    {
        m_batch2->DrawLine(holeVert[i], holeVert[i + 1]);
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

    std::vector<std::vector<BallMotion>> hydraPath = pGolf->GetHydraShotPath();

    if (hydraPath.size() > 1)
    {
        for (int i = 0; i < hydraPath.size(); ++i)
        {
            DirectX::SimpleMath::Vector3 prevPos2 = hydraPath[i][0].position;
            int ballPosIndex2 = 0;
            for (int j = 0; j < hydraPath[i].size(); ++j)
            {
                DirectX::SimpleMath::Vector3 p1(prevPos2);
                DirectX::SimpleMath::Vector3 p2(hydraPath[i][j].position);
                //VertexPositionColor aV(p1, lineColor[i]);
               // VertexPositionColor bV(p2, lineColor[i]);
                VertexPositionColor aV(p1, lineEndColor[i]);
                VertexPositionColor bV(p2, lineEndColor[i]);
                //VertexPositionColor aV(p1, DirectX::Colors::White);
                //VertexPositionColor bV(p2, DirectX::Colors::White);

                if (hydraPath[i][j].time < m_projectileTimer)
                {
                    m_batch->DrawLine(aV, bV);
                    ballPosIndex2 = j;
                }
                prevPos2 = hydraPath[i][j].position;
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
    //float fadeDuration = 1.5f;
    //float logoDisplayDuration = 5.f;
    float fadeDuration = 0.01f;
    float logoDisplayDuration = 15.f;
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
        m_currentGameState = GameState::GAMESTATE_STARTSCREEN;
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

    // Demo
    ////////////////////////////

    lineDrawY += menuObj0Pos.y;
    std::string menuObjHydraString = "Hole 12 Golden Bell";
    DirectX::SimpleMath::Vector2 menuObjHydraPos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObjHydraOrigin = m_font->MeasureString(menuObjHydraString.c_str()) / 2.f;

    ///////////////////////////

    lineDrawY += menuObj0Pos.y;
    std::string menuObj3String = "Quit";
    DirectX::SimpleMath::Vector2 menuObj3Pos(menuTitlePosX, lineDrawY);
    DirectX::SimpleMath::Vector2 menuObj3Origin = m_font->MeasureString(menuObj3String.c_str()) / 2.f;

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
    std::vector<BallMotion> shotPath = pGolf->GetShotPath();

    if (shotPath.size() > 1)
    {
        int stepCount = (int)shotPath.size();

        if (m_projectilePathStep >= stepCount)
        {
            m_flightStepTimer.ResetElapsedTime();
            m_projectilePathStep = 0;
        }
        pGolf->SetBallPosition(shotPath[m_projectilePathStep].position);
        ++m_projectilePathStep;

        DirectX::SimpleMath::Vector3 prevPos = shotPath[0].position;
        for (int i = 0; i < m_projectilePathStep; ++i)
        {
            DirectX::SimpleMath::Vector3 p1(prevPos);
            DirectX::SimpleMath::Vector3 p2(shotPath[i].position);

            VertexPositionColor aV(p1, Colors::White);
            VertexPositionColor bV(p2, Colors::White);

            m_batch->DrawLine(aV, bV);
            prevPos = shotPath[i].position;
        }
    }
}

void Game::DrawProjectileRealTime()
{
    //std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();
    std::vector<BallMotion> shotPath = pGolf->GetShotPath();

    if (shotPath.size() > 1)
    {
        //std::vector<float> shotTimeStep = pGolf->GetShotPathTimeSteps();

        DirectX::SimpleMath::Vector3 prevPos = shotPath[0].position;
        int ballPosIndex = 0;
        for (int i = 0; i < shotPath.size(); ++i)
        {
            //if (shotTimeStep[i] < m_projectileTimer)
            if (shotPath[i].time < m_projectileTimer)
            {
                DirectX::SimpleMath::Vector3 p1(prevPos);
                DirectX::SimpleMath::Vector3 p2(shotPath[i].position);
                VertexPositionColor aV(p1, Colors::White);
                VertexPositionColor bV(p2, Colors::White);

                m_batch->DrawLine(aV, bV);
                ballPosIndex = i;

                prevPos = shotPath[i].position;

                // Deactivate gameplay UI and display the score for the hole cause the ball is in it, further updates will be needed when scorecard functionality is implemented
                if (pGolf->GetIsBallInHole() == true && m_currentUiState != UiState::UISTATE_SCORE && i == shotPath.size() - 1)
                {
                    pPlay->CalculateScoreString(pGolf->GetParFromEnviron());
                    m_currentUiState = UiState::UISTATE_SCORE;
                }
            }
        }
        pGolf->SetBallPosition(shotPath[ballPosIndex].position);

        // Set camera targe on ball position if using projectile tracking camera
        if (pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
        {
            pCamera->SetTargetPos(pGolf->GetBallPosition());
        }
    }
}

void Game::DrawSand()
{
    DirectX::XMVECTORF32 sandColor1 = DirectX::Colors::Beige;
    DirectX::XMVECTORF32 sandColor2 = DirectX::Colors::SandyBrown;
    DirectX::XMVECTORF32 sandColor9 = DirectX::Colors::Red;
    const float height = 0.004;

    DirectX::SimpleMath::Vector3 b1nw(2.45f, height, -0.6f);
    DirectX::SimpleMath::Vector3 b1ne(2.65f, height, -0.5f);
    DirectX::SimpleMath::Vector3 b1se(2.66f, 0.0, -0.21f);
    DirectX::SimpleMath::Vector3 b1sw(2.35f, height, -0.5f);

    DirectX::SimpleMath::Vector3 e(4.55, height, 2.001);
    DirectX::SimpleMath::Vector3 w(0.55, height, -2.001);

    VertexPositionColor b1v1(b1nw, sandColor1);
    VertexPositionColor b1v2(b1ne, sandColor2);
    VertexPositionColor b1v3(b1se, sandColor1);
    VertexPositionColor b1v4(b1sw, sandColor1);

    m_batch->DrawQuad(b1v1, b1v2, b1v3, b1v4);


    DirectX::XMVECTORF32 sandColor4 = DirectX::Colors::SandyBrown;
    DirectX::XMVECTORF32 sandColor3 = DirectX::Colors::Beige;

    DirectX::SimpleMath::Vector3 b2nw(2.7f, height, -0.4f);
    DirectX::SimpleMath::Vector3 b2ne(2.85f, height, -0.2f);
    DirectX::SimpleMath::Vector3 b2se(2.8f, height, -0.05f);
    DirectX::SimpleMath::Vector3 b2sw(2.49f, 0.0, -0.4f);

    VertexPositionColor b2v1(b2nw, sandColor2);
    VertexPositionColor b2v2(b2ne, sandColor1);
    VertexPositionColor b2v3(b2se, sandColor1);
    VertexPositionColor b2v4(b2sw, sandColor1);

    m_batch->DrawQuad(b2v1, b2v2, b2v3, b2v4);

    DirectX::XMVECTORF32 sandColor5 = DirectX::Colors::Red;

    DirectX::SimpleMath::Vector3 b3nw(3.6f, height, -0.55f);
    DirectX::SimpleMath::Vector3 b3ne(3.7f, height, -0.41f);
    DirectX::SimpleMath::Vector3 b3se(3.56f, height, -0.34f);
    DirectX::SimpleMath::Vector3 b3sw(3.41f, height, -0.55f);

    VertexPositionColor b3v1(b3nw, sandColor1);
    VertexPositionColor b3v2(b3ne, sandColor2);
    VertexPositionColor b3v3(b3se, sandColor1);
    VertexPositionColor b3v4(b3sw, sandColor1);

    m_batch->DrawQuad(b3v1, b3v2, b3v3, b3v4);


    DirectX::SimpleMath::Vector3 b4nw(3.7f, height, -0.38f);
    DirectX::SimpleMath::Vector3 b4ne(3.85f, height, -0.2f);
    DirectX::SimpleMath::Vector3 b4se(3.71f, height, -0.18f);
    DirectX::SimpleMath::Vector3 b4sw(3.58f, height, -0.29f);

    VertexPositionColor b4v1(b4nw, sandColor1);
    VertexPositionColor b4v2(b4ne, sandColor2);
    VertexPositionColor b4v3(b4se, sandColor1);
    VertexPositionColor b4v4(b4sw, sandColor1);

    m_batch->DrawQuad(b4v1, b4v2, b4v3, b4v4);
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

    aimLine += pGolf->GetShotStartPos();
    aimLineLeft += pGolf->GetShotStartPos();
    aimLineRight += pGolf->GetShotStartPos();

    DirectX::SimpleMath::Vector3 centerBase = DirectX::SimpleMath::Vector3(centerIndent, aimHeight, 0.0f);
    centerBase = DirectX::SimpleMath::Vector3::Transform(centerBase, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection())));
    centerBase += pGolf->GetShotStartPos();

    VertexPositionColor origin(centerBase, Colors::Red);
    VertexPositionColor aimPoint(aimLine, Colors::Red);
    VertexPositionColor aimBaseLeft(aimLineLeft, Colors::Red);
    VertexPositionColor aimBaseRight(aimLineRight, Colors::DarkRed);
    VertexPositionColor originRight = origin;
    originRight.color.x = 0.5450980663F;
    VertexPositionColor aimPointRight = aimPoint;
    aimPointRight.color.x = 0.5450980663F;

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
    const float width = .0145f;
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

void Game::DrawStartScreen()
{
    const std::string title = "GolfGame1989";
    const std::string author = "By Lehr Jackson";
    const std::string startText = "Press Enter to Start";
    const float fontTitlePosX = m_fontPos.x;
    const float fontTitlePosY = m_fontPos.y / 2.f;
    const DirectX::SimpleMath::Vector2 titlePos(fontTitlePosX, fontTitlePosY);
    const float fontAuthorPosX = m_fontPos.x;
    const float fontAuthorPosY = m_fontPos.y;
    const DirectX::SimpleMath::Vector2 authorPos(fontAuthorPosX, fontAuthorPosY);
    const DirectX::SimpleMath::Vector2 startTextPos(m_fontPos.x, m_fontPos.y + fontTitlePosY);

    const DirectX::SimpleMath::Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    const DirectX::SimpleMath::Vector2 authorOrigin = m_font->MeasureString(author.c_str()) / 2.f;
    const DirectX::SimpleMath::Vector2 startTextOrigin = m_font->MeasureString(startText.c_str()) / 2.f;

    const DirectX::XMVECTORF32 shadowColor = DirectX::Colors::Green;
    const DirectX::XMVECTORF32 highlightColor = DirectX::Colors::LawnGreen;
    const DirectX::XMVECTORF32 faceColor = DirectX::Colors::LimeGreen;
    const DirectX::XMVECTORF32 textColor = DirectX::Colors::White;
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(7.f, 7.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(6.f, 6.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(5.f, 5.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(4.f, 4.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(3.f, 3.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(2.f, 2.f), shadowColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + DirectX::SimpleMath::Vector2(-1.f, -1.f), highlightColor, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, faceColor, 0.f, titleOrigin);

    m_font->DrawString(m_spriteBatch.get(), author.c_str(), authorPos, textColor, 0.f, authorOrigin);
    m_font->DrawString(m_spriteBatch.get(), startText.c_str(), startTextPos, textColor, 0.f, startTextOrigin);
}

void Game::DrawSwing()
{
    std::vector<DirectX::SimpleMath::Vector3> angles = pGolf->GetRawSwingAngles();

    if (angles.size() >= 1)
    {
        const int impactPoint = pGolf->GetImpactStep();
        DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(pPlay->GetShotDirection()));
        const float armLength = pGolf->GetArmLength() * pGolf->GetEnvironScale();
        const float clubLength = pGolf->GetClubLength() * pGolf->GetEnvironScale();

        DirectX::SimpleMath::Vector3 shoulderPos = pGolf->GetSwingShoulderOrigin() * pGolf->GetEnvironScale();
        const float clubFaceWidth = -shoulderPos.z * 2; // WLJ ToDo: Replace this dirty implementation to get club head width once club head shape system is implemented
        shoulderPos = DirectX::SimpleMath::Vector3::Transform(shoulderPos, rotMat);
        shoulderPos += pGolf->GetShotStartPos();

        VertexPositionColor staticShoulderVert(shoulderPos, DirectX::Colors::Yellow);
        VertexPositionColor staticBaseVert(pGolf->GetShotStartPos(), DirectX::Colors::White);
        //m_batch->DrawLine(staticShoulderVert, staticBaseVert);

        DirectX::XMVECTORF32 preImpactShadowColor = DirectX::Colors::DarkSlateGray;
        DirectX::XMVECTORF32 postImpactShadowColor = DirectX::Colors::DarkSlateBlue;
        DirectX::XMVECTORF32 color1 = DirectX::Colors::White;
        DirectX::XMVECTORF32 color2 = DirectX::Colors::Red;
        DirectX::XMVECTORF32 color3 = DirectX::Colors::Red;
        DirectX::XMVECTORF32 color4 = DirectX::Colors::Blue;
        DirectX::XMVECTORF32 color5 = DirectX::Colors::Yellow;
        DirectX::XMVECTORF32 shoulderColor = color1;
        DirectX::XMVECTORF32 handColor = color1;
        DirectX::XMVECTORF32 clubHeadColor = color1;

        if (m_swingPathStep >= (int)angles.size())
        {
            m_swingPathStep = 0;
        }
        ++m_swingPathStep;

        const float grooveGap = .0005;
        const double clubFaceAngle = Utility::ToRadians(pGolf->GetClubFaceAngle());
        bool isBallHit = false;
        for (int i = 0; i < m_swingPathStep; ++i)
        {
            if (i < impactPoint)
            {
                shoulderColor = preImpactShadowColor;
                handColor = preImpactShadowColor;
                clubHeadColor = preImpactShadowColor;
            }
            if (i > impactPoint)
            {
                shoulderColor = postImpactShadowColor;
                handColor = postImpactShadowColor;
                clubHeadColor = postImpactShadowColor;
                isBallHit = true;
            }
            if (i == m_swingPathStep - 1)
            {
                shoulderColor = color1;
                handColor = color1;
                clubHeadColor = color1;
            }
            if (i == impactPoint)
            {
                shoulderColor = color3;
                handColor = color3;
                clubHeadColor = color3;
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
            DirectX::XMVECTORF32 testColor1 = DirectX::Colors::White;
            int testVar = (m_swingPathStep - 1) - i;
            float testDelta = testVar * .03;
            testColor1.f[0] -= testDelta;
            testColor1.f[1] -= testDelta;
            testColor1.f[2] -= testDelta;
            testColor1.f[3] -= testDelta;

            DirectX::XMVECTORF32 testColor2 = DirectX::Colors::Blue;
            testColor2.f[0] -= testDelta;
            testColor2.f[1] -= testDelta;
            testColor2.f[2] -= testDelta;
            testColor2.f[3] -= testDelta;

            DirectX::SimpleMath::Vector3 theta(0.0, -armLength, 0.0);
            DirectX::SimpleMath::Vector3 beta(0.0, -clubLength, 0.0);
            theta = DirectX::SimpleMath::Vector3::Transform(theta, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].z));
            beta = DirectX::SimpleMath::Vector3::Transform(beta, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].y + -angles[i].z));
            theta = DirectX::SimpleMath::Vector3::Transform(theta, rotMat);
            beta = DirectX::SimpleMath::Vector3::Transform(beta, rotMat);

            theta += shoulderPos;
            beta += theta;

            DirectX::SimpleMath::Vector3 clubHeadBase = DirectX::SimpleMath::Vector3::Zero;
            clubHeadBase.z += clubFaceWidth;

            // Rotate to point swing draw in direction of swing aim

            DirectX::SimpleMath::Matrix clubFaceRot = DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].y + -angles[i].z + clubFaceAngle);
            clubHeadBase = DirectX::SimpleMath::Vector3::Transform(clubHeadBase, rotMat);
            clubHeadBase += beta;

            DirectX::SimpleMath::Vector3 toeGroove1 = DirectX::SimpleMath::Vector3::Zero;
            toeGroove1.y += grooveGap;
            toeGroove1 = DirectX::SimpleMath::Vector3::Transform(toeGroove1, clubFaceRot);
            toeGroove1 = DirectX::SimpleMath::Vector3::Transform(toeGroove1, rotMat);
            toeGroove1 += clubHeadBase;
            //VertexPositionColor toeGrooveVert1(toeGroove1, clubHeadColor);
            VertexPositionColor toeGrooveVert1(toeGroove1, testColor1);

            DirectX::SimpleMath::Vector3 heelGroove1 = DirectX::SimpleMath::Vector3::Zero;
            heelGroove1.y += grooveGap;
            heelGroove1 = DirectX::SimpleMath::Vector3::Transform(heelGroove1, clubFaceRot);
            heelGroove1 += beta;
            //VertexPositionColor heelGrooveVert1(heelGroove1, clubHeadColor);
            VertexPositionColor heelGrooveVert1(heelGroove1, testColor1);

            DirectX::SimpleMath::Vector3 toeGroove2 = DirectX::SimpleMath::Vector3::Zero;
            toeGroove2.y += grooveGap * 2;
            toeGroove2 = DirectX::SimpleMath::Vector3::Transform(toeGroove2, clubFaceRot);
            toeGroove2 += clubHeadBase;
            //VertexPositionColor toeGrooveVert2(toeGroove2, clubHeadColor);
            VertexPositionColor toeGrooveVert2(toeGroove2, testColor1);

            DirectX::SimpleMath::Vector3 heelGroove2 = DirectX::SimpleMath::Vector3::Zero;
            heelGroove2.y += grooveGap * 2;
            heelGroove2 = DirectX::SimpleMath::Vector3::Transform(heelGroove2, clubFaceRot);
            heelGroove2 += beta;
            //VertexPositionColor heelGrooveVert2(heelGroove2, clubHeadColor);
            VertexPositionColor heelGrooveVert2(heelGroove2, testColor1);

            DirectX::SimpleMath::Matrix testRot = clubFaceRot * rotMat;
            DirectX::SimpleMath::Vector3 toeGroove3 = DirectX::SimpleMath::Vector3::Zero;
            toeGroove3.y += grooveGap * 3;
            toeGroove3 = DirectX::SimpleMath::Vector3::Transform(toeGroove3, testRot);
            toeGroove3 += clubHeadBase;
            //VertexPositionColor toeGrooveVert3(toeGroove3, clubHeadColor);
            VertexPositionColor toeGrooveVert3(toeGroove3, testColor1);

            DirectX::SimpleMath::Vector3 heelGroove3 = DirectX::SimpleMath::Vector3::Zero;
            heelGroove3.y += grooveGap * 3;
            heelGroove3 = DirectX::SimpleMath::Vector3::Transform(heelGroove3, clubFaceRot);
            heelGroove3 += beta;
            //VertexPositionColor heelGrooveVert3(heelGroove3, clubHeadColor);
            VertexPositionColor heelGrooveVert3(heelGroove3, testColor1);

            //color5 = DirectX::Colors::Black;
            //color4 = DirectX::Colors::Transparent;
            VertexPositionColor clubHeadVert(clubHeadBase, clubHeadColor);
            VertexPositionColor shoulderVert(shoulderPos, shoulderColor);
            //VertexPositionColor shoulderTestVert(shoulderPos, color4);
            VertexPositionColor shoulderTestVert(shoulderPos, testColor2);
            //VertexPositionColor thetaTestVert(theta, color4);
            VertexPositionColor thetaTestVert(theta, testColor2);
            VertexPositionColor thetaTestLowerVert(theta, testColor1);
            //VertexPositionColor thetaTestLowerVert(theta, color5);
            VertexPositionColor thetaVert(theta, handColor);
            VertexPositionColor betaTestVert(beta, testColor1);
            //VertexPositionColor betaTestVert(beta, color5);
            VertexPositionColor betaVert(beta, clubHeadColor);
            //m_batch->DrawLine(shoulderVert, thetaVert);
            //m_batch->DrawLine(shoulderVert, thetaVert);
            //m_batch->DrawLine(thetaVert, betaVert);

            if (((m_swingPathStep - 1) - i) < 40)
            {
                //m_batch->DrawLine(shoulderTestVert, thetaTestVert);
                m_batch->DrawLine(thetaTestLowerVert, betaTestVert);
                //m_batch->DrawLine(shoulderVert, thetaVert);
                //m_batch->DrawLine(thetaVert, betaVert);
                m_batch->DrawLine(betaVert, clubHeadVert);
                m_batch->DrawLine(clubHeadVert, toeGrooveVert1);
                m_batch->DrawLine(toeGrooveVert1, heelGrooveVert1);
                m_batch->DrawLine(toeGrooveVert1, toeGrooveVert2);
                m_batch->DrawLine(toeGrooveVert2, heelGrooveVert2);
                m_batch->DrawLine(toeGrooveVert3, heelGrooveVert3);
                m_batch->DrawLine(toeGrooveVert2, toeGrooveVert3);
                m_batch->DrawLine(betaVert, heelGrooveVert3);
            }
            if (i == impactPoint)
            {
                //m_batch->DrawLine(shoulderVert, thetaVert);
                //m_batch->DrawLine(thetaVert, betaVert);
            }
            if (i == m_swingPathStep - 1)
            {
                VertexPositionColor shoulderLiveVert(shoulderPos, DirectX::Colors::Blue);
                VertexPositionColor thetaLiveVert(theta, DirectX::Colors::White);
                VertexPositionColor betaLiveVert(beta, DirectX::Colors::White);
                m_batch->DrawLine(shoulderLiveVert, thetaLiveVert);
                m_batch->DrawLine(thetaLiveVert, betaLiveVert);

                /*
                m_batch->DrawLine(shoulderVert, thetaVert);
                m_batch->DrawLine(thetaVert, betaVert);
                m_batch->DrawLine(betaVert, clubHeadVert);
                m_batch->DrawLine(clubHeadVert, toeGrooveVert1);
                m_batch->DrawLine(toeGrooveVert1, heelGrooveVert1);
                m_batch->DrawLine(toeGrooveVert1, toeGrooveVert2);
                m_batch->DrawLine(toeGrooveVert2, heelGrooveVert2);
                m_batch->DrawLine(toeGrooveVert3, heelGrooveVert3);
                m_batch->DrawLine(toeGrooveVert2, toeGrooveVert3);
                */
                //m_batch->DrawTriangle(betaVert2, clubHeadVert, toeGrooveVert3);
            }

            // debug data drawing
            if (i == impactPoint)
            {
                DirectX::SimpleMath::Vector3 horNorm = toeGroove2 + heelGroove2;

                DirectX::SimpleMath::Vector3 vertNorm = toeGroove2 + toeGroove3;
                DirectX::SimpleMath::Vector3 faceNorm = horNorm;
                faceNorm.Normalize();
                faceNorm += horNorm;
                faceNorm.Cross(vertNorm, faceNorm);
                VertexPositionColor faceVert(faceNorm, DirectX::Colors::Yellow);
                //m_batch->DrawLine(toeGrooveVert2, faceVert);

                DirectX::SimpleMath::Vector3 impactFaceNorm = pGolf->GetFaceImpact();
                impactFaceNorm += toeGroove2;
                VertexPositionColor faceNormVert(impactFaceNorm, DirectX::Colors::Orange);
                m_batch->DrawLine(toeGrooveVert2, faceNormVert);
            }
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
    const float teeBoxHorizontalScale = 10.1f;
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

void Game::DrawTeeBoxFixture(const DirectX::SimpleMath::Vector3 aPos, const float aVariation)
{
    //draw tee box;
    DirectX::SimpleMath::Vector3 teeBoxOrigin = pGolf->GetTeePos();
    teeBoxOrigin.y += 0.003;
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

void Game::DrawTeeBoxFixtureTest1(const DirectX::SimpleMath::Vector3 aPos, const float aVariation)
{
    //draw tee box;
    DirectX::SimpleMath::Vector3 teeBoxOrigin = pGolf->GetTeePos();
    teeBoxOrigin.y += 0.003;
    const float teeBoxLengthScale = 0.05f;
    const float teeBoxHorizontalScale = 0.1f;
    DirectX::SimpleMath::Vector3 t1(teeBoxOrigin.x - teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z - teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t2(teeBoxOrigin.x + teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z - teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t3(teeBoxOrigin.x - teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z + teeBoxHorizontalScale);
    DirectX::SimpleMath::Vector3 t4(teeBoxOrigin.x + teeBoxLengthScale, teeBoxOrigin.y, teeBoxOrigin.z + teeBoxHorizontalScale);
    VertexPositionNormalColor vt1(t1, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor vt2(t2, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor vt3(t3, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor vt4(t4, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(vt1, vt2);
    m_batch2->DrawLine(vt1, vt3);
    m_batch2->DrawLine(vt3, vt4);
    m_batch2->DrawLine(vt4, vt2);
    // end tee box draw
}

void Game::DrawTree03(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
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

    // tree trunk
    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);

    const float branchGap = .01;
    const float halfBranchGap = branchGap * .5;

    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Yellow;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::DarkOliveGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    const int layerCount = 20;
    for (int i = 1; i < layerCount; ++i)
    {
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

        branchEndR.y += branchGap;
        branchEndR += swayVec;

        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, branchColor3);
        VertexPositionColor leafL(branchEndL, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor2);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor2);
        VertexPositionColor branchBaseLower(branchBaseLowerVert, branchColor2);

        m_batch->DrawQuad(leafL, leafLeftLower, branchBaseVertex, branchBaseLower);
        m_batch->DrawQuad(leafR, leafRightLower, branchBaseVertex, branchBaseLower);
    }
}

void Game::DrawTree04(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

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
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    testBaseTop += swayBase;
    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(testBaseTop, Colors::Gray);

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .09f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .09f);


    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    //trunkBottomRight += swayBase;
    //trunkBottomLeft += swayBase;

    /// tree trunk
    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::DeepPink;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::GreenYellow;

    branchEndL += swayVec;
    VertexPositionColor leafR(branchEndR, branchColor1);
    VertexPositionColor leafL(branchEndL, branchColor1);
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
        VertexPositionColor leafR(branchEndR, branchColor1);
        VertexPositionColor leafL(branchEndL, branchColor3);
        VertexPositionColor branchRoot(branchBase, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor2);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor2);
        m_batch->DrawQuad(leafL, leafR, leafRightLower, leafLeftLower);

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

void Game::DrawTree04Test1(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

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
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    DirectX::SimpleMath::Vector3 trunkNorm = viewLine;
    testBaseTop += swayBase;
    VertexPositionNormalColor treeRootBase(aTreePos, trunkNorm, Colors::Gray);
    VertexPositionNormalColor treeRootTop(testBaseTop, trunkNorm, Colors::Gray);

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .09f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .09f);


    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    //trunkBottomRight += swayBase;
    //trunkBottomLeft += swayBase;

    /// tree trunk
    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    DirectX::SimpleMath::Vector3 upNorm = DirectX::SimpleMath::Vector3::UnitY;
    VertexPositionNormalColor treeRootTop2(baseTop, upNorm, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    VertexPositionNormalColor branchBaseVertex(branchBase, upNorm, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::DeepPink;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::GreenYellow;

    branchEndL += swayVec;
    VertexPositionNormalColor leafR(branchEndR, upNorm, branchColor1);
    VertexPositionNormalColor leafL(branchEndL, upNorm, branchColor1);
    m_batch2->DrawLine(branchBaseVertex, leafR);
    m_batch2->DrawLine(branchBaseVertex, leafL);

    //// increse width
    float widthMod = 1.0;
    int layerHeight = 6;
    for (int i = 0; i < layerHeight * 3; ++i)
    {
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + branchBase;

        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + branchBase;

        branchEndL += swayVec;
        VertexPositionNormalColor leafR(branchEndR, upNorm, branchColor1);
        VertexPositionNormalColor leafL(branchEndL, upNorm, branchColor3);
        VertexPositionNormalColor branchRoot(branchBase, upNorm, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, upNorm, branchColor2);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, upNorm, branchColor2);
        m_batch2->DrawQuad(leafL, leafR, leafRightLower, leafLeftLower);

        m_batch2->DrawLine(branchRoot, leafR);
        m_batch2->DrawLine(branchRoot, leafL);
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

void Game::DrawTree05(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

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
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    testBaseTop += swayBase;
    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(testBaseTop, Colors::Gray);

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .09f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .09f);


    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    //trunkBottomRight += swayBase;
    //trunkBottomLeft += swayBase;

    /// tree trunk
    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::White;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::GreenYellow;

    branchEndL += swayVec;
    VertexPositionColor leafR(branchEndR, branchColor1);
    VertexPositionColor leafL(branchEndL, branchColor1);
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
        VertexPositionColor leafR(branchEndR, branchColor1);
        VertexPositionColor leafL(branchEndL, branchColor3);
        VertexPositionColor branchRoot(branchBase, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor2);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor2);
        m_batch->DrawQuad(leafL, leafR, leafRightLower, leafLeftLower);

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

void Game::DrawTree05Test1(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 1.0;
    const float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation));

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .04;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 trunkNorm = viewLine;

    //DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross(viewLine, (aTreePos + baseTop));
    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine) * 1.1;
    //viewHorizontal = viewHorizontal / 2;

    const float branchGap = .01;
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    testBaseTop += swayBase;

    VertexPositionNormalColor treeRootBase(aTreePos, trunkNorm, Colors::Gray);
    VertexPositionNormalColor treeRootTop(testBaseTop, trunkNorm, Colors::Gray);

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .09f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .09f);


    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    //trunkBottomRight += swayBase;
    //trunkBottomLeft += swayBase;

    /// tree trunk
    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionNormalColor treeRootTop2(baseTop, trunkNorm, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    DirectX::SimpleMath::Vector3 upNorm = DirectX::SimpleMath::Vector3::UnitY;

    VertexPositionNormalColor branchBaseVertex(branchBase, upNorm, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;

    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::White;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::GreenYellow;

    branchEndL += swayVec;
    VertexPositionNormalColor leafR(branchEndR, upNorm, branchColor1);
    VertexPositionNormalColor leafL(branchEndL, upNorm, branchColor1);
    m_batch2->DrawLine(branchBaseVertex, leafR);
    m_batch2->DrawLine(branchBaseVertex, leafL);

    //// increse width
    float widthMod = 1.0;
    int layerHeight = 6;
    for (int i = 0; i < layerHeight * 3; ++i)
    {
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + branchBase;

        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + branchBase;

        branchEndL += swayVec;
        VertexPositionNormalColor leafR(branchEndR, upNorm, branchColor1);
        VertexPositionNormalColor leafL(branchEndL, upNorm, branchColor3);
        VertexPositionNormalColor branchRoot(branchBase, upNorm, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, upNorm, branchColor2);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, upNorm, branchColor2);
        m_batch2->DrawQuad(leafL, leafR, leafRightLower, leafLeftLower);

        m_batch2->DrawLine(branchRoot, leafR);
        m_batch2->DrawLine(branchRoot, leafL);
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

void Game::DrawTree06(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    //DirectX::SimpleMath::Vector3 swayVec = windVector * scale * cosf(static_cast<float>(m_timer.GetTotalSeconds()));
    //DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>(m_timer.GetTotalSeconds() + aVariation)));
    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.001;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .06;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);


    const float branchGap = .01;
    const float halfBranchGap = branchGap * .5f;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);


    /*
    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal);
    */

    /*
    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    trunkBottomRight += swayBase;
    trunkBottomLeft += swayBase;
    */

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    //m_batch->DrawLine(treeRootBase, treeRootTop);
    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor4 = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor5 = DirectX::Colors::LawnGreen;
    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        VertexPositionColor branchBaseVertexLite(branchBase, branchColor5);

        branchBase.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, branchColor4);
        VertexPositionColor leafL(branchEndL, branchColor4);

        //
        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor3);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor3);
        VertexPositionColor branchBaseLower(branchBaseLowerVert, branchColor2);
        VertexPositionColor branchBaseLowerLite(branchBaseLowerVert, branchColor5);

        //m_batch->DrawLine(branchBaseVertex, leafR);
        //m_batch->DrawLine(branchBaseVertex, leafL);
        //m_batch->DrawQuad(leafL, leafLeftLower, branchBaseLower, branchBaseVertexLite);
        //m_batch->DrawTriangle(leafL, leafLeftLower, branchBaseLower);// , branchBaseVertexLite);
        m_batch->DrawTriangle(leafL, branchBaseLower , branchBaseVertexLite);
        //m_batch->DrawQuad(leafR, branchBaseVertex, branchBaseLower, leafRightLower);
        m_batch->DrawTriangle(leafRightLower, branchBaseVertex, branchBaseLower);// , leafRightLower);
    }
}

void Game::DrawTree06Test1(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)  // tri
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 testNorm = DirectX::SimpleMath::Vector3::UnitY;
    auto time = static_cast<float>(m_timer.GetTotalSeconds());
    float yaw = time * 0.4f;
    float pitch = time * 0.7f;
    float roll = time * 1.1f;

    auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(pitch, yaw, roll);
    DirectX::SimpleMath::Vector3 light0 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat0);
    //testNorm = light0;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.001;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .06;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);

    const float branchGap = .01;
    const float halfBranchGap = branchGap * .5f;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, testNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, testNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor4 = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor5 = DirectX::Colors::LawnGreen;
    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, testNorm, Colors::ForestGreen);
        VertexPositionNormalColor branchBaseVertexLite(branchBase, testNorm, branchColor5);

        branchBase.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionNormalColor leafR(branchEndR, testNorm, branchColor4);
        VertexPositionNormalColor leafL(branchEndL, testNorm, branchColor4);

        //
        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, testNorm, branchColor3);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, testNorm, branchColor3);
        VertexPositionNormalColor branchBaseLower(branchBaseLowerVert, testNorm, branchColor2);
        VertexPositionNormalColor branchBaseLowerLite(branchBaseLowerVert, testNorm, branchColor5);

        m_batch2->DrawTriangle(leafL, branchBaseLower, branchBaseVertexLite);
        m_batch2->DrawTriangle(leafRightLower, branchBaseVertex, branchBaseLower);
    }
}

void Game::DrawTree06Test2(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)  // tri
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 testNorm = DirectX::SimpleMath::Vector3::UnitY;
    auto time = static_cast<float>(m_timer.GetTotalSeconds());
    float yaw = time * 0.4f;
    float pitch = time * 0.7f;
    float roll = time * 1.1f;

    auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(pitch, yaw, roll);
    DirectX::SimpleMath::Vector3 light0 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat0);
    //testNorm = light0;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.001;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .06;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine = aTreePos - pCamera->GetPos();
    viewLine.y = 0.0;
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);
    DirectX::SimpleMath::Vector3 testLeafNormRight = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixRight = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRight);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateRight = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRotateRight);

    testLeafNormRight.Normalize();
    VertexPositionNormalColor leafNormPointRight(baseTop + testLeafNormRight, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointRight);

    DirectX::SimpleMath::Vector3 testLeafNormLeft = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixLeft = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-135.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateLeft = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(45.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixRotateLeft);

    testLeafMatrixLeft *= testLeafMatrixRotateLeft;
    testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);

    testLeafNormLeft.Normalize();
    VertexPositionNormalColor leafNormPointLeft(baseTop + testLeafNormLeft, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointLeft);

    DirectX::SimpleMath::Vector3 testNormL = testLeafNormLeft;
    DirectX::SimpleMath::Vector3 testNormR = testLeafNormRight;
    //testNormL = - DirectX::SimpleMath::Vector3::UnitX;
    //testNormR = - DirectX::SimpleMath::Vector3::UnitX;

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);

    const float branchGap = .01;
    const float halfBranchGap = branchGap * .5f;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, testNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, testNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, testNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, testNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    DirectX::SimpleMath::Vector3 testTop;
    DirectX::SimpleMath::Vector3 testBase = branchBase;
    DirectX::SimpleMath::Vector3 testLeafL;
    DirectX::SimpleMath::Vector3 testLeafR;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor4 = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor5 = DirectX::Colors::LawnGreen;
    const int layerCount = 30;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, testNorm, Colors::ForestGreen);
        VertexPositionNormalColor branchBaseVertexLite(branchBase, testNorm, branchColor5);

        VertexPositionNormalColor branchBaseVertexL(branchBase, testNormL, Colors::ForestGreen);
        VertexPositionNormalColor branchBaseVertexLiteL(branchBase, testNormL, branchColor5);
        VertexPositionNormalColor branchBaseVertexR(branchBase, testNormR, Colors::ForestGreen);
        VertexPositionNormalColor branchBaseVertexLiteR(branchBase, testNormR, branchColor5);

        branchBase.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;
        VertexPositionNormalColor leafR(branchEndR, testNorm, branchColor4);
        VertexPositionNormalColor leafL(branchEndL, testNorm, branchColor4);

        //
        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, testNormL, branchColor3);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, testNormR, branchColor3);
        VertexPositionNormalColor branchBaseLower(branchBaseLowerVert, testNorm, branchColor2);
        VertexPositionNormalColor branchBaseLowerLite(branchBaseLowerVert, testNorm, branchColor5);

        VertexPositionNormalColor branchBaseLowerL(branchBaseLowerVert, testNormL, branchColor2);
        VertexPositionNormalColor branchBaseLowerLiteL(branchBaseLowerVert, testNormL, branchColor5);
        VertexPositionNormalColor branchBaseLowerR(branchBaseLowerVert, testNormR, branchColor2);
        VertexPositionNormalColor branchBaseLowerLiteR(branchBaseLowerVert, testNormR, branchColor5);

        m_batch2->DrawTriangle(leafL, branchBaseLowerL, branchBaseVertexLiteL);
        m_batch2->DrawTriangle(leafRightLower, branchBaseVertexR, branchBaseLowerR);

        if (i > 28)
        {
            testTop = branchBaseLowerVert;
            testLeafL = branchEndL;
            testLeafR = branchEndR;
        }
    }

    VertexPositionNormalColor topVertL(testTop, testNormL, branchColor1);
    VertexPositionNormalColor topVertR(testTop, testNormR, branchColor1);

    VertexPositionNormalColor baseVertL(testBase, testNorm, branchColor1);
    VertexPositionNormalColor baseVertR(testBase, testNorm, branchColor1);

    testNormL = DirectX::SimpleMath::Vector3::UnitY;
    testNormR = DirectX::SimpleMath::Vector3::UnitY;
    VertexPositionNormalColor leafVertL(testLeafL, testNormL, branchColor1);
    VertexPositionNormalColor leafVertR(testLeafR, testNormR, branchColor1);

    float posOffset = 0.3;
    topVertL.position.y += posOffset;
    baseVertL.position.y += posOffset;
    leafVertL.position.y += posOffset;
    topVertR.position.y += posOffset;
    baseVertR.position.y += posOffset;
    leafVertR.position.y += posOffset;

    m_batch2->DrawTriangle(topVertL, baseVertL, leafVertL);
    m_batch2->DrawTriangle(topVertR, baseVertR, leafVertR);
}


void Game::DrawTree07(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 0.9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    const float branchGap = .013;
    const float halfBranchGap = branchGap * .5;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap * 7;
    branchBase.y += branchGap;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Red;

    float widthMod = 1.2f;
    const int layerCount = 25;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionColor leafR(branchEndR, branchColor2);
        VertexPositionColor leafRTest(branchEndR, branchColor1);
        VertexPositionColor leafL(branchEndL, branchColor2);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor1);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor2);     
        VertexPositionColor leafRightLowerTest(branchEndRightLower, branchColor2);
        VertexPositionColor branchBaseLower(branchBaseLowerVert, branchColor1);
        VertexPositionColor branchBaseLowerRight(branchBaseLowerVert, branchColor2);

        //m_batch->DrawQuad(leafL, leafLeftLower, branchBaseLower, branchBaseVertex);
        m_batch->DrawQuad(leafLeftLower, leafL, branchBaseLower, branchBaseVertex);
        //m_batch->DrawQuad(leafR, branchBaseVertex, branchBaseLower, leafRightLower);
        //m_batch->DrawQuad(leafRightLower, leafR, branchBaseVertex, leafRightLower);
        m_batch->DrawQuad(leafRightLower, leafRTest, branchBaseLowerRight, branchBaseVertex);
     
        m_batch->DrawLine(branchBaseVertex, leafRightLower);
        //m_batch->DrawLine(branchBaseVertex, leafL);
    }
}

void Game::DrawTree07Test1(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 0.9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine = aTreePos - pCamera->GetPos();
    viewLine.y = 0.0;
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    const float branchGap = .013;
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 trunkNorm = viewLine;
    VertexPositionNormalColor treeRootTop2(baseTop, trunkNorm, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap * 7;
    branchBase.y += branchGap;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Red;
    /*
    const float branchPitchL = Utility::ToRadians(0.0);
    const float branchYawL = Utility::ToRadians((0.0));
    const float branchRollL = Utility::ToRadians((-135.0 - 180.));
    const float branchPitchR = Utility::ToRadians(0.0);
    const float branchYawR = Utility::ToRadians((-135.0));
    const float branchRollR = Utility::ToRadians((-135.0 - 180.));
    */

    const float branchYawL = Utility::ToRadians((150.0));
    const float branchPitchL = Utility::ToRadians(30.0);
    const float branchRollL = Utility::ToRadians((0.0));
    const float branchYawR = Utility::ToRadians((30.0));
    const float branchPitchR = Utility::ToRadians(0.0);   
    const float branchRollR = Utility::ToRadians((-40.0));
    auto turnQuatLeft = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawL, branchPitchL, branchRollL);
    auto turnQuatRight = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawR, branchPitchR, branchRollR);
    DirectX::SimpleMath::Vector3 leafNormLeft = XMVector3Rotate(viewHorizontal, turnQuatLeft);
    DirectX::SimpleMath::Vector3 leafNormRight = XMVector3Rotate(-viewHorizontal, turnQuatRight);
    //leafNormLeft = -viewHorizontal;
    //leafNormRight = viewHorizontal;

    VertexPositionNormalColor testLeft((leafNormLeft * 1.0) + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Yellow);
    VertexPositionNormalColor testRight((leafNormRight * 1.0) + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Red);
    VertexPositionNormalColor testHor(viewHorizontal + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor testViewLine(viewLine + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Blue);
    VertexPositionNormalColor testBase(aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::ForestGreen);
    m_batch2->DrawLine(testBase, testLeft);
    m_batch2->DrawLine(testBase, testRight);

    m_batch2->DrawLine(testBase, testHor);
    m_batch2->DrawLine(testBase, testViewLine);

    DirectX::SimpleMath::Vector3 testTop;
    DirectX::SimpleMath::Vector3 testBase2 = branchBase;
    DirectX::SimpleMath::Vector3 testLeafL;
    DirectX::SimpleMath::Vector3 testLeafR;

    float widthMod = 1.2f;
    const int layerCount = 25;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, DirectX::SimpleMath::Vector3:: UnitY, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionNormalColor leafR(branchEndR, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRTest(branchEndR, leafNormRight, branchColor1);
        VertexPositionNormalColor leafL(branchEndL, leafNormLeft, branchColor2);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, leafNormLeft, branchColor1);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRightLowerTest(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor branchBaseLower(branchBaseLowerVert, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
        VertexPositionNormalColor branchBaseLowerRight(branchBaseLowerVert, leafNormRight, branchColor2);

        //m_batch->DrawQuad(leafL, leafLeftLower, branchBaseLower, branchBaseVertex);
        m_batch2->DrawQuad(leafLeftLower, leafL, branchBaseLower, branchBaseVertex);
        //m_batch->DrawQuad(leafR, branchBaseVertex, branchBaseLower, leafRightLower);
        //m_batch->DrawQuad(leafRightLower, leafR, branchBaseVertex, leafRightLower);
        m_batch2->DrawQuad(leafRightLower, leafRTest, branchBaseLowerRight, branchBaseVertex);

        m_batch2->DrawLine(branchBaseVertex, leafRightLower);
        //m_batch->DrawLine(branchBaseVertex, leafL);

        // Test verts
        if (i > 28)
        {
            /*
            testTop = branchBaseLowerVert;
            testLeafL = branchEndL;
            testLeafR = branchEndR;
            */
            testTop = branchBaseLowerVert;
            //testLeafL = branchEndLeftLower;
            //testLeafR = branchEndRightLower;
        }
        if (i < 2)
        {
            testLeafL = branchEndLeftLower;
            testLeafR = branchEndRightLower;
        }
    }
    testTop = testBase2;
    testTop.y += .4;
    VertexPositionNormalColor topVertL(testTop, leafNormLeft, branchColor1);
    VertexPositionNormalColor topVertR(testTop, leafNormRight, branchColor1);

    VertexPositionNormalColor baseVertL(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
    VertexPositionNormalColor baseVertR(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);

    //leafNormLeft = DirectX::SimpleMath::Vector3::UnitY;
    //leafNormRight = DirectX::SimpleMath::Vector3::UnitY;
    VertexPositionNormalColor leafVertL(testLeafL, leafNormLeft, branchColor1);
    VertexPositionNormalColor leafVertR(testLeafR, leafNormRight, branchColor1);
    
    float posOffset = 0.3;
    topVertL.position.y += posOffset;
    baseVertL.position.y += posOffset;
    leafVertL.position.y += posOffset;
    topVertR.position.y += posOffset;
    baseVertR.position.y += posOffset;
    leafVertR.position.y += posOffset;
    
    m_batch2->DrawTriangle(topVertL, baseVertL, leafVertL);
    m_batch2->DrawTriangle(topVertR, baseVertR, leafVertR);

}

void Game::DrawTree07Test2(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 0.9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine = aTreePos - pCamera->GetPos();
    viewLine.y = 0.0;
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    const float branchGap = .013;
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 trunkNorm = viewLine;
    VertexPositionNormalColor treeRootTop2(baseTop, trunkNorm, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap * 7;
    branchBase.y += branchGap;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Red;

    // Test lighting norms 1
    /*
    const float branchPitchL = Utility::ToRadians(0.0);
    const float branchYawL = Utility::ToRadians((0.0));
    const float branchRollL = Utility::ToRadians((-135.0 - 180.));
    const float branchPitchR = Utility::ToRadians(0.0);
    const float branchYawR = Utility::ToRadians((-135.0));
    const float branchRollR = Utility::ToRadians((-135.0 - 180.));
    */

    const float branchYawL = Utility::ToRadians((150.0));
    const float branchPitchL = Utility::ToRadians(30.0);
    const float branchRollL = Utility::ToRadians((0.0));
    const float branchYawR = Utility::ToRadians((30.0));
    const float branchPitchR = Utility::ToRadians(0.0);
    const float branchRollR = Utility::ToRadians((-40.0));
    
    auto turnQuatLeft = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawL, branchPitchL, branchRollL);
    auto turnQuatRight = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawR, branchPitchR, branchRollR);
    DirectX::SimpleMath::Vector3 leafNormLeft = XMVector3Rotate(viewHorizontal, turnQuatLeft);
    DirectX::SimpleMath::Vector3 leafNormRight = XMVector3Rotate(-viewHorizontal, turnQuatRight);
    //leafNormLeft = -viewHorizontal;
    //leafNormRight = viewHorizontal;

    // End test lighting norms 1
    // Test light norms 2
    DirectX::SimpleMath::Vector3 testLeafNormRight = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixRight = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRight);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateRight = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRotateRight);

    testLeafNormRight.Normalize();
    VertexPositionNormalColor leafNormPointRight(baseTop + testLeafNormRight, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointRight);

    DirectX::SimpleMath::Vector3 testLeafNormLeft = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixLeft = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-135.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateLeft = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(45.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixRotateLeft);

    testLeafMatrixLeft *= testLeafMatrixRotateLeft;
    testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);

    testLeafNormLeft.Normalize();
    VertexPositionNormalColor leafNormPointLeft(baseTop + testLeafNormLeft, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointLeft);

    DirectX::SimpleMath::Vector3 testNormL = testLeafNormLeft;
    DirectX::SimpleMath::Vector3 testNormR = testLeafNormRight;
    //testNormL = - DirectX::SimpleMath::Vector3::UnitX;
    //testNormR = - DirectX::SimpleMath::Vector3::UnitX;
    leafNormLeft = testNormL;
    leafNormRight = testNormR;
    // End test lighting norms 2
    
    VertexPositionNormalColor testLeft(leafNormLeft  + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Yellow);
    VertexPositionNormalColor testRight((leafNormRight * 1.0) + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Red);
    VertexPositionNormalColor testHor(viewHorizontal + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor testViewLine(viewLine + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Blue);
    VertexPositionNormalColor testBase(aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::ForestGreen);
    m_batch2->DrawLine(testBase, testLeft);
    m_batch2->DrawLine(testBase, testRight);

    m_batch2->DrawLine(testBase, testHor);
    m_batch2->DrawLine(testBase, testViewLine);

    DirectX::SimpleMath::Vector3 testTop;
    DirectX::SimpleMath::Vector3 testBase2 = branchBase;
    DirectX::SimpleMath::Vector3 testLeafL;
    DirectX::SimpleMath::Vector3 testLeafR;


    float widthMod = 1.2f;
    const int layerCount = 25;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, DirectX::SimpleMath::Vector3::UnitY, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionNormalColor leafR(branchEndR, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRTest(branchEndR, leafNormRight, branchColor1);
        VertexPositionNormalColor leafL(branchEndL, leafNormLeft, branchColor2);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, leafNormLeft, branchColor1);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRightLowerTest(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor branchBaseLower(branchBaseLowerVert, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
        VertexPositionNormalColor branchBaseLowerRight(branchBaseLowerVert, leafNormRight, branchColor2);

        //m_batch->DrawQuad(leafL, leafLeftLower, branchBaseLower, branchBaseVertex);
        m_batch2->DrawQuad(leafLeftLower, leafL, branchBaseLower, branchBaseVertex);
        //m_batch->DrawQuad(leafR, branchBaseVertex, branchBaseLower, leafRightLower);
        //m_batch->DrawQuad(leafRightLower, leafR, branchBaseVertex, leafRightLower);
        m_batch2->DrawQuad(leafRightLower, leafRTest, branchBaseLowerRight, branchBaseVertex);

        m_batch2->DrawLine(branchBaseVertex, leafRightLower);
        //m_batch->DrawLine(branchBaseVertex, leafL);

        // Test verts
        if (i > 28)
        {
            /*
            testTop = branchBaseLowerVert;
            testLeafL = branchEndL;
            testLeafR = branchEndR;
            */
            testTop = branchBaseLowerVert;
            //testLeafL = branchEndLeftLower;
            //testLeafR = branchEndRightLower;
        }
        if (i < 2)
        {
            testLeafL = branchEndLeftLower;
            testLeafR = branchEndRightLower;
        }
    }
    testTop = testBase2;
    testTop.y += .4;
    VertexPositionNormalColor topVertL(testTop, leafNormLeft, branchColor1);
    VertexPositionNormalColor topVertR(testTop, leafNormRight, branchColor1);

    VertexPositionNormalColor baseVertL(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
    VertexPositionNormalColor baseVertR(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);

    //leafNormLeft = DirectX::SimpleMath::Vector3::UnitY;
    //leafNormRight = DirectX::SimpleMath::Vector3::UnitY;
    VertexPositionNormalColor leafVertL(testLeafL, leafNormLeft, branchColor1);
    VertexPositionNormalColor leafVertR(testLeafR, leafNormRight, branchColor1);

    float posOffset = 0.3;
    topVertL.position.y += posOffset;
    baseVertL.position.y += posOffset;
    leafVertL.position.y += posOffset;
    topVertR.position.y += posOffset;
    baseVertR.position.y += posOffset;
    leafVertR.position.y += posOffset;

    m_batch2->DrawTriangle(topVertL, baseVertL, leafVertL);
    m_batch2->DrawTriangle(topVertR, baseVertR, leafVertR);

}

void Game::DrawTree07Test3(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = 0.9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine = aTreePos - pCamera->GetPos();
    viewLine.y = 0.0;
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    const float branchGap = .013;
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 trunkNorm = viewLine;
    VertexPositionNormalColor treeRootTop2(baseTop, trunkNorm, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNorm, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNorm, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNorm, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNorm, Colors::Gray);

    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap * 7;
    branchBase.y += branchGap;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::Red;

    // Test lighting norms 1
    /*
    //const float branchPitchL = Utility::ToRadians(0.0);
    //const float branchYawL = Utility::ToRadians((0.0));
    //const float branchRollL = Utility::ToRadians((-135.0 - 180.));
    //const float branchPitchR = Utility::ToRadians(0.0);
    //const float branchYawR = Utility::ToRadians((-135.0));
    //const float branchRollR = Utility::ToRadians((-135.0 - 180.));
    
    const float branchYawL = Utility::ToRadians((150.0));
    const float branchPitchL = Utility::ToRadians(30.0);
    const float branchRollL = Utility::ToRadians((0.0));
    const float branchYawR = Utility::ToRadians((30.0));
    const float branchPitchR = Utility::ToRadians(0.0);
    const float branchRollR = Utility::ToRadians((-40.0));

    auto turnQuatLeft = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawL, branchPitchL, branchRollL);
    auto turnQuatRight = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(branchYawR, branchPitchR, branchRollR);
    DirectX::SimpleMath::Vector3 leafNormLeft = XMVector3Rotate(viewHorizontal, turnQuatLeft);
    DirectX::SimpleMath::Vector3 leafNormRight = XMVector3Rotate(-viewHorizontal, turnQuatRight);
    //leafNormLeft = -viewHorizontal;
    //leafNormRight = viewHorizontal;
    */
    // End test lighting norms 1
    // Test light norms 2
    DirectX::SimpleMath::Vector3 testLeafNormRight = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixRight = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRight);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateRight = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRotateRight);

    testLeafNormRight.Normalize();
    VertexPositionNormalColor leafNormPointRight(baseTop + testLeafNormRight, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointRight);

    DirectX::SimpleMath::Vector3 testLeafNormLeft = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixLeft = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-135.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateLeft = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(45.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixRotateLeft);

    testLeafMatrixLeft *= testLeafMatrixRotateLeft;
    testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);

    testLeafNormLeft.Normalize();
    VertexPositionNormalColor leafNormPointLeft(baseTop + testLeafNormLeft, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    m_batch2->DrawLine(basePoint, leafNormPointLeft);

    DirectX::SimpleMath::Vector3 testNormL = testLeafNormLeft;
    DirectX::SimpleMath::Vector3 testNormR = testLeafNormRight;
    //testNormL = - DirectX::SimpleMath::Vector3::UnitX;
    //testNormR = - DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3 leafNormLeft = testNormL;
    DirectX::SimpleMath::Vector3 leafNormRight = testNormR;
    // End test lighting norms 2

    VertexPositionNormalColor testLeft(leafNormLeft + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Yellow);
    VertexPositionNormalColor testRight((leafNormRight * 1.0) + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Red);
    VertexPositionNormalColor testHor(viewHorizontal + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor testViewLine(viewLine + aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::Blue);
    VertexPositionNormalColor testBase(aTreePos, DirectX::SimpleMath::Vector3::UnitY, Colors::ForestGreen);
    m_batch2->DrawLine(testBase, testLeft);
    m_batch2->DrawLine(testBase, testRight);

    m_batch2->DrawLine(testBase, testHor);
    m_batch2->DrawLine(testBase, testViewLine);

    DirectX::SimpleMath::Vector3 testTop;
    DirectX::SimpleMath::Vector3 testBase2 = branchBase;
    DirectX::SimpleMath::Vector3 testLeafL;
    DirectX::SimpleMath::Vector3 testLeafR;


    float widthMod = 1.2f;
    const int layerCount = 25;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= 1.05;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, DirectX::SimpleMath::Vector3::UnitY, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionNormalColor leafR(branchEndR, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRTest(branchEndR, leafNormRight, branchColor1);
        VertexPositionNormalColor leafL(branchEndL, leafNormLeft, branchColor2);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchBaseLowerVert = branchBase;
        branchBaseLowerVert.y -= halfBranchGap;

        VertexPositionNormalColor leafLeftLower(branchEndLeftLower, leafNormLeft, branchColor1);
        VertexPositionNormalColor leafRightLower(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor leafRightLowerTest(branchEndRightLower, leafNormRight, branchColor2);
        VertexPositionNormalColor branchBaseLower(branchBaseLowerVert, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
        VertexPositionNormalColor branchBaseLowerRight(branchBaseLowerVert, leafNormRight, branchColor2);

        //m_batch->DrawQuad(leafL, leafLeftLower, branchBaseLower, branchBaseVertex);
        m_batch2->DrawQuad(leafLeftLower, leafL, branchBaseLower, branchBaseVertex);
        //m_batch->DrawQuad(leafR, branchBaseVertex, branchBaseLower, leafRightLower);
        //m_batch->DrawQuad(leafRightLower, leafR, branchBaseVertex, leafRightLower);
        m_batch2->DrawQuad(leafRightLower, leafRTest, branchBaseLowerRight, branchBaseVertex);

        m_batch2->DrawLine(branchBaseVertex, leafRightLower);
        //m_batch->DrawLine(branchBaseVertex, leafL);

        // Test verts
        if (i > 28)
        {
            /*
            testTop = branchBaseLowerVert;
            testLeafL = branchEndL;
            testLeafR = branchEndR;
            */
            testTop = branchBaseLowerVert;
            //testLeafL = branchEndLeftLower;
            //testLeafR = branchEndRightLower;
        }
        if (i < 2)
        {
            testLeafL = branchEndLeftLower;
            testLeafR = branchEndRightLower;
        }
    }
    testTop = testBase2;
    testTop.y += .4;
    VertexPositionNormalColor topVertL(testTop, leafNormLeft, branchColor1);
    VertexPositionNormalColor topVertR(testTop, leafNormRight, branchColor1);

    VertexPositionNormalColor baseVertL(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);
    VertexPositionNormalColor baseVertR(testBase2, DirectX::SimpleMath::Vector3::UnitY, branchColor1);

    //leafNormLeft = DirectX::SimpleMath::Vector3::UnitY;
    //leafNormRight = DirectX::SimpleMath::Vector3::UnitY;
    VertexPositionNormalColor leafVertL(testLeafL, leafNormLeft, branchColor1);
    VertexPositionNormalColor leafVertR(testLeafR, leafNormRight, branchColor1);

    float posOffset = 0.3;
    topVertL.position.y += posOffset;
    baseVertL.position.y += posOffset;
    leafVertL.position.y += posOffset;
    topVertR.position.y += posOffset;
    baseVertR.position.y += posOffset;
    leafVertR.position.y += posOffset;

    m_batch2->DrawTriangle(topVertL, baseVertL, leafVertL);
    m_batch2->DrawTriangle(topVertR, baseVertR, leafVertR);

}

void Game::DrawTree08(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    baseTop.y += .08;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    baseTop.y -= branchGap;
    branchBase.y += branchGap;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::White;
    DirectX::SimpleMath::Vector3 prevBBV = branchBase;

    VertexPositionColor prevBranchBaseVertex(prevBBV, branchColor2);
    const float heightLayer = 1.05f;
    float widthMod = 1.0f;
    const int layerCount = 20;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= heightLayer;
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);

        if (i > 1)
        {
            m_batch->DrawLine(branchBaseVertex, leafR);
            m_batch->DrawLine(branchBaseVertex, leafL);

            //////

            m_batch->DrawLine(prevBranchBaseVertex, leafR);
            m_batch->DrawLine(prevBranchBaseVertex, leafL);
        }
        prevBranchBaseVertex = branchBaseVertex;
    }
}

void Game::DrawTree09(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    const float trunkLength = .1;
    baseTop.y += trunkLength;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////
    // tree trunk

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    //m_batch->DrawLine(treeRootBase, treeRootTop);
    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);

    baseTop.y -= branchGap * 7;  /// adjust the angle of the tree branches and overall tree pointy'ness

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::ForestGreen;
    DirectX::SimpleMath::Vector3 prevBBV = branchBase;

    VertexPositionColor prevBranchBaseVertex(prevBBV, branchColor2);
    const float heightLayer = 1.05f;
    float widthMod = 1.1f;
    const int layerCount = 24;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= heightLayer;
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::DarkGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionColor leafR(branchEndR, Colors::DarkGreen);
        VertexPositionColor leafL(branchEndL, Colors::LawnGreen);

        VertexPositionColor branchBaseBackground = branchBaseVertex;
        //VertexPositionColor branchBaseBackground(branchBase, Colors::Black);

        //VertexPositionColor prevBranchBaseBackground = prevBranchBaseVertex;
        VertexPositionColor prevBranchBaseBackground(branchBase, Colors::Black);

        VertexPositionColor leafRBackground(branchEndR, Colors::PaleGreen);
        VertexPositionColor leafLBackground(branchEndL, Colors::SpringGreen);

        //prevBranchBaseVertex = branchBaseVertex;

        m_batch->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafRBackground);
        m_batch->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafLBackground);
        
        //m_batch->DrawLine(branchBaseVertex, leafR);
        //m_batch->DrawLine(branchBaseVertex, leafL);

        //////

        //m_batch->DrawLine(prevBranchBaseVertex, leafR);
        //m_batch->DrawLine(prevBranchBaseVertex, leafL);

        prevBranchBaseVertex = branchBaseVertex;
    }
}

void Game::DrawTree09Test1(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    const float trunkLength = .1;
    baseTop.y += trunkLength;

    //DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    DirectX::SimpleMath::Vector3 viewLine = aTreePos - pCamera->GetPos();
    viewLine.y = 0.0;
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);
    DirectX::SimpleMath::Vector3 trunkNormal = viewLine;
    VertexPositionNormalColor treeRootBase(aTreePos, trunkNormal, Colors::Gray);
    VertexPositionNormalColor treeRootTop(baseTop, trunkNormal, Colors::Gray);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionNormalColor treeRootTop2(baseTop, trunkNormal, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////
    // tree trunk

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionNormalColor trunkBackTL(trunkTopLeft, trunkNormal, Colors::Black);
    VertexPositionNormalColor trunkBackTR(trunkTopRight, trunkNormal, Colors::Black);
    VertexPositionNormalColor trunkBackBR(trunkBottomRight, trunkNormal, Colors::Black);
    VertexPositionNormalColor trunkBackBL(trunkBottomLeft, trunkNormal, Colors::Black);

    m_batch2->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionNormalColor trunkTL(trunkTopLeft, trunkNormal, Colors::Gray);
    VertexPositionNormalColor trunkTR(trunkTopRight, trunkNormal, Colors::Gray);
    VertexPositionNormalColor trunkBR(trunkBottomRight, trunkNormal, Colors::Gray);
    VertexPositionNormalColor trunkBL(trunkBottomLeft, trunkNormal, Colors::Gray);

    //m_batch->DrawLine(treeRootBase, treeRootTop);
    m_batch2->DrawLine(trunkTL, trunkBL);
    m_batch2->DrawLine(trunkTR, trunkBR);

    // Test light norms 1
    DirectX::SimpleMath::Vector3 testLeafNormRight = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixRight = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRight);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateRight = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(-45.0));
    testLeafNormRight = DirectX::SimpleMath::Vector3::Transform(testLeafNormRight, testLeafMatrixRotateRight);

    testLeafNormRight.Normalize();
    VertexPositionNormalColor leafNormPointRight(baseTop + testLeafNormRight, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //m_batch2->DrawLine(basePoint, leafNormPointRight);

    DirectX::SimpleMath::Vector3 testLeafNormLeft = viewHorizontal;
    DirectX::SimpleMath::Matrix testLeafMatrixLeft = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(viewLine, Utility::ToRadians(-135.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);
    DirectX::SimpleMath::Matrix testLeafMatrixRotateLeft = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(45.0));
    //testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixRotateLeft);

    testLeafMatrixLeft *= testLeafMatrixRotateLeft;
    testLeafNormLeft = DirectX::SimpleMath::Vector3::Transform(testLeafNormLeft, testLeafMatrixLeft);

    testLeafNormLeft.Normalize();
    VertexPositionNormalColor leafNormPointLeft(baseTop + testLeafNormLeft, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //VertexPositionNormalColor basePoint(baseTop, DirectX::SimpleMath::Vector3::UnitY, Colors::White);
    //m_batch2->DrawLine(basePoint, leafNormPointLeft);

    DirectX::SimpleMath::Vector3 testNormL = testLeafNormLeft;
    DirectX::SimpleMath::Vector3 testNormR = testLeafNormRight;
    //testNormL = - DirectX::SimpleMath::Vector3::UnitX;
    //testNormR = - DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3 leafNormLeft = testNormL;
    DirectX::SimpleMath::Vector3 leafNormRight = testNormR;
    // End test lighting norms 1

    baseTop.y -= branchGap * 7;  /// adjust the angle of the tree branches and overall tree pointy'ness

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Green;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::ForestGreen;
    DirectX::SimpleMath::Vector3 prevBBV = branchBase;

    VertexPositionNormalColor prevBranchBaseVertex(prevBBV, DirectX::SimpleMath::Vector3::UnitY, branchColor2);
    const float heightLayer = 1.05f;
    float widthMod = 1.1f;
    const int layerCount = 24;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= heightLayer;
        branchBase += swayBase;
        VertexPositionNormalColor branchBaseVertex(branchBase, DirectX::SimpleMath::Vector3::UnitY, Colors::DarkGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionNormalColor leafR(branchEndR, leafNormRight, Colors::DarkGreen);
        VertexPositionNormalColor leafL(branchEndL, leafNormLeft, Colors::LawnGreen);

        VertexPositionNormalColor branchBaseBackground = branchBaseVertex;
        //VertexPositionColor branchBaseBackground(branchBase, Colors::Black);

        //VertexPositionColor prevBranchBaseBackground = prevBranchBaseVertex;
        VertexPositionNormalColor prevBranchBaseBackground(branchBase, DirectX::SimpleMath::Vector3::UnitY, Colors::Black);

        VertexPositionNormalColor leafRBackground(branchEndR, leafNormRight, Colors::PaleGreen);
        VertexPositionNormalColor leafLBackground(branchEndL, leafNormLeft, Colors::SpringGreen);

        //prevBranchBaseVertex = branchBaseVertex;

        m_batch2->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafRBackground);
        m_batch2->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafLBackground);

        //m_batch->DrawLine(branchBaseVertex, leafR);
        //m_batch->DrawLine(branchBaseVertex, leafL);

        //////

        //m_batch->DrawLine(prevBranchBaseVertex, leafR);
        //m_batch->DrawLine(prevBranchBaseVertex, leafL);

        prevBranchBaseVertex = branchBaseVertex;
    }
}

void Game::DrawTree10(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
{
    DirectX::SimpleMath::Vector3 windVector = pGolf->GetEnvironWindVector();
    double windDirection = pGolf->GetWindDirectionRad();
    double windSpeed = windVector.Length();
    DirectX::SimpleMath::Vector3 windNormalized = windVector;
    windNormalized.Normalize();
    const float scaleMod = .9;
    float scale = pGolf->GetEnvironScale() * scaleMod;

    DirectX::SimpleMath::Vector3 swayVec = windNormalized * scale * (cosf(static_cast<float>((m_timer.GetTotalSeconds() + aVariation) * windSpeed * .1)));
    swayVec += windVector * 0.003;

    DirectX::SimpleMath::Vector3 swayBase = swayVec;
    swayBase = swayBase * 0.05;

    DirectX::SimpleMath::Vector3 baseTop = aTreePos;
    const float trunkLength = .08;
    baseTop.y += trunkLength;

    DirectX::SimpleMath::Vector3 viewLine = pCamera->GetTargetPos() - pCamera->GetPos();
    viewLine.Normalize();

    DirectX::SimpleMath::Vector3 viewHorizontal = DirectX::XMVector3Cross((aTreePos - baseTop), viewLine);

    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(baseTop, Colors::Gray);
    //m_batch->DrawLine(treeRootBase, treeRootTop);

    const float branchGap = .01;

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::DarkGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    ///////////////////////////////////////////
    // tree trunk

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .04f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .04f);

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(treeRootBase, treeRootTop);
    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);


    baseTop.y -= branchGap * 7;  /// adjust the angle of the tree branches and overall tree pointy'ness

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::ForestGreen;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::SimpleMath::Vector3 prevBBV = branchBase;

    VertexPositionColor prevBranchBaseVertex(prevBBV, branchColor2);
    const float heightLayer = 1.05f;
    float widthMod = 1.0f;
    const int layerCount = 4;
    for (int i = 1; i < layerCount; ++i)
    {
        swayBase *= heightLayer;
        branchBase += swayBase;
        VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
        branchBase.y += branchGap;

        baseTop.y += branchGap;

        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + baseTop;
        branchEndR.y += branchGap;
        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + baseTop;
        branchEndL.y += branchGap;
        branchEndL += swayVec;

        widthMod -= .033f;

        VertexPositionColor leafR(branchEndR, Colors::ForestGreen);
        VertexPositionColor leafL(branchEndL, Colors::ForestGreen);


        VertexPositionColor branchBaseBackground = branchBaseVertex;
        //VertexPositionColor branchBaseBackground(branchBase, Colors::Black);

        //VertexPositionColor prevBranchBaseBackground = prevBranchBaseVertex;
        VertexPositionColor prevBranchBaseBackground(branchBase, Colors::Black);


        VertexPositionColor leafRBackground(branchEndR, Colors::Black);
        VertexPositionColor leafLBackground(branchEndL, Colors::Black);

        //prevBranchBaseVertex = branchBaseVertex;

        m_batch->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafRBackground);
        m_batch->DrawTriangle(branchBaseBackground, prevBranchBaseBackground, leafLBackground);

        m_batch->DrawLine(branchBaseVertex, leafR);
        m_batch->DrawLine(branchBaseVertex, leafL);

        //////

        m_batch->DrawLine(prevBranchBaseVertex, leafR);
        m_batch->DrawLine(prevBranchBaseVertex, leafL);

        prevBranchBaseVertex = branchBaseVertex;
    }
}

void Game::DrawTree11(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation)
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
    const float halfBranchGap = branchGap * .5;

    DirectX::SimpleMath::Vector3 testBaseTop = baseTop;

    testBaseTop += swayBase;
    VertexPositionColor treeRootBase(aTreePos, Colors::Gray);
    VertexPositionColor treeRootTop(testBaseTop, Colors::Gray);

    DirectX::SimpleMath::Vector3 trunkTopLeft = baseTop + (viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkTopRight = baseTop + (-viewHorizontal * .05f);
    DirectX::SimpleMath::Vector3 trunkBottomRight = aTreePos + (-viewHorizontal * .09f);
    DirectX::SimpleMath::Vector3 trunkBottomLeft = aTreePos + (viewHorizontal * .09f);


    trunkTopLeft += swayBase;
    trunkTopRight += swayBase;
    //trunkBottomRight += swayBase;
    //trunkBottomLeft += swayBase;

    /// /////
    /// tree trunk

    VertexPositionColor trunkBackTL(trunkTopLeft, Colors::Black);
    VertexPositionColor trunkBackTR(trunkTopRight, Colors::Black);
    VertexPositionColor trunkBackBR(trunkBottomRight, Colors::Black);
    VertexPositionColor trunkBackBL(trunkBottomLeft, Colors::Black);

    m_batch->DrawQuad(trunkBackTL, trunkBackBL, trunkBackBR, trunkBackTR);

    VertexPositionColor trunkTL(trunkTopLeft, Colors::Gray);
    VertexPositionColor trunkTR(trunkTopRight, Colors::Gray);
    VertexPositionColor trunkBR(trunkBottomRight, Colors::Gray);
    VertexPositionColor trunkBL(trunkBottomLeft, Colors::Gray);

    m_batch->DrawLine(trunkTL, trunkBL);
    m_batch->DrawLine(trunkTR, trunkBR);
    m_batch->DrawLine(treeRootBase, treeRootTop);

    VertexPositionColor treeRootTop2(baseTop, DirectX::Colors::ForestGreen);
    DirectX::XMVECTORF32 leafColor = DirectX::Colors::ForestGreen;

    DirectX::SimpleMath::Vector3 branchBase = baseTop;

    VertexPositionColor branchBaseVertex(branchBase, Colors::ForestGreen);
    DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal + branchBase;


    branchEndR += swayVec;
    DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal + branchBase;

    DirectX::XMVECTORF32 branchColor1 = DirectX::Colors::Orange;
    DirectX::XMVECTORF32 branchColor2 = DirectX::Colors::Black;
    DirectX::XMVECTORF32 branchColor3 = DirectX::Colors::GreenYellow;

    branchEndL += swayVec;
    VertexPositionColor leafR(branchEndR, branchColor1);
    VertexPositionColor leafL(branchEndL, branchColor1);
    m_batch->DrawLine(branchBaseVertex, leafR);
    m_batch->DrawLine(branchBaseVertex, leafL);

    //// increase width
    float widthMod = 1.0;
    int layerHeight = 6;
    for (int i = 0; i < layerHeight * 3; ++i)
    {
        DirectX::SimpleMath::Vector3 branchEndR = viewHorizontal * widthMod + branchBase;

        branchEndR += swayVec;
        DirectX::SimpleMath::Vector3 branchEndL = -viewHorizontal * widthMod + branchBase;

        branchEndL += swayVec;
        VertexPositionColor leafR(branchEndR, branchColor1);
        VertexPositionColor leafL(branchEndL, branchColor3);
        VertexPositionColor branchRoot(branchBase, branchColor1);

        DirectX::SimpleMath::Vector3 branchEndLeftLower = branchEndL;
        branchEndLeftLower.y -= halfBranchGap;
        DirectX::SimpleMath::Vector3 branchEndRightLower = branchEndR;
        branchEndRightLower.y -= halfBranchGap;

        VertexPositionColor leafLeftLower(branchEndLeftLower, branchColor2);
        VertexPositionColor leafRightLower(branchEndRightLower, branchColor2);
        m_batch->DrawQuad(leafL, leafR, leafRightLower, leafLeftLower);

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
}

void Game::DrawUI()
{
    if (m_currentUiState == UiState::UISTATE_SWING)
    {
        std::vector<std::string> uiString = pGolf->GetUISwingStrings();
        DirectX::SimpleMath::Vector2 fontPos = m_fontPos2;
        for (int i = 0; i < uiString.size(); ++i)
        {
            std::string uiLine = std::string(uiString[i]);
            DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
            m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
            fontPos.y += 35;
        }

        // temp for testing swing count
        std::string uiLine = "Swing Count = " + std::to_string(pPlay->GetSwingCount());
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
        fontPos.y += 35;

        // temp for testing terrain height
        /*
        uiLine = "Height = " + std::to_string(m_debugHeight);
        lineOrigin = m_font->MeasureString(uiLine.c_str());
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
        fontPos.y += 35;

        uiLine = "X = " + std::to_string(m_debugXpoint);
        lineOrigin = m_font->MeasureString(uiLine.c_str());
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
        fontPos.y += 35;

        uiLine = "Z = " + std::to_string(m_debugZpoint);
        lineOrigin = m_font->MeasureString(uiLine.c_str());
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
        fontPos.y += 35;
        */
    }

    if (m_currentUiState == UiState::UISTATE_SHOT)
    {
        std::vector<std::string> uiString = pGolf->GetUIShotStrings();
        DirectX::SimpleMath::Vector2 fontPos = m_fontPos2;
        for (int i = 0; i < uiString.size(); ++i)
        {
            std::string uiLine = std::string(uiString[i]);
            DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
            m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
            fontPos.y += 35;
        }

        // drawing text and numbers seperatly to prevent text from shifting postion as the numbes change
        std::string shotString("Shot Distance =               Yards ");
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(shotString.c_str());
        m_font->DrawString(m_spriteBatch.get(), shotString.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
        std::string distanceString = pGolf->GetShotDistanceString();
        lineOrigin = m_font->MeasureString(distanceString.c_str());
        fontPos.x -= 130;
        m_font->DrawString(m_spriteBatch.get(), distanceString.c_str(), fontPos, Colors::White, 0.f, lineOrigin);
    }

    if (m_currentUiState == UiState::UISTATE_SCORE)
    {
        std::string uiLine = pPlay->GetUIScoreString();
        DirectX::SimpleMath::Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
        //DirectX::SimpleMath::Vector2 lineOrigin = m_titleFont->MeasureString(uiLine.c_str());
        //DirectX::SimpleMath::Vector2 scorePos = m_bitwiseFontPos;
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_bitwiseFontPos, Colors::White, 0.f, lineOrigin);
    }
}

void Game::DrawWater()
{
    DirectX::XMVECTORF32 waterColor = DirectX::Colors::Blue;
    DirectX::XMVECTORF32 waterColorDeep = DirectX::Colors::Navy;
    DirectX::XMVECTORF32 waterHighlight = DirectX::Colors::SkyBlue;

    //const float height = 0.003;
    const float height = 0.09;
    const float offset = .2;
    /*
    DirectX::SimpleMath::Vector3 nw(0.8, height, -2.001);
    DirectX::SimpleMath::Vector3 ne(4.7, height, 2.001);
    DirectX::SimpleMath::Vector3 se(4.3, height, 2.001);
    DirectX::SimpleMath::Vector3 sw(.3, height, -2.001);

    DirectX::SimpleMath::Vector3 e(4.55, height, 2.001);
    DirectX::SimpleMath::Vector3 w(0.55, height, -2.001);
    */

    //DirectX::SimpleMath::Vector3 nw(-0.457775 + offset, height, -3.184981);
    DirectX::SimpleMath::Vector3 nw(-0.077775 + offset, height, -3.184981);
    //DirectX::SimpleMath::Vector3 ne(4.998624, height, 2.250326 - offset);
    DirectX::SimpleMath::Vector3 ne(5.378624, height, 2.250326 - offset);
    DirectX::SimpleMath::Vector3 se(4.998624, height, 2.601 + offset);
    //DirectX::SimpleMath::Vector3 se(4.3, height, 2.001);
    DirectX::SimpleMath::Vector3 sw(-0.980228 - offset, height, -3.184981);

    DirectX::SimpleMath::Vector3 e(4.998624, height, 2.425663);
    DirectX::SimpleMath::Vector3 w(-0.7190015, height, -3.184981);

    VertexPositionColor v1(nw, waterColor);
    VertexPositionColor v2(ne, waterColor);
    VertexPositionColor v3(se, waterColor);
    VertexPositionColor v4(sw, waterColor);

    VertexPositionColor v5(e, waterColorDeep);
    VertexPositionColor v6(w, waterColorDeep);

    m_batch->DrawQuad(v1, v2, v5, v6);
    m_batch->DrawQuad(v6, v5, v3, v4);

    VertexPositionColor v7(nw, waterHighlight);
    VertexPositionColor v8(ne, waterHighlight);
    VertexPositionColor v9(se, waterHighlight);
    VertexPositionColor v10(sw, waterHighlight);

    m_batch->DrawLine(v7, v8);
    m_batch->DrawLine(v9, v10);
}

void Game::DrawWater2()
{
    DirectX::XMVECTORF32 waterColor = DirectX::Colors::Blue;
    DirectX::XMVECTORF32 waterColorDeep = DirectX::Colors::Navy;
    DirectX::XMVECTORF32 waterHighlight = DirectX::Colors::SkyBlue;

    const float height = 0.09;
    const float offset = 0.2;
    
    DirectX::SimpleMath::Vector3 waterNormal = DirectX::SimpleMath::Vector3::UnitY;

    DirectX::SimpleMath::Vector3 nw(-0.077775 + offset, height, -3.184981);
    DirectX::SimpleMath::Vector3 ne(5.378624, height, 2.250326 - offset);
    DirectX::SimpleMath::Vector3 se(4.998624, height, 2.601 + offset);
    DirectX::SimpleMath::Vector3 sw(-0.980228 - offset, height, -3.184981);

    DirectX::SimpleMath::Vector3 e(4.998624, height, 2.425663);
    DirectX::SimpleMath::Vector3 w(-0.7190015, height, -3.184981);

    VertexPositionNormalColor v1(nw, waterNormal, waterColor);
    VertexPositionNormalColor v2(ne, waterNormal, waterColor);
    VertexPositionNormalColor v3(se, waterNormal, waterColor);
    VertexPositionNormalColor v4(sw, waterNormal, waterColor);

    VertexPositionNormalColor v5(e, waterNormal, waterColorDeep);
    VertexPositionNormalColor v6(w, waterNormal, waterColorDeep);

    m_batch2->DrawQuad(v1, v2, v5, v6);
    m_batch2->DrawQuad(v6, v5, v3, v4);

    VertexPositionNormalColor v7(nw, waterNormal, waterHighlight);
    VertexPositionNormalColor v8(ne, waterNormal, waterHighlight);
    VertexPositionNormalColor v9(se, waterNormal, waterHighlight);
    VertexPositionNormalColor v10(sw, waterNormal, waterHighlight);

    m_batch2->DrawLine(v7, v8);
    m_batch2->DrawLine(v9, v10);
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
            //VertexPositionColor v1(scale - zAxis, gridColor);
            //VertexPositionColor v2(scale + zAxis, gridColor);
            VertexPositionColor v1(scale - zAxis, DirectX::Colors::LawnGreen); // Center line
            VertexPositionColor v2(scale + zAxis, DirectX::Colors::LawnGreen); // Center line
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
            //VertexPositionColor v1(scale - xAxis, gridColor); // Center line
            //VertexPositionColor v2(scale + xFarAxis, gridColor); // Center line
            VertexPositionColor v1(scale - xAxis, DirectX::Colors::LawnGreen); // Center line
            VertexPositionColor v2(scale + xFarAxis, DirectX::Colors::LawnGreen); // Center line
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, gridColor);
            VertexPositionColor v2(scale + xFarAxis, gridColor);
            m_batch->DrawLine(v1, v2);
        }
    }

    pGolf->UpdateEnvironmentSortingForDraw(pCamera->GetPos());

    std::vector<Fixture> fixtureList = pGolf->GetEnvironFixtureBucket();

    for (int i = 0; i < fixtureList.size(); ++i)
    {
        if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_FLAGSTICK)
        {
            DrawFlagHoleFixture(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TEEBOX)
        {
            DrawTeeBoxFixture(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE03)
        {
            DrawTree03(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE04)
        {
            DrawTree04(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE05)
        {
            DrawTree05(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE06)
        {
            DrawTree06(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE07)
        {
            DrawTree07(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE08)
        {
            DrawTree08(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE09)
        {
            DrawTree09(fixtureList[i].position, fixtureList[i].animationVariation);
        }
    }
}

void Game::DrawWorld12thHole()
{
    /*
    // draw world grid
    DirectX::SimpleMath::Vector3 xAxis(2.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 xFarAxis(5.f, 0.f, 0.f);
    DirectX::SimpleMath::Vector3 zAxis(0.f, 0.f, 2.f);
    DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3::Zero;
    size_t divisions = 50;
    size_t extention = 37;

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
            //VertexPositionColor v1(scale - zAxis, DirectX::Colors::LawnGreen); // Center line
            //VertexPositionColor v2(scale + zAxis, DirectX::Colors::LawnGreen); // Center line
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
            //VertexPositionColor v1(scale - xAxis, DirectX::Colors::LawnGreen); // Center line
            //VertexPositionColor v2(scale + xFarAxis, DirectX::Colors::LawnGreen); // Center line
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, gridColor);
            VertexPositionColor v2(scale + xFarAxis, gridColor);
            m_batch->DrawLine(v1, v2);
        }
    }

    // green
    const float height = 0.0;
    DirectX::XMVECTORF32 greenColor = DirectX::Colors::Green;
    VertexPositionColor bLeft(DirectX::SimpleMath::Vector3(2.5f, height, -0.7f), greenColor);
    VertexPositionColor bRight(DirectX::SimpleMath::Vector3(3.1f, height, -0.05f), greenColor);
    VertexPositionColor tRight(DirectX::SimpleMath::Vector3(3.5f, height, -0.1f), greenColor);
    VertexPositionColor tLeft(DirectX::SimpleMath::Vector3(3.0f, height, -0.75f), greenColor);
    m_batch->DrawQuad(bLeft, tLeft, tRight, bRight);
    */


    //DrawWater();


    //DrawSand();

    //pGolf->UpdateEnvironmentSortingForDraw(pCamera->GetPos()); // disabled since not needed when deapth buffer is enabled

    std::vector<Fixture> fixtureList = pGolf->GetEnvironFixtureBucket();

    for (int i = 0; i < fixtureList.size(); ++i)
    {
        if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_FLAGSTICK)
        {
            DrawFlagHoleFixture(fixtureList[i].position, fixtureList[i].animationVariation);           
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TEEBOX)
        {
            DrawTeeBoxFixture(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_BRIDGE)
        {
            DrawBridge(fixtureList[i].position, fixtureList[i].animationVariation);
        }


        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE03)
        {
            DrawTree03(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE04)
        {
            DrawTree04(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE05)
        {
            DrawTree05(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE06)
        {
            DrawTree06(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE07)
        {
            DrawTree07(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE08)
        {
            DrawTree08(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE09)
        {
            DrawTree09(fixtureList[i].position, fixtureList[i].animationVariation);
        }
    }

    //m_batch->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_terrainVertexArrayBase, m_terrainVertexCount);
    //m_batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, m_terrainVertexArray, m_terrainVertexCount);
}

void Game::DrawWorldWithLighting()
{
    DrawWater2();
    //DrawSand();

    //pGolf->UpdateEnvironmentSortingForDraw(pCamera->GetPos()); // disabled since not needed when deapth buffer is enabled

    std::vector<Fixture> fixtureList = pGolf->GetEnvironFixtureBucket();

    for (int i = 0; i < fixtureList.size(); ++i)
    {
        if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_FLAGSTICK)
        {
            DrawFlagHoleFixtureTest1(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TEEBOX)
        {
            DrawTeeBoxFixtureTest1(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_BRIDGE)
        {
            DrawBridgeTest2(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE03)
        {
            //DrawTree03(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE04)
        {
            DrawTree04Test1(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE05)
        {
            DrawTree05Test1(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE06)
        {
            DrawTree06Test2(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE07)
        {
            DrawTree07Test3(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE08)
        {
            //DrawTree08(fixtureList[i].position, fixtureList[i].animationVariation);
        }
        else if (fixtureList[i].fixtureType == FixtureType::FIXTURETYPE_TREE09)
        {
            DrawTree09Test1(fixtureList[i].position, fixtureList[i].animationVariation);
        }
    }

    //m_batch->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_terrainVertexArrayBase, m_terrainVertexCount);
    //m_batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, m_terrainVertexArray, m_terrainVertexCount);
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1600;
    height = 900;
}

DirectX::SimpleMath::Vector3 Game::GetSurfaceNormal(DirectX::SimpleMath::Vector3 aVecA, DirectX::SimpleMath::Vector3 aVecB, DirectX::SimpleMath::Vector3 aVecC)
{
    DirectX::SimpleMath::Vector3 v = (aVecA - aVecB);
    DirectX::SimpleMath::Vector3 w = (aVecC - aVecB);
    DirectX::SimpleMath::Vector3 n;
    n.x = (v.y * w.z) - (v.z * w.y);
    n.y = (v.z * w.x) - (v.x * w.z);
    n.z = (v.x * w.y) - (v.y * w.x);
    float denominator = (n.x * n.x) + (n.y * n.y) + (n.z * n.z);
    DirectX::SimpleMath::Vector3 a;
    a.x = n.x / denominator;
    a.y = n.y / denominator;
    a.z = n.z / denominator;
    a.Normalize();
    return a;
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

    bool result;
    bool isInitSuccessTrue = true;

    result = InitializeTerrainArray();
    if (!result)
    {
        isInitSuccessTrue = false;
    }
    result = InitializeTerrainArray2();
    if (!result)
    {
        isInitSuccessTrue = false;
    }
    if (!isInitSuccessTrue)
    {
        // add initialization failure testing  here;
        int errorBreak = 0;
        errorBreak++;
    }

    // testing new terrain map
    m_terrainVector.clear();
    //m_terrainVector = pZone->GetTerrainMap();
}

// Testing Terrain Vertex
bool Game::InitializeTerrainArray()
{
    std::vector<DirectX::VertexPositionColor> vertexPC = pGolf->GetTerrainVertex();
    m_terrainVertexCount = vertexPC.size();
    m_terrainVertexArray = new DirectX::VertexPositionColor[m_terrainVertexCount];
    m_terrainVertexArrayBase = new DirectX::VertexPositionColor[m_terrainVertexCount];

    //lawngreen = { { { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f } } };
    //DirectX::XMFLOAT4 lineColor(0.0, 0.501960814f, 0.0, 1.0);
    DirectX::XMFLOAT4 lineColor(.486274540f, .988235354f, 0.0, 1.0);
    DirectX::XMFLOAT4 baseColor(0.0, 0.0, 0.0, 1.0);
    //DirectX::XMFLOAT4 baseColor2(0.3, 0.3, 0.3, 1.0);
    //DirectX::XMFLOAT4 baseColor2(1.0, 1.0, 1.0, 1.0);
    DirectX::XMFLOAT4 baseColor2(0.0, 0.0, 0.0, 1.0);

    DirectX::XMFLOAT4 sandColor1(0.956862807f, 0.643137276f, 0.376470625f, 1.0);
    DirectX::XMFLOAT4 sandColor2(0.960784376f, 0.960784376f, 0.862745166f, 1.0);
    DirectX::XMFLOAT4 greenColor1 = DirectX::XMFLOAT4(0.0, 0.501960814f, 0.0, 1.0);
    DirectX::XMFLOAT4 greenColor2 = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.0, 1.0);

    DirectX::XMFLOAT4 grassColor1 = DirectX::XMFLOAT4(0.133333340f, 0.545098066f, 0.133333340f, 1.0);
    DirectX::XMFLOAT4 grassColor2 = DirectX::XMFLOAT4(0.000000000f, 0.392156899f, 0.0, 1.0);

    DirectX::Colors::ForestGreen;
    DirectX::Colors::DarkGreen;
    //baseColor = DirectX::XMFLOAT4(0.0, 0.501960814f, 0.0, 1.0);
    //baseColor2 = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.0, 1.0);
    //XMGLOBALCONST XMVECTORF32 SandyBrown = { { { 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 Beige = { { { 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 Green = { { { 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 LawnGreen = { { { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 ForestGreen          = { { { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 DarkGreen            = { { { 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f } } };
    for (int i = 0; i < m_terrainVertexCount; ++i)
    {
        m_terrainVertexArray[i].position = vertexPC[i].position;
        m_terrainVertexArray[i].color = lineColor;
        m_terrainVertexArrayBase[i].position = vertexPC[i].position;

        // Green
        //if (i > 1802 && i < 1818 || i > 1991 && i < 2007 || i > 2183 && i < 2190)
        if (i > 1805 && i < 1818 || i > 1991 && i < 2007 || i > 2183 && i < 2190)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase[i].color = greenColor1;
            }
            else
            {
                m_terrainVertexArrayBase[i].color = greenColor2;
            }
        }
        // bunker 1
        else if (i > 2195 && i < 2202 || i > 2381 && i < 2391)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase[i].color = sandColor2;
            }
        }
        // bunker 2
        else if (i > 1613 && i < 1617 || i > 1424 && i < 1428 )
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase[i].color = sandColor2;
            }
        }
        // bunker 3
        else if (i > 1796 && i < 1803 )
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase[i].color = sandColor2;
            }
        }
        else if (i % 2 == 0)
        {
            m_terrainVertexArrayBase[i].color = baseColor;
        }
        else
        {
            m_terrainVertexArrayBase[i].color = baseColor2;     
        }
    }

    return true;
}

bool Game::InitializeTerrainArray2()
{
    std::vector<DirectX::VertexPositionNormalColor> vertexPC = pGolf->GetTerrainVertex2();

    m_terrainVertexCount2 = vertexPC.size();
    m_terrainVertexArray2 = new DirectX::VertexPositionNormalColor[m_terrainVertexCount2];
    m_terrainVertexArrayBase2 = new DirectX::VertexPositionNormalColor[m_terrainVertexCount2];

    //lawngreen = { { { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f } } };
    //DirectX::XMFLOAT4 lineColor(0.0, 0.501960814f, 0.0, 1.0);
    DirectX::XMFLOAT4 lineColor(.486274540f, .988235354f, 0.0, 1.0);
    DirectX::XMFLOAT4 baseColor(0.0, 0.0, 0.0, 1.0);
    //DirectX::XMFLOAT4 baseColor2(0.3, 0.3, 0.3, 1.0);
    DirectX::XMFLOAT4 baseColor2(1.0, 1.0, 1.0, 1.0);
    //DirectX::XMFLOAT4 baseColor2(0.0, 0.0, 0.0, 1.0);

    DirectX::XMFLOAT4 sandColor1(0.956862807f, 0.643137276f, 0.376470625f, 1.0);
    DirectX::XMFLOAT4 sandColor2(0.960784376f, 0.960784376f, 0.862745166f, 1.0);
    DirectX::XMFLOAT4 greenColor1 = DirectX::XMFLOAT4(0.0, 0.501960814f, 0.0, 1.0);
    DirectX::XMFLOAT4 greenColor2 = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.0, 1.0);

    DirectX::XMFLOAT4 grassColor1 = DirectX::XMFLOAT4(0.133333340f, 0.545098066f, 0.133333340f, 1.0);
    DirectX::XMFLOAT4 grassColor2 = DirectX::XMFLOAT4(0.000000000f, 0.392156899f, 0.0, 1.0);
    DirectX::XMFLOAT4 testRed = DirectX::XMFLOAT4(1.000000000f, 0.000000000f, 0.0, 1.0);
    DirectX::XMFLOAT4 testBlue = DirectX::XMFLOAT4(0.000000000f, 0.000000000f, 1.0, 1.0);
    DirectX::XMFLOAT4 testGray = DirectX::XMFLOAT4(0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f);
    DirectX::XMFLOAT4 testWhite = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);

    //baseColor = DirectX::XMFLOAT4(0.0, 0.501960814f, 0.0, 1.0);
    //baseColor2 = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.0, 1.0);
    //XMGLOBALCONST XMVECTORF32 SandyBrown = { { { 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 Beige = { { { 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 Green = { { { 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 LawnGreen = { { { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 ForestGreen          = { { { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f } } };
    //XMGLOBALCONST XMVECTORF32 DarkGreen            = { { { 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f } } };
   
    
    baseColor = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f);
    baseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.000000000f);
    baseColor = DirectX::XMFLOAT4(0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f);
    testGray = DirectX::XMFLOAT4(0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f);
    greenColor2 = greenColor1;
    sandColor2 = sandColor1;
    baseColor2 = baseColor;
    lineColor = baseColor;
    testGray = baseColor;
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationY(Utility::GetPi());
    DirectX::SimpleMath::Matrix rotMat2 = DirectX::SimpleMath::Matrix::CreateRotationX(Utility::GetPi());
    for (int i = 0; i < m_terrainVertexCount2; ++i)
    {
        m_terrainVertexArray2[i].position = vertexPC[i].position;
        DirectX::SimpleMath::Vector3 testFlip = vertexPC[i].normal;
        m_terrainVertexArray2[i].normal = vertexPC[i].normal;
        m_terrainVertexArray2[i].normal = DirectX::SimpleMath::Vector3::Transform(m_terrainVertexArray2[i].normal, rotMat);
        m_terrainVertexArray2[i].normal = DirectX::SimpleMath::Vector3::Transform(m_terrainVertexArray2[i].normal, rotMat2);
        m_terrainVertexArray2[i].color = lineColor;
        
        m_terrainVertexArray2[i].normal.x = - testFlip.x;
        m_terrainVertexArray2[i].normal.y = - testFlip.y;
        m_terrainVertexArray2[i].normal.z = - testFlip.z;
        
        testFlip = vertexPC[i].normal;
        m_terrainVertexArrayBase2[i].position = vertexPC[i].position;
        m_terrainVertexArrayBase2[i].normal = vertexPC[i].normal;
        m_terrainVertexArrayBase2[i].normal = DirectX::SimpleMath::Vector3::Transform(m_terrainVertexArrayBase2[i].normal, rotMat);
        m_terrainVertexArrayBase2[i].normal = DirectX::SimpleMath::Vector3::Transform(m_terrainVertexArrayBase2[i].normal, rotMat2);
        
        m_terrainVertexArrayBase2[i].normal.x = -testFlip.x;
        m_terrainVertexArrayBase2[i].normal.y = -testFlip.y;
        m_terrainVertexArrayBase2[i].normal.z = -testFlip.z;
        
        if (i % 2 == 0)
        {
            m_terrainVertexArrayBase2[i].color = baseColor;
        }
        else
        {
            m_terrainVertexArrayBase2[i].color = baseColor;
        }

        if (i == i)
            //if (i < 96)
            //if (i % 96 == 0)
        {
            if ((i + 5) % 6 == 0)
            {
                m_terrainVertexArrayBase2[i].color = baseColor;
            }
            if ((i + 2) % 6 == 0)
            {
                m_terrainVertexArrayBase2[i].color = testGray;
            }
            if (i % 6 == 0)
            {
                m_terrainVertexArrayBase2[i].color = testGray;
            }
        }

        //////////////////////////////////
        // Testing bunker and green draws
                // Green
        //if (i > 1802 && i < 1818 || i > 1991 && i < 2007 || i > 2183 && i < 2190)
        if (i > 1805 && i < 1818 || i > 1991 && i < 2007 || i > 2183 && i < 2190)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase2[i].color = greenColor1;
            }
            else
            {
                m_terrainVertexArrayBase2[i].color = greenColor2;
            }
        }
        // bunker 1
        else if (i > 2195 && i < 2202 || i > 2381 && i < 2391)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase2[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase2[i].color = sandColor2;
            }
        }
        // bunker 2
        else if (i > 1613 && i < 1617 || i > 1424 && i < 1428)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase2[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase2[i].color = sandColor2;
            }
        }
        // bunker 3
        else if (i > 1796 && i < 1803)
        {
            if (i % 2 == 0)
            {
                m_terrainVertexArrayBase2[i].color = sandColor1;
            }
            else
            {
                m_terrainVertexArrayBase2[i].color = sandColor2;
            }
        }
        else if (i % 2 == 0)
        {
            m_terrainVertexArrayBase[i].color = baseColor;
        }
        else
        {
            m_terrainVertexArrayBase[i].color = baseColor2;
        }
        
    }

    return true;
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

    m_shape.reset();

    // lighting effect and batch
    m_effectNormColorLighting.reset();
    m_batchNormColorLighting.reset();
    m_effect2.reset();
    m_batch2.reset();


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
    //m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0); // doesnt facecull
    m_d3dContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    //m_d3dContext->RSSetState(m_states->CullNone());
    //m_d3dContext->RSSetState(m_states->CullClockwise());
    //10  m_d3dContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    //11  m_d3dContext->RSSetState(m_states->CullCounterClockwise());

    m_d3dContext->RSSetState(m_raster.Get()); // WLJ anti-aliasing  RenderTesting


    //auto sampler = m_states->LinearClamp();
    //m_d3dContext->PSSetSamplers(0, 1, &sampler);

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    //auto sampler = m_states->LinearClamp();
    //m_d3dContext->PSSetSamplers(0, 1, &sampler);

    void const* shaderByteCode2;
    size_t byteCodeLength2;
    m_effect2->GetVertexShaderBytecode(&shaderByteCode2, &byteCodeLength2);
    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType2::InputElements, VertexType2::InputElementCount, shaderByteCode2, byteCodeLength2, m_inputLayout.ReleaseAndGetAddressOf()));
    m_batch2 = std::make_unique<PrimitiveBatch<VertexType2>>(m_d3dContext.Get());
    m_effect2->Apply(m_d3dContext.Get());

    //m_d3dContext->PSSetSamplers(0, 1, &sampler);
    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    m_batch2->Begin();

    m_batch2->Draw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_terrainVertexArrayBase2, m_terrainVertexCount2);

    DirectX::SimpleMath::Vector3 treePos1(0.0, 1.0, 0.0);
    //DrawTree06Test1(treePos1, 0.0);  // tri
    DirectX::SimpleMath::Vector3 treePos2(0.0, 1.0, 0.2);
    DrawTree06Test2(treePos2, 0.0);
    DirectX::SimpleMath::Vector3 treePos3(1.3, 1.0, -0.8);
    //DrawTree04Test1(treePos3, 0.0);
    DirectX::SimpleMath::Vector3 treePos4(1.3, 1.0, -0.1);
    DrawTree07Test1(treePos4, 0.0);
    DirectX::SimpleMath::Vector3 treePos5(1.3, 1.0, 0.2);
    DrawTree07Test2(treePos5, 0.0);
    DirectX::SimpleMath::Vector3 treePos6(0.9, 1.0, -0.7);
    DrawTree09Test1(treePos6, 0.0);

    DrawWater2();

    DirectX::SimpleMath::Vector3 bridgePos1(2.0, 0.2, 0.0);
    DrawBridgeTest2(bridgePos1, 0.0);
    DirectX::SimpleMath::Vector3 bridgePos2(2.6, 0.2, 0.3);
    DrawBridgeTest2(bridgePos2, 3.14);
    /*
    DirectX::SimpleMath::Vector3 bridgePos3(.6, 0.8, -0.3);
    DrawBridgeTest2(bridgePos3, 0.0);
    DirectX::SimpleMath::Vector3 bridgePos4(.6, 0.8, -0.6);
    DrawBridgeTest2(bridgePos4, 3.14);
    DirectX::SimpleMath::Vector3 bridgePos5(.6, 0.8, -0.9);
    DrawBridgeTest2(bridgePos5, 0.0);
    DirectX::SimpleMath::Vector3 bridgePos6(.6, 0.8, -1.2);
    DrawBridgeTest2(bridgePos6, Utility::ToRadians(90.0));
    DirectX::SimpleMath::Vector3 bridgePos7(.8, 0.8, -1.5);
    DrawBridgeTest2(bridgePos7, Utility::ToRadians(45.0));
    DirectX::SimpleMath::Vector3 bridgePos8(1.3, 0.8, -1.0);
    DrawBridgeTest2(bridgePos8, Utility::ToRadians(135.0));

    DirectX::SimpleMath::Vector3 bridgePos9(1.3, 1.5, -0.5);
    //DrawBridgeTest2(bridgePos9, Utility::ToRadians(m_timer.GetTotalSeconds() * 6.0 ));
    DrawBridgeTest2(bridgePos9, m_timer.GetTotalSeconds() * 0.5);

    DirectX::SimpleMath::Vector3 bridgePos10(1.3, 1.5, -0.8);
    DrawBridgeTest2(bridgePos10, - m_timer.GetTotalSeconds() * 0.5);
    */

    DrawWorldWithLighting();

    m_batch2->End();
    
    DirectX::SimpleMath::Vector3 testCam = pCamera->GetPos();
     
    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));
    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(m_d3dContext.Get());
    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    m_effect->SetWorld(m_world);

    m_effect->Apply(m_d3dContext.Get());

    //m_d3dContext->IASetInputLayout(m_inputLayout.Get());   
    
    m_batch->Begin();
    DrawTree07(DirectX::SimpleMath::Vector3(1.0, 1.0, 1.0), 0.0);
    //DrawDebugLines();
    if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
    {
        //DrawWorld12thHole();
        //DrawShotAimCone();
        DrawShotAimArrow();

        if (pCamera->GetCameraState() == CameraState::CAMERASTATE_SWINGVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_FIRSTPERSON)
        {
            DrawSwing();
        }

        if (pCamera->GetCameraState() == CameraState::CAMERASTATE_PRESWINGVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW || pCamera->GetCameraState() == CameraState::CAMERASTATE_FIRSTPERSON)
        {
            m_flightStepTimer.ResetElapsedTime();
            DrawProjectileRealTime();
            DrawHydraShot();
        }
        if (m_isInDebugMode == true)
        {
            //DrawCameraFocus();
            //DrawDebugLines();
        }
    }
    
    m_batch->End();
    
   
 
    

    /*
    void const* shaderByteCodeLighting;
    size_t byteCodeLengthLighting;
    m_effectNormColorLighting->GetVertexShaderBytecode(&shaderByteCodeLighting, &byteCodeLengthLighting);
    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(DirectX::VertexPositionNormalColor::InputElements, DirectX::VertexPositionNormalColor::InputElementCount, shaderByteCodeLighting, byteCodeLengthLighting, m_inputLayout.ReleaseAndGetAddressOf()));
    m_batchNormColorLighting = std::make_unique<PrimitiveBatch<DirectX::VertexPositionNormalColor>>(m_d3dContext.Get());

    m_effectNormColorLighting->Apply(m_d3dContext.Get());

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());
    m_batchNormColorLighting->Begin();
    m_batchNormColorLighting->End();
    */



    // Testing shapes for skydome
    //void XM_CALLCONV Draw(FXMMATRIX world, CXMMATRIX view, CXMMATRIX projection, FXMVECTOR color = Colors::White, _In_opt_ ID3D11ShaderResourceView * texture = nullptr, bool wireframe = false,
    //   _In_opt_ std::function<void __cdecl()> setCustomState = nullptr) const;
    //m_shape->Draw(m_world, m_view, m_proj);
    //m_shape->Draw(m_world, m_view, m_proj);
    //m_shape->Draw(m_world, m_view, m_proj, DirectX::Colors::SkyBlue, nullptr, false);

    m_spriteBatch->Begin();
    
    //DrawShotTimerUI();

    if (m_currentGameState == GameState::GAMESTATE_INTROSCREEN)
    {
        DrawIntroScreen();
    }
    if (m_currentGameState == GameState::GAMESTATE_STARTSCREEN)
    {
        DrawStartScreen();
    }
    if (m_currentGameState == GameState::GAMESTATE_MAINMENU)
    {
        DrawMenuMain();
    }
    if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
    {
        DrawMenuCharacterSelect();
    }
    if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
    {
        DrawMenuEnvironmentSelect();
    }
    if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
    {
        DrawPowerBarUI();
        //DrawSwingUI();
        DrawUI();
    }

    m_spriteBatch->End();
    
    Present();
}

void Game::ResetGamePlay()
{
    pGolf->ZeroUIandRenderData();
    pPlay->ResetPlayData();
    pPlay->SetAim(pGolf->GetShotStartPos(), pGolf->GetHolePosition());
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

    if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
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

    if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
    {
        pPlay->Swing();

        if (pPlay->UpdateSwing() == true)
        {
            AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK::XACT_WAVEBANK_AUDIOBANK_IMPACTSFX1);

            pPlay->ResetSwingUpdateReady();
            m_currentUiState = UiState::UISTATE_SHOT;
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

    pLighting->UpdateLighting(m_effect2, aTimer.GetTotalSeconds());
    pCamera->UpdateCamera(aTimer);
    DirectX::SimpleMath::Matrix viewMatrix = pCamera->GetViewMatrix();
    m_effect->SetView(viewMatrix);
    m_effect2->SetView(viewMatrix);

    UpdateInput(aTimer);
}

void Game::UpdateInput(DX::StepTimer const& aTimer)
{
    // WLJ add for mouse and keybord interface   
    auto kb = m_keyboard->GetState();
    m_kbStateTracker.Update(kb);

    if (kb.Escape)
    {
        if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
        {
            pCamera->SetCameraState(CameraState::CAMERASTATE_PRESWINGVIEW);
            ResetGamePlay();
        }
        m_currentGameState = GameState::GAMESTATE_MAINMENU;
    }
    if (m_kbStateTracker.pressed.Enter)
    {
        if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
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
            m_currentGameState = GameState::GAMESTATE_STARTSCREEN;
        }
        if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
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
            //m_currentGameState = GameState::GAMESTATE_MAINMENU; // Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
            m_currentGameState = GameState::GAMESTATE_STARTSCREEN;// Return to Main Menu after selecting character, ToDo: using value of 1 doesn't return to main menu
        }
        if (m_currentGameState == GameState::GAMESTATE_MAINMENU)
        {
            if (m_menuSelect == 0) // GoTo Game State
            {
                m_currentGameState = GameState::GAMESTATE_GAMEPLAY;
            }
            if (m_menuSelect == 1) // GoTo Character Select State
            {
                m_currentGameState = GameState::GAMESTATE_CHARACTERSELECT;
            }
            if (m_menuSelect == 2) // GoTo Environment Select State
            {
                m_currentGameState = GameState::GAMESTATE_ENVIRONTMENTSELECT;
            }
            if (m_menuSelect == 3) // GoTo Demo Select State
            {
                m_currentGameState = GameState::GAMESTATE_GAMEPLAY;
            }
            if (m_menuSelect == 4) // Quit Game
            {
                ExitGame();
            }
            m_menuSelect = 0;
        }
        if (m_currentGameState == GameState::GAMESTATE_STARTSCREEN)
        {
            m_currentGameState = GameState::GAMESTATE_MAINMENU;
        }
    }
    if (m_kbStateTracker.pressed.Up)
    {
        if (m_currentGameState == GameState::GAMESTATE_MAINMENU)
        {
            --m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Down)
    {
        if (m_currentGameState == GameState::GAMESTATE_MAINMENU)
        {
            ++m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            ++m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Left)
    {
        if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            --m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker.pressed.Right)
    {
        if (m_currentGameState == GameState::GAMESTATE_CHARACTERSELECT)
        {
            ++m_menuSelect;
        }
        if (m_currentGameState == GameState::GAMESTATE_ENVIRONTMENTSELECT)
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
        pPlay->ResetSwingCount();
        pGolf->SetBallPosition(pGolf->GetTeePos());
        pGolf->SetShotStartPos(pGolf->GetBallPosition());
        pCamera->SetCameraStartPos(pCamera->GetPos());
        //pCamera->SetCameraEndPos(pCamera->GetPreSwingCamPos(pGolf->GetTeePos(), pGolf->GetTeeDirection()));
        pCamera->SetCameraEndPos(pCamera->GetPreSwingCamPos(pGolf->GetTeePos(), pGolf->GetDirectionToHoleInRads()));
        pCamera->SetTargetStartPos(pCamera->GetTargetPos());
        //pCamera->SetTargetEndPos(pCamera->GetPreSwingTargPos(pGolf->GetTeePos(), pGolf->GetTeeDirection()));
        pCamera->SetTargetEndPos(pGolf->GetHolePosition());
        pCamera->SetCameraState(CameraState::CAMERASTATE_RESET);
        pGolf->ResetIsBallInHole();
        ResetGamePlay();
        m_currentUiState = UiState::UISTATE_SWING;
    }
    if (m_kbStateTracker.pressed.B) // move cameras to new ball position and prep for next shot
    {
        pGolf->SetShotStartPos(pGolf->GetBallPosition());
        pCamera->SetCameraStartPos(pCamera->GetPos());
        pCamera->SetCameraEndPos(pCamera->GetPreSwingCamPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
        pCamera->SetTargetStartPos(pCamera->GetTargetPos());
        //pCamera->SetTargetEndPos(pCamera->GetPreSwingTargPos(pGolf->GetShotStartPos(), pGolf->GetDirectionToHoleInRads()));
        pCamera->SetTargetEndPos(pGolf->GetHolePosition());
        pCamera->SetCameraState(CameraState::CAMERASTATE_TRANSTONEWSHOT);
        ResetGamePlay();
        m_currentUiState = UiState::UISTATE_SWING;
    }
    if (m_kbStateTracker.pressed.Space)
    {
        if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
        {
            if (pPlay->IsSwingStateAtImpact() == true)
            {
                pPlay->IncrementSwingCount();
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
        if (m_currentGameState == GameState::GAMESTATE_GAMEPLAY)
        {
            pPlay->IncrementSwingCount();
            m_kbStateTracker.Reset();
            m_projectileTimer = 0.0;
            pPlay->DebugShot();
        }
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
        pLighting->SetLightControlDirection( - DirectX::SimpleMath::Vector3::UnitY);
    }
    if (m_kbStateTracker.pressed.I)
    {
        pLighting->SetLightControlDirection(DirectX::SimpleMath::Vector3::UnitX);
    }
    if (m_kbStateTracker.pressed.O)
    {
        pLighting->SetLightControlDirection(DirectX::SimpleMath::Vector3::UnitY);
    }
    if (m_kbStateTracker.pressed.P)
    {
        pLighting->SetLightControlDirection(DirectX::SimpleMath::Vector3(1.5, -1.0, 1.5));
    }
    if (m_kbStateTracker.pressed.J)
    {
        pLighting->SetLightControlDirection( - DirectX::SimpleMath::Vector3::UnitZ);
    }
    if (m_kbStateTracker.pressed.K)
    {
        pLighting->SetLightControlDirection(- DirectX::SimpleMath::Vector3::UnitX);
    }
    if (m_kbStateTracker.pressed.L)
    {
        pLighting->SetLightControlDirection(DirectX::SimpleMath::Vector3::UnitZ);
    }
    if (m_kbStateTracker.pressed.F1)
    {
        pCamera->SetCameraState(CameraState::CAMERASTATE_SWINGVIEW);
    }
    if (kb.OemPeriod)
    {
        pPlay->TurnShotAim(static_cast<float>(-aTimer.GetElapsedSeconds()), pCamera->GetAimTurnRate());
        if (pCamera->GetCameraState() != CameraState::CAMERASTATE_FIRSTPERSON)  // for debuging don't move camera if in first person mode
        {
            pCamera->TurnAroundPoint(static_cast<float>(-aTimer.GetElapsedSeconds()), pGolf->GetShotStartPos());
        }
    }
    if (kb.OemComma)
    {
        pPlay->TurnShotAim(static_cast<float>(aTimer.GetElapsedSeconds()), pCamera->GetAimTurnRate());
        if (pCamera->GetCameraState() != CameraState::CAMERASTATE_FIRSTPERSON) // for debuging don't move camera if in first person mode
        {
            pCamera->TurnAroundPoint(static_cast<float>(aTimer.GetElapsedSeconds()), pGolf->GetShotStartPos());
        }
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
