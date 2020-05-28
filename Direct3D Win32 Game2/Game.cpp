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
}
/*
Game::~Game()
{
    //delete pGolf;
}
*/

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
    // world start
    m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
    // world end

    elapsedTime;
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
    /////////********* Start swing draw
    /*
    Vector4d launchData = pGolf->GetLaunchVector();
    double armLength = launchData.GetFirst();
    double clubLength = launchData.GetSecond();
    double launchAngle = launchData.GetThird();
    double launchVelocity = launchData.GetForth();
    std::vector<Vector4d> swingAngles;
    swingAngles.resize(pGolf->GetSwingStepIncCount());
    swingAngles = pGolf->GetSwingData();

    Vector3 origin(0.0f, 0.0f, 0.0f);
    Vector3 armPivot(0.0f, armLength, 0.0f);
    Vector3 clubHead(clubLength, armLength, 0.0f);
    VertexPositionColor vert1(origin, Colors::Red);
    VertexPositionColor vert2(armPivot, Colors::Red);
    VertexPositionColor vert3(clubHead, Colors::Red);
    m_batch->DrawLine(vert1, vert2);
    m_batch->DrawLine(vert2, vert3);
    */
    // end swing draw

    /////////********* Start projectile draw
    std::vector<double> xVec = pGolf->GetVect(0);
    std::vector<double> yVec = pGolf->GetVect(1);
    std::vector<double> zVec = pGolf->GetVect(2);

    double groundLevel = yVec[0];

    Vector3 xaxis(2.f, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, 2.f);

    Vector3 origin = Vector3::Zero;

    size_t divisions = 20;

    for (size_t i = 0; i <= divisions; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = xaxis * fPercent + origin;

        VertexPositionColor v1(scale - yaxis, Colors::Green);
        VertexPositionColor v2(scale + yaxis, Colors::Green);
        m_batch->DrawLine(v1, v2);
    }


    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + origin;

        VertexPositionColor v1(scale - xaxis, Colors::Green);
        VertexPositionColor v2(scale + xaxis, Colors::Green);
        m_batch->DrawLine(v1, v2);
    }

    //draw tee box
    double originX = xVec[0];
    double originZ = zVec[0];
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

    int stepCount = xVec.size();

    int timeCount = m_timer.GetElapsedSeconds();

    if (m_timer.GetTotalSeconds() < 1)
    {
        timeCount = 1;
    }
    else
    {

    }

    //int frameCount = timeCount % stepCount;
    //int frameCount = stepCount % timeCount;

    if (arcCount >= stepCount)
    {
        arcCount = 0;
    }
    ++arcCount;
    //double prevX = 0.0;
    //double prevY = 0.0;
    double prevX = xVec[0];
    double prevY = yVec[0];
    double prevZ = zVec[0];

    for (int i = 0; i < arcCount; ++i)
    {
        Vector3 p1(prevX, prevY, prevZ);
        Vector3 p2(xVec[i], yVec[i], zVec[i]);
        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);
        m_batch->DrawLine(aV, bV);
        prevX = xVec[i];
        prevY = yVec[i];
        prevZ = zVec[i];
    }

    ///// Landing explosion
    if (arcCount == stepCount - 1)
    {
        Vector3 f1(prevX, prevY, prevZ);
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
    }
    // end landing explosion

    m_batch->End();

    // start UI draw
    std::vector<double> uiData = pGolf->GetUIdata();
    std::vector<std::string> uiString = pGolf->GetUIstrings();

    // Drawing text using a font
    /*
    m_spriteBatch->Begin();
    const wchar_t* output = L"Hello World";
    const wchar_t*
    Vector2 originText = m_font->MeasureString(output) / 2.f;
    m_font->DrawString(m_spriteBatch.get(), output,
        m_fontPos, Colors::White, 0.f, originText);
    m_spriteBatch->End();
    */
    // text end

    // Using std::wstring for text
    //std::wstring output = std::wstring(L"Hello") + std::wstring(L" World");
    //std::string output = std::string("Hello") + std::string(" World");
    std::string output = uiString[0];
    std::string line1 = std::string(uiString[0]) + std::to_string(uiData[0]);

    float fontOriginPosX = m_fontPos2.x;
    float fontOriginPosY = m_fontPos2.y;

    m_spriteBatch->Begin();

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

    /*
    Vector2 originText = m_font->MeasureString(output.c_str()) / 2.f;
    Vector2 origin1 = m_font->MeasureString(line1.c_str()) / 2.f;

    m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos2, Colors::White, 0.f, originText);
    //m_fontPos.y = m_fontPos.y + 30.f;
    m_fontPos.y = m_outputHeight - 40.f;
    m_font->DrawString(m_spriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, originText);
    m_fontPos.y = m_fontPos.y - 40.f;
    */
    m_spriteBatch->End();
    // wsting end
    /* Game::CreateResources()
    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = backBufferWidth;
    m_fontPos.x = backBufferWidth / 2.f;
    m_fontPos.y = backBufferHeight / 2.f;
    */
    // end UI draw

    Present();
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
    m_fontPos2.x = backBufferWidth;
    m_fontPos2.y = backBufferHeight / 30.f;

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
    // end
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}