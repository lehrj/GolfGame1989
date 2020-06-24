//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath; // WLJ add
using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
    pGolf = new Golf;
    pPlay = new GolfPlay;
}

Game::~Game()
{
    delete pGolf;
    delete pPlay;
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
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    pPlay->Swing();

    if (pPlay->UpdateSwing() == true)
    {
        pPlay->ResetSwingUpdateReady();
        pGolf->UpdateImpact(pPlay->GetImpactData());
    }
    
    UpdateCamera(timer);

    // WLJ add for mouse and keybord interface
    auto kb = m_keyboard->GetState();
    //m_kbStateTracker.Update(kb);
    if (kb.Escape)
    {
        ExitGame();
    }
    if (kb.D1)
    {
        pGolf->SelectInputClub(1);
    }
    if (kb.D2)
    {
        pGolf->SelectInputClub(2);
    }
    if (kb.D3)
    {
        pGolf->SelectInputClub(3);
    }
    if (kb.D4)
    {
        pGolf->SelectInputClub(4);
    }
    if (kb.D5)
    {
        pGolf->SelectInputClub(5);
    }
    if (kb.D6)
    {
        pGolf->SelectInputClub(6);
    }
    if (kb.D7)
    {
        pGolf->SelectInputClub(7);
    }
    if (kb.D8)
    {
        pGolf->SelectInputClub(8);
    }
    if (kb.D9)
    {
        pGolf->SelectInputClub(9);
    }
    if (kb.D0)
    {
        pGolf->SelectInputClub(10);
    }
    if (kb.Z)
    {
        pPlay->StartSwing();
    }
    if (kb.X)
    {
        pPlay->SetPower();
    }
    if (kb.C)
    {
        pPlay->SetImpact();
    }
    if (kb.V)
    {
        pPlay->ResetPlayData();
        ResetPowerMeter();
    }
    if (kb.A)
    {
        if (pPlay->GetIsGameplayButtonReady() == true)
        {
            pPlay->UpdateSwingState();
            pPlay->SetGameplayButtonReadyFalse();
        }
    }
    if (kb.IsKeyUp(DirectX::Keyboard::Keys::A))
    {
        pPlay->ResetGamePlayButton();
    }
    /*
    if (m_kbStateTracker.IsKeyReleased(DirectX::Keyboard::Keys::A))
    {
        pPlay->ResetGamePlayButton();
    }
    */
    if (kb.F1)
    {
        SetGameCamera(1);
    }
    if (kb.F2)
    {
        SetGameCamera(2);
    }
    if (kb.F3)
    {
        SetGameCamera(4);
    }
    if (kb.F4)
    {
        SetGameCamera(4);
    }
    if (kb.F5)
    {
        SetGameCamera(5);
    }
    if (kb.F6)
    {
        SetGameCamera(6);
    }
    if (kb.F7)
    {
        SetGameCamera(7);
    }
    if (kb.F8)
    {
        SetGameCamera(8);
    }
    if (kb.F9)
    {
        SetGameCamera(9);
    }
    if (kb.OemMinus)
    {
        m_cameraRotationX -= .01;
    }
    if (kb.OemPlus)
    {
        m_cameraRotationX += .01;
    }
    if (kb.OemOpenBrackets)
    {
        m_cameraRotationY -= .01;
    }
    if (kb.OemCloseBrackets)
    {
        m_cameraRotationY += .01;
    }
    auto mouse = m_mouse->GetState();

    elapsedTime;
}

void Game::UpdateCamera(DX::StepTimer const& timer)
{
    // world start
    if (m_gameCamera == 1)
    {
        m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
        //m_world = Matrix::CreateRotationY((static_cast<float>(m_cameraRotationX)));
        m_worldAntiRotation = m_world.Invert();
    }
    if (m_gameCamera == 2)
    {
        m_world = Matrix::CreateRotationX(m_cameraRotationY);
    }
    if (m_gameCamera == 3)
    {
        m_world = Matrix::CreateRotationY(m_cameraRotationX);
    }
    if (m_gameCamera == 4)
    {
        m_view = Matrix::CreateLookAt(Vector3(0.f, 0.f, 3.f),
            Vector3::Zero, Vector3::UnitY);

        m_effect->SetView(m_view);
    }
    if (m_gameCamera == 5)
    {
        const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
        const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

        m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 6.f,
            float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);
        m_effect->SetProjection(m_proj);
    }
    if (m_gameCamera == 6)
    {
        const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
        const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
        m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f), Vector3::Zero, Vector3::UnitY);
        m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);

        m_effect->SetView(m_view);
        m_effect->SetProjection(m_proj);
    }
    if (m_gameCamera == 7)
    {
        m_view = Matrix::CreateLookAt(Vector3(.0f, 0.0f, 7.0f), Vector3::Zero, Vector3::UnitY);
        m_effect->SetView(m_view);
    }
    if (m_gameCamera == 8)
    {

    }
    if (m_gameCamera == 9)
    {

    }
    else
    {

    }
    // world end
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
    m_d3dContext->RSSetState(m_raster.Get()); // WLJ anti-aliasing
    m_effect->SetWorld(m_world);
    //world end
    m_effect->Apply(m_d3dContext.Get());

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    m_batch->Begin();

    //DrawSwing();
    DrawWorld();
    DrawProjectile(); 

    m_batch->End();

    m_spriteBatch->Begin();

    RenderUIPowerBar();
    if (1 == 1) // toggle between debug and normal UI rendering
    {
        RenderDebugInfo();
        RenderUI();
    }
    else
    {
        RenderUI();
    }
    m_spriteBatch->End();

    Present();

    // Switch to next club in the bag after impact of previous shot
    /*
    if (toggleGetNextClub == 1)
    {
        xVec.clear();
        yVec.clear();
        zVec.clear();
        pGolf->SelectNextClub();
    }
    */
}

void Game::RenderDebugInfo()
{
    //m_fontPosDebug
    std::vector<std::string> uiString = pPlay->GetDebugData();

    float fontOriginPosX = m_fontPosDebug.x;
    float fontOriginPosY = m_fontPosDebug.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());

        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
        m_fontPosDebug.y += 35;
    }
    m_fontPosDebug.y = fontOriginPosY;
}

void Game::RenderUI()
{
    std::vector<std::string> uiString = pGolf->GetUIstrings();

    std::string output = uiString[0];

    float fontOriginPosX = m_fontPos2.x;
    float fontOriginPosY = m_fontPos2.y;

    for (int i = 0; i < uiString.size(); ++i)
    {
        std::string uiLine = std::string(uiString[i]);
        //Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str()) / 2.f;
        Vector2 lineOrigin = m_font->MeasureString(uiLine.c_str());
        //m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, originText);
        m_font->DrawString(m_spriteBatch.get(), uiLine.c_str(), m_fontPos2, Colors::White, 0.f, lineOrigin);
        m_fontPos2.y += 35;
    }
    m_fontPos2.y = fontOriginPosY;
}

void Game::RenderUIPowerBar()
{
    if (pPlay->GetMeterPower() >= 0.0)
    {
        //m_powerMeterBarRect.left = m_powerMeterBarRect.right - (m_powerMeterSize * (pPlay->GetMeterPower() * 0.01));
        //m_powerMeterBarRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * (pPlay->GetMeterPower() * 0.01));
        m_powerMeterBarRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));

        //m_powerMeterBarRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * (pPlay->GetMeterPower() * 0.007));
    }
    else
    {
        //m_powerMeterBarRect.right = m_powerMeterBarRect.right - (m_powerMeterSize * (pPlay->GetMeterPower() * 0.01));
        //m_powerMeterBarRect.right = m_powerMeterImpactPoint - (m_powerMeterSize * (pPlay->GetMeterPower() * 0.01));
        m_powerMeterBarRect.right = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));
    }
    if (pPlay->GetIsBackswingSet() == false)
    {
        m_powerMeterBackswingRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetMeterPower() * m_powerMeterBarScale) * 0.01));
    }
    else
    {
        m_powerMeterBackswingRect.left = m_powerMeterImpactPoint - (m_powerMeterSize * ((pPlay->GetBackswingSet() * m_powerMeterBarScale) * 0.01));
    }
    //m_powerMeterBackswingRect = m_powerMeterFrameRect;

    m_spriteBatch->Draw(m_powerBackswingTexture.Get(), m_powerMeterBackswingRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerMeterTexture.Get(), m_powerMeterBarRect, nullptr, Colors::White);

    m_spriteBatch->Draw(m_powerFrameTexture.Get(), m_powerMeterFrameRect, nullptr, Colors::White);
    m_spriteBatch->Draw(m_powerImpactTexture.Get(), m_powerMeterImpactRect, nullptr, Colors::White);
}

void Game::SetGameCamera(int aCamera)
{
    if (aCamera == 1)
    {
        m_gameCamera = 1;
    }
    if (aCamera == 2)
    {
        m_gameCamera = 2; 
    }
    if (aCamera == 3)
    {
        m_gameCamera = 3;
    }
    if (aCamera == 4)
    {
        m_gameCamera = 4;
    }
    if (aCamera == 5)
    {
        m_gameCamera = 5;
    }
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
    //m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
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

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
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
    // WLJ start
    // world start
    m_world = Matrix::Identity;
    m_worldAntiRotation = Matrix::Identity;
    // world end
    m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());

    m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
    m_effect->SetVertexColorEnabled(true);

    void const* shaderByteCode;
    size_t byteCodeLength;

    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    DX::ThrowIfFailed(
        m_d3dDevice->CreateInputLayout(VertexType::InputElements,
            VertexType::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_inputLayout.ReleaseAndGetAddressOf()));

    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(m_d3dContext.Get());


    // world start
    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);

    DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rastDesc,
        m_raster.ReleaseAndGetAddressOf()));
    // world end


    m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"myfile.spritefont");
    m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());
    // end

    // Start Texture
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarFrame.png", nullptr, m_powerFrameTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarMeter.png", nullptr, m_powerMeterTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarImpact.png", nullptr, m_powerImpactTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"PowerbarBackswing.png", nullptr, m_powerBackswingTexture.ReleaseAndGetAddressOf()));
    ComPtr<ID3D11Resource> resource;
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
    // End texture
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

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.

    ////********* WLJ world start ----- deactivate to turn off world spin
    m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f), Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.f);

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);
    // world end

    m_fontPos.x = backBufferWidth / 2.f;
    m_fontPos.y = backBufferHeight / 2.f;
    //m_fontPos2.x = backBufferWidth / 5.f;
    m_fontPos2.x = backBufferWidth - 5;
    //m_fontPos2.y = backBufferHeight / 30.f;
    m_fontPos2.y = 35;
    m_fontPosDebug.x = 480;
    m_fontPosDebug.y = 35;

    // Start swing power bar
    m_powerMeterFrameRect.left = (backBufferWidth / 2) - m_powerBarFrameOrigin.x;
    m_powerMeterFrameRect.right = (backBufferWidth / 2) + m_powerBarFrameOrigin.x;
    m_powerMeterFrameRect.top = (backBufferHeight / 1.08) - m_powerBarFrameOrigin.y;
    m_powerMeterFrameRect.bottom = (backBufferHeight / 1.08) + m_powerBarFrameOrigin.y;

    m_powerMeterSize = m_powerMeterFrameRect.right - m_powerMeterFrameRect.left;

    float powerMeterScale = pPlay->GetMeterLength();
    float impactPointScale = pPlay->GetMeterImpactPoint();
    impactPointScale = impactPointScale * (m_powerMeterSize / powerMeterScale);
    m_powerMeterImpactPoint = m_powerMeterFrameRect.right - impactPointScale;
    
    m_powerMeterImpactRect.top = m_powerMeterFrameRect.top;
    m_powerMeterImpactRect.bottom = m_powerMeterFrameRect.bottom;
    m_powerMeterImpactRect.right = m_powerMeterFrameRect.right - impactPointScale + 20;
    m_powerMeterImpactRect.left = m_powerMeterFrameRect.right - impactPointScale - 20;

    m_powerMeterBarRect = m_powerMeterFrameRect;
    m_powerMeterBarRect.left = m_powerMeterFrameRect.right - impactPointScale;
    m_powerMeterBarRect.right = m_powerMeterFrameRect.right - impactPointScale;

    m_powerMeterBackswingRect = m_powerMeterFrameRect;
    m_powerMeterBackswingRect.left = m_powerMeterBarRect.left;
    m_powerMeterBackswingRect.right = m_powerMeterBarRect.right;

    m_powerMeterBarScale = 1.0 - (pPlay->GetMeterImpactPoint() / pPlay->GetMeterLength());

    // End Texture
}

void Game::DrawProjectile()
{
    /////////********* Start projectile draw
    //std::vector<double> xVec = pGolf->GetVect(0);
    //std::vector<double> yVec = pGolf->GetVect(1);
    //std::vector<double> zVec = pGolf->GetVect(2);
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();


    //draw tee box
    double originX = shotPath[0].x;
    double originZ = shotPath[0].z;
    Vector3 t1(originX - .05, 0.0f, -0.1f);
    Vector3 t2(originX + .05, 0.0f, -0.1f);
    Vector3 t3(originX - 0.05, 0.0f, 0.1f);
    Vector3 t4(originX + .05, 0.0f, 0.1f);
    VertexPositionColor vt1(t1, Colors::White);
    VertexPositionColor vt2(t2, Colors::White);
    VertexPositionColor vt3(t3, Colors::White);
    VertexPositionColor vt4(t4, Colors::White);
    m_batch->DrawLine(vt1, vt2);
    m_batch->DrawLine(vt1, vt3);
    m_batch->DrawLine(vt3, vt4);
    m_batch->DrawLine(vt4, vt2);

    // end tee box draw

    int stepCount = shotPath.size();

    if (arcCount >= stepCount)
    {
        arcCount = 0;
    }
    ++arcCount;
    //double prevX = 0.0;
    //double prevY = 0.0;

    Vector3 prevPos = shotPath[0];
    for (int i = 0; i < arcCount; ++i)
    {
        Vector3 p1(prevPos);
        
        Vector3 p2(shotPath[i]);
        //VertexPositionColor aV(p1, Colors::White);
        //VertexPositionColor bV(p2, Colors::White);

        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);
        VertexPositionColor aVRed(p1, Colors::Red);
        VertexPositionColor bVRed(p2, Colors::Red);
        VertexPositionColor aVBlue(p1, Colors::Blue);
        VertexPositionColor bVBlue(p2, Colors::Blue);
        VertexPositionColor aVYellow(p1, Colors::Yellow);
        VertexPositionColor bVYellow(p2, Colors::Yellow);
        std::vector<int> colorVec = pGolf->GetDrawColorVector();
        int vecIndex = pGolf->GetDrawColorIndex();
        
        if (vecIndex > 0)
        {
            if (i > colorVec[0])
            {
                aV = aVRed;
                bV = bVRed;
            }
        }
        if (vecIndex > 1)
        {
            if (i > colorVec[1])
            {
                aV = aVBlue;
                bV = bVBlue;
            }
        }
        if (vecIndex > 2)
        {
            if (i > colorVec[2])
            {
                aV = aVYellow;
                bV = bVYellow;
            }
        }
        
        m_batch->DrawLine(aV, bV);
        prevPos = shotPath[i];

    }

    //bool toggleGetNextClub = 0;
    ///// Landing explosion
    //if (arcCount == stepCount)
    //{
        /*
        Vector3 f1(prevPos);
        Vector3 f2(prevX, prevY + 0.2f, prevZ);
        Vector3 f3(prevX + 0.1f, prevY + 0.1f, prevZ + 0.1f);
        Vector3 f4(prevX - 0.1f, prevY + 0.1f, prevZ - 0.1f);
        Vector3 f5(prevX + 0.1f, prevY + 0.1f, prevZ - 0.1f);
        Vector3 f6(prevX - 0.1f, prevY + 0.1f, prevZ + 0.1f);
        Vector3 f7(prevX + 0.01f, prevY + 0.1f, prevZ + 0.01f);
        Vector3 f8(prevX - 0.01f, prevY + 0.1f, prevZ - 0.01f);
        Vector3 f9(prevX + 0.01f, prevY + 0.1f, prevZ - 0.01f);
        Vector3 f10(prevX - 0.01f, prevY + 0.1f, prevZ + 0.01f);

        VertexPositionColor ft1(f1, Colors::Red);
        VertexPositionColor ft2(f2, Colors::Red);
        VertexPositionColor ft3(f3, Colors::Red);
        VertexPositionColor ft4(f4, Colors::Red);
        VertexPositionColor ft5(f5, Colors::Red);
        VertexPositionColor ft6(f6, Colors::Red);
        VertexPositionColor ft7(f7, Colors::Red);
        VertexPositionColor ft8(f8, Colors::Red);
        VertexPositionColor ft9(f9, Colors::Red);
        VertexPositionColor ft10(f10, Colors::Red);
        m_batch->DrawLine(ft1, ft2);
        m_batch->DrawLine(ft1, ft3);
        m_batch->DrawLine(ft1, ft4);
        m_batch->DrawLine(ft1, ft5);
        m_batch->DrawLine(ft1, ft6);
        m_batch->DrawLine(ft1, ft7);
        m_batch->DrawLine(ft1, ft8);
        m_batch->DrawLine(ft1, ft9);
        m_batch->DrawLine(ft1, ft10);
        */
        //toggleGetNextClub = 1;
    //}
    // end landing explosion
}

void Game::DrawWorld()
{
    // draw world grid
    Vector3 xaxis(2.f, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, 2.f);
    Vector3 origin = Vector3::Zero;
    size_t divisions = 20;
    for (size_t i = 0; i <= divisions; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        Vector3 scale = xaxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - yaxis, Colors::Green);
            VertexPositionColor v2(scale + yaxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - yaxis, Colors::Green);
            VertexPositionColor v2(scale + yaxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + origin;

        if (scale.z == 0.0f)
        {
            //VertexPositionColor v1(scale - xaxis, Colors::Red);
            //VertexPositionColor v2(scale + xaxis, Colors::Red);
            //m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xaxis, Colors::Green);
            VertexPositionColor v2(scale + xaxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
}

void Game::DrawSwing()
{
    /////////********* Start swing draw
/*
std::vector<DirectX::SimpleMath::Vector3> alphaVec = pGolf->GetAlpha();
std::vector<DirectX::SimpleMath::Vector3> betaVec = pGolf->GetBeta();
std::vector<DirectX::SimpleMath::Vector3> thetaVec = pGolf->GetTheta();
int swingStepCount = alphaVec.size();

if (arcCount >= swingStepCount)
{
    arcCount = 0;
}
++arcCount;
Vector3 swingOrigin(0.0f, 0.0f, 0.0f);
Vector3 swingTop(0.0f, 1.0f, 0.0f);
VertexPositionColor feet(swingOrigin, Colors::Yellow);
VertexPositionColor shoulder(swingTop, Colors::White);
for (int i = 0; i < arcCount; ++i)
{
    //Vector3 hand = thetaVec[i];
    Vector3 hand = alphaVec[i];
    hand += swingTop;
    Vector3 clubHead = betaVec[i];
    //clubHead -= hand;
    VertexPositionColor body(swingOrigin, Colors::Red);
    VertexPositionColor arm(hand, Colors::Green);
    VertexPositionColor shaft(clubHead, Colors::Blue);
    //m_batch->DrawLine(feet, shoulder);
    //m_batch->DrawLine(shoulder, arm);
    //m_batch->DrawLine(arm, shaft);
    //m_batch->DrawLine(vert3, vert1);
    m_batch->DrawLine(feet, arm);
    m_batch->DrawLine(feet, shoulder);
    m_batch->DrawLine(feet, shaft);
}
*/

/*
Vector4d launchData = pGolf->GetLaunchVector();
double armLength = launchData.GetFirst();
double clubLength = launchData.GetSecond();
double launchAngle = launchData.GetThird();
double launchVelocity = launchData.GetForth();
std::vector<Vector4d> swingAngles;
swingAngles.resize(pGolf->GetSwingStepIncCount());
swingAngles = pGolf->GetSwingData();
//Vector3 swingOrigin(0.0f, 0.0f, 0.0f);
Vector3 armPivot(0.0f, armLength, 0.0f);
Vector3 clubHead(clubLength, armLength, 0.0f);
VertexPositionColor vert1(swingOrigin, Colors::Red);
VertexPositionColor vert2(armPivot, Colors::Red);
VertexPositionColor vert3(clubHead, Colors::Red);
m_batch->DrawLine(vert1, vert2);
m_batch->DrawLine(vert2, vert3);
*/

// end swing draw
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // WLJ start
    m_states.reset();
    m_effect.reset();
    m_batch.reset();

    m_inputLayout.Reset();

    m_font.reset();
    m_spriteBatch.reset();
    m_powerFrameTexture.Reset();
    m_powerMeterTexture.Reset();
    m_powerImpactTexture.Reset();
    m_powerBackswingTexture.Reset();
    // end
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}




