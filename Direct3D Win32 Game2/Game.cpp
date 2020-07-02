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
    m_kbStateTracker = std::make_unique< Keyboard::KeyboardStateTracker>();

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

    m_flightStepTimer.Tick([&]()
        {
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    m_projectileTimer += elapsedTime;
    // TODO: Add your game logic here.

    if (m_gameState == 2)
    {
        if (m_menuSelect == 0)
        {
            m_character0->Update(elapsedTime);
        }
        if (m_menuSelect == 1)
        {
            m_character1->Update(elapsedTime);
        }
        if (m_menuSelect == 2)
        {
            m_character2->Update(elapsedTime);
        }
    }
    pPlay->Swing();

    if (pPlay->UpdateSwing() == true)
    {
        pPlay->ResetSwingUpdateReady();
        pGolf->UpdateImpact(pPlay->GetImpactData());
    }

    UpdateCamera(timer);

    // WLJ add for mouse and keybord interface
    auto kb = m_keyboard->GetState();    
    
    m_kbStateTracker->Update(kb);

    if (kb.Escape)
    {
        m_gameState = 1;
    }
    
    if (m_kbStateTracker->pressed.Enter)
    {
        if (m_gameState == 1) // Main Menu State
        {
            if (m_menuSelect == 0) // GoTo Game State
            {
                m_gameState = 10;
            }
            if (m_menuSelect == 1) // GoTo Character Select State
            {
                m_gameState = 2;
            }
            if (m_menuSelect == 2) // Quit Game
            {
                ExitGame();
            }
        }
        if (m_gameState == 0) // Start Screen state
        {
            m_gameState = 1; // GoTo Main Menu
        }
    }
    if (m_kbStateTracker->pressed.Up)
    {
        if (m_gameState == 1)
        {
            --m_menuSelect;
        }
        if (m_gameState == 2)
        {
            --m_menuSelect;
        }
    }
    if (m_kbStateTracker->pressed.Down)
    {
        if (m_gameState == 1)
        {
            ++m_menuSelect;
        }
        if (m_gameState == 2)
        {
            ++m_menuSelect;
        }
    }
    
    if (kb.L)
    {
        if (m_gameState == 1)
        {
            m_gameState = 2;
        }
    }
    if (kb.K)
    {
        if (m_gameState == 2)
        {
            m_gameState = 3;
        }
    }
    if (kb.J)
    {
        if (m_gameState == 3)
        {
            m_gameState = 0;
        }
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
    if (m_kbStateTracker->pressed.Space)
    {
        if (m_gameState == 10)
        {
            pPlay->UpdateSwingState();
        }
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
    if (kb.NumPad6)
    {
        m_cameraRotationX -= m_cameraMovementSpeed;
    }
    if (kb.NumPad4)
    {
        m_cameraRotationX += m_cameraMovementSpeed;
    }
    if (kb.NumPad2)
    {
        m_cameraRotationY -= m_cameraMovementSpeed;
    }
    if (kb.NumPad8)
    {
        m_cameraRotationY += m_cameraMovementSpeed;
    }
    if (kb.NumPad7)
    {
        m_cameraTargetX += m_cameraMovementSpeed;
    }
    if (kb.NumPad9)
    {
        m_cameraTargetX -= m_cameraMovementSpeed;
    }
    if (kb.NumPad1)
    {
        m_cameraTargetZ += m_cameraMovementSpeed;
    }
    if (kb.NumPad3)
    {
        m_cameraTargetZ -= m_cameraMovementSpeed;
    }
    if (kb.OemMinus)
    {
        m_cameraZoom -= m_cameraMovementSpeed + .3f;
    }
    if (kb.OemPlus)
    {
        m_cameraZoom += m_cameraMovementSpeed + .3f;
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

    }
    if (m_gameCamera == 2)
    {
        m_view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 0.f), Vector3::Zero, Vector3::UnitY);
        m_world = Matrix::CreateRotationY(Utility::ToRadians(90));
        m_effect->SetView(m_view);
    }
    if (m_gameCamera == 3)
    {
        m_world = Matrix::CreateRotationY(m_cameraRotationX);
        m_view = Matrix::CreateLookAt(Vector3(6.f, 0.f, 0.f), Vector3::Zero, Vector3::UnitY);
        //m_world = Matrix::CreateRotationY(Utility::ToRadians(90));
        m_effect->SetView(m_view);
    }
    if (m_gameCamera == 4)
    {
        const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
        const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
        //m_view = Matrix::CreateLookAt(Vector3(2.f, m_cameraRotationY, 2.f), Vector3::Zero, Vector3::UnitY);
        m_view = Matrix::CreateLookAt(Vector3(2.f, m_cameraRotationY, 2.f), Vector3(m_cameraTargetX, 0.0, m_cameraTargetZ) , Vector3::UnitY);
        m_world = Matrix::CreateRotationY(m_cameraRotationX);
        m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
            float(backBufferWidth) / float(backBufferHeight), 0.1f, 10.0f);

        m_effect->SetView(m_view);
        m_effect->SetProjection(m_proj);
    }
    if (m_gameCamera == 5)
    {
        m_view = Matrix::CreateLookAt(Vector3(-6.f, 1.f, 2.f), Vector3::Zero, Vector3::UnitY);
        //m_world = Matrix::CreateRotationY(Utility::ToRadians(45));
        //m_world = Matrix::CreateRotationY(0.0);
        m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
        m_effect->SetView(m_view);
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
        m_view = Matrix::CreateLookAt(Vector3(-6.f, 1.f, 2.f), Vector3::Zero, Vector3::UnitY);
        //m_world = Matrix::CreateRotationY(Utility::ToRadians(45));
        //m_world = Matrix::CreateRotationY(0.0);
        m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
        m_effect->SetView(m_view);

    }
    if (m_gameCamera == 9)
    {
        m_view = Matrix::CreateLookAt(Vector3(0.f, 0.f, 3.f), Vector3::Zero, Vector3::UnitY);

        m_effect->SetView(m_view);
        /*
        m_view = Matrix::CreateLookAt(Vector3(m_cameraRotationY, 1.f, 2.f), Vector3::Zero, Vector3::UnitY);
        //m_world = Matrix::CreateRotationY(Utility::ToRadians(45));
        //m_world = Matrix::CreateRotationY(0.0);
        m_world = Matrix::CreateRotationY(m_cameraRotationX);
        m_effect->SetView(m_view);
        */
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

    if (m_gameState == 10)
    {
        //DrawSwing();
        DrawWorld();
        DrawProjectile();
        //DrawProjectileRealTime();
    }

    m_batch->End();

    m_spriteBatch->Begin();

    //DrawShotTimerUI();
    if (m_gameState == 0)
    {
        DrawStartScreen();
    }
    if (m_gameState == 1)
    {
        DrawMenuMain();
    }
    if (m_gameState == 2)
    {
        DrawMenuCharacterSelect();
    }
    if (m_gameState == 10)
    {
        DrawPowerBarUI();
    }
    if (m_gameState == 10)
    {
        DrawSwingUI();
        DrawUI();
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

void Game::DrawSwingUI()
{
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

void Game::DrawUI()
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

void Game::DrawPowerBarUI()
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
    m_keyboard.reset();
    m_kbStateTracker.reset();
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
    m_keyboard.reset();
    m_kbStateTracker.reset();
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
    m_keyboard.reset();
    m_kbStateTracker.reset();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_kbStateTracker.reset();
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

    DX::ThrowIfFailed(m_d3dDevice->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));

    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(m_d3dContext.Get());

    // world start
    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);

    DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rastDesc, m_raster.ReleaseAndGetAddressOf()));
    // world end

    m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"myfile.spritefont");
    m_titleFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"titleFont.spritefont");
    m_textFont = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"bitwise16.spritefont");
    m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());
    // end

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

    // Character Textures
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

    //m_sprites = std::make_unique<SpriteSheet>();
    //m_sprites->Load(m_characterTexture.Get(), L"CharacterSpriteSheetData.txt");
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
    
    // UI font positions
    m_fontPos.x = backBufferWidth / 2.f;
    m_fontPos.y = backBufferHeight / 2.f;
    m_fontPos2.x = backBufferWidth - 5;
    m_fontPos2.y = 35;
    m_fontPosDebug.x = 480;
    m_fontPosDebug.y = 35;
    m_fontMenuPos.x = backBufferWidth / 2.f;
    m_fontMenuPos.y = 35;
    m_textFontPos.x = backBufferWidth / 2.f;
    m_textFontPos.y = backBufferHeight / 2.f;

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

    // Character texture
    m_characterPos.x = float(backBufferWidth / 2);
    m_characterPos.y = float((backBufferHeight / 2) + (backBufferHeight / 4));

    m_character0Pos.x = float(backBufferWidth / 2);
    m_character0Pos.y = float((backBufferHeight / 2) + (backBufferHeight / 4));

    m_character1Pos.x = float(backBufferWidth / 2);
    m_character1Pos.y = float((backBufferHeight / 2) + (backBufferHeight / 4));

    m_character2Pos.x = float(backBufferWidth / 2);
    m_character2Pos.y = float((backBufferHeight / 2) + (backBufferHeight / 4));

    DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), L"CharacterBackground.png", nullptr, m_characterBackgroundTexture.ReleaseAndGetAddressOf()));
    m_characterBackgroundPos.x = backBufferWidth / 2.f;
    m_characterBackgroundPos.y = backBufferHeight / 2.f;
    // End Texture
}

// working old version prior to impmenting real time match update
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

    if (m_arcCount >= stepCount)
    {
        m_flightStepTimer.ResetElapsedTime();
        m_arcCount = 0;
    }
    ++m_arcCount;
    //double prevX = 0.0;
    //double prevY = 0.0;

    Vector3 prevPos = shotPath[0];
    for (int i = 0; i < m_arcCount; ++i)
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

void Game::DrawProjectileRealTime()
{
    std::vector<DirectX::SimpleMath::Vector3> shotPath = pGolf->GetShotPath();

    std::vector<float> shotTimeStep = pGolf->GetShotPathTimeSteps();
    int stepCount = shotPath.size();
    float shotTimeTotal = shotTimeStep.back();

    if (m_arcCount >= stepCount)
    {
        //m_projectileTimer = 0.0;
        m_arcCount = 0;
    }
    ++m_arcCount;

    /*
    Vector3 prevPos = shotPath[0];
    int i = -1;
    do
    {
        ++i;
        Vector3 p1(prevPos);

        Vector3 p2(shotPath[i]);

        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);
        m_batch->DrawLine(aV, bV);
        prevPos = shotPath[i];
        
        //++i;
        if (i > shotTimeStep.size()-1)
        {
            break;
        }
        
    } while (shotTimeStep[i] < m_projectileTimer || i < shotTimeStep.size()-1);
    */
    
    Vector3 prevPos = shotPath[0];
    for (int i = 0; i < shotPath.size(); ++i)
    {
        Vector3 p1(prevPos);
        Vector3 p2(shotPath[i]);
        VertexPositionColor aV(p1, Colors::White);
        VertexPositionColor bV(p2, Colors::White);

        if (shotTimeStep[i] < m_projectileTimer)
        {
            m_batch->DrawLine(aV, bV);
        }
        prevPos = shotPath[i];
    }
    
    if (m_projectileTimer > shotTimeStep.back())
    {
        m_projectileTimer = 0.0;
    }
}

void Game::DrawShotTimerUI()
{
    std::string timerUI = "Timer = " + std::to_string(m_projectileTimer);
    Vector2 lineOrigin = m_font->MeasureString(timerUI.c_str());
    m_font->DrawString(m_spriteBatch.get(), timerUI.c_str(), m_fontPosDebug, Colors::White, 0.f, lineOrigin);
}

void Game::DrawMenuCharacterSelect()
{
    float lineDrawY = m_fontMenuPos.y + 25;
    float lineDrawSpacingY = 15;
    std::string menuTitle = "Character Select";
    float menuTitlePosX = m_fontMenuPos.x;
    float menuTitlePosY = lineDrawY;
    Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);
    Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);

    lineDrawY += menuTitlePosY + lineDrawSpacingY;
    std::string menuObj0String = "Character 1";
    Vector2 menuObj0Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;
    m_character0Pos.x = menuTitlePosX / 3;
    m_character0Pos.y = lineDrawY - menuObj0Origin.y;
    m_characterBackgroundPos.x = m_character0Pos.x + menuObj0Origin.x;
    m_characterBackgroundPos.y = m_character0Pos.y + 10;
    m_characterBackgroundOrigin = menuObj0Origin;

    if (m_menuSelect == 0)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);       
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character0->Draw(m_spriteBatch.get(), m_character0Pos);
    
    lineDrawY += menuObj0Pos.y;
    std::string menuObj1String = "Character 2";
    //Vector2 menuObj1Pos(menuTitlePosX, menuObj0Pos.y + menuOrigin.x + 0);
    Vector2 menuObj1Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;
    m_character1Pos.x = menuTitlePosX / 3;
    m_character1Pos.y = lineDrawY - menuObj1Origin.y;
    m_characterBackgroundPos.x = m_character1Pos.x + menuObj1Origin.x;
    m_characterBackgroundPos.y = m_character1Pos.y + 10;
    m_characterBackgroundOrigin = menuObj1Origin;
    if (m_menuSelect == 1)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
    }
    m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos, nullptr, Colors::White, 0.f, m_characterBackgroundOrigin);
    m_character1->Draw(m_spriteBatch.get(), m_character1Pos);

    lineDrawY += menuObj0Pos.y;
    std::string menuObj2String = "Character 3";
    Vector2 menuObj2Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;
    m_character2Pos.x = menuTitlePosX / 3;
    m_character2Pos.y = lineDrawY - menuObj2Origin.y;
    m_characterBackgroundPos.x = m_character2Pos.x + menuObj2Origin.x;
    m_characterBackgroundPos.y = m_character2Pos.y + 10;
    m_characterBackgroundOrigin = menuObj2Origin;
    if (m_menuSelect == 2)
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::LawnGreen, 0.f, m_characterBackgroundOrigin);
    }
    else
    {
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-4.f, -4.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-3.f, -3.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-2.f, -2.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
        m_spriteBatch->Draw(m_characterBackgroundTexture.Get(), m_characterBackgroundPos + Vector2(-1.f, -1.f), nullptr, Colors::Gray, 0.f, m_characterBackgroundOrigin);
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
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

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
    //float menuTitlePosY = m_fontPos.y / 2.f;
    float menuTitlePosY = lineDrawY;
    Vector2 menuTitlePos(menuTitlePosX, menuTitlePosY);   
    Vector2 menuOrigin = m_titleFont->MeasureString(menuTitle.c_str()) / 2.f;
    m_titleFont->DrawString(m_spriteBatch.get(), menuTitle.c_str(), menuTitlePos, Colors::White, 0.f, menuOrigin);  

    lineDrawY += menuTitlePosY + lineDrawSpacingY;
    std::string menuObj0String = "Driving Range";
    Vector2 menuObj0Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj0Origin = m_font->MeasureString(menuObj0String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj1String = "Charachter Select";
    //Vector2 menuObj1Pos(menuTitlePosX, menuObj0Pos.y + menuOrigin.x + 0);
    Vector2 menuObj1Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj1Origin = m_font->MeasureString(menuObj1String.c_str()) / 2.f;

    lineDrawY += menuObj0Pos.y;
    std::string menuObj2String = "Quit";
    Vector2 menuObj2Pos(menuTitlePosX, lineDrawY);
    Vector2 menuObj2Origin = m_font->MeasureString(menuObj2String.c_str()) / 2.f;

    if (m_menuSelect < 0 || m_menuSelect > 2)
    {
        m_menuSelect = 0;
    }
    if (m_menuSelect == 0)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj0Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj0String.c_str(), menuObj0Pos, Colors::White, 0.f, menuObj0Origin);
    }

    if (m_menuSelect == 1)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj1Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj1String.c_str(), menuObj1Pos, Colors::White, 0.f, menuObj1Origin);
    }

    if (m_menuSelect == 2)
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-4.f, 4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-4.f, -4.f), Colors::White, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(4.f, -4.f), Colors::White, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-2.f, 2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(-2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos + Vector2(2.f, -2.f), Colors::Black, 0.f, menuObj2Origin);

        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
    else
    {
        m_font->DrawString(m_spriteBatch.get(), menuObj2String.c_str(), menuObj2Pos, Colors::White, 0.f, menuObj2Origin);
    }
}

/*
void Game::DrawStartScreen()
{
    std::string title = "Golf Gm 1989";
    float lineDraw = m_fontMenuPos.y + 10;
    float fontTitlePosX = m_fontPosDebug.x - 100;
    //float fontTitlePosX = m_fontPos.x;
    float fontTitlePosY = lineDraw;
    Vector2 titlePos(fontTitlePosX, fontTitlePosY);


    Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    
    float space = 35;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);
    titlePos.y += titleOrigin.y + space;

    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::Lime, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y = m_fontMenuPos.y + 15;
    titlePos.x = m_fontPos2.x - 450;/////////////////////////////Break

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-10.f, -10.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-9.f, -9.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-10.f, -10.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-9.f, -9.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Gray, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Green, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::White, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(8.f, 8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(7.f, 7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(5.f, 5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(3.f, 3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(1.f, 1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::White, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;

    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-8.f, -8.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-7.f, -7.f), Colors::DarkGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::DarkGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-5.f, -5.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::ForestGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-3.f, -3.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-1.f, -1.f), Colors::Lime, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LawnGreen, 0.f, titleOrigin);

    titlePos.y += titleOrigin.y + space;
}
*/

void Game::DrawStartScreen()
{
    std::string title = "GolfGame1989";
    std::string author = "By Lehr Jackson";
    std::string startText = "Press Enter to Start";
    float fontTitlePosX = m_fontPos.x;
    float fontTitlePosY = m_fontPos.y / 2.f;
    Vector2 titlePos(fontTitlePosX, fontTitlePosY);
    float fontAuthorPosX = m_fontPos.x;
    float fontAuthorPosY = m_fontPos.y;
    Vector2 authorPos(fontAuthorPosX, fontAuthorPosY);
    Vector2 startTextPos(m_fontPos.x, m_fontPos.y + fontTitlePosY);

    Vector2 titleOrigin = m_titleFont->MeasureString(title.c_str()) / 2.f;
    Vector2 authorOrigin = m_font->MeasureString(author.c_str()) / 2.f;
    Vector2 startTextOrigin = m_font->MeasureString(startText.c_str()) / 2.f;

    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(6.f, 6.f), Colors::White, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-6.f, -6.f), Colors::ForestGreen, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(4.f, 4.f), Colors::Black, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-4.f, -4.f), Colors::Green, 0.f, titleOrigin);
    //m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(2.f, 2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos + Vector2(-2.f, -2.f), Colors::LawnGreen, 0.f, titleOrigin);
    m_titleFont->DrawString(m_spriteBatch.get(), title.c_str(), titlePos, Colors::LimeGreen, 0.f, titleOrigin);

    m_font->DrawString(m_spriteBatch.get(), author.c_str(), authorPos, Colors::White, 0.f, authorOrigin);
    m_font->DrawString(m_spriteBatch.get(), startText.c_str(), startTextPos, Colors::White, 0.f, startTextOrigin);
}


void Game::DrawSwing()
{
    float shoulderAccel = .98;

    std::vector<DirectX::SimpleMath::Vector3> angles;
    angles = pGolf->GetRawSwingAngles();
    Vector3 origin;
    origin.Zero;

    Vector3 thetaOrigin;
    thetaOrigin.Zero;
    thetaOrigin.y = -1.0;

    Vector3 armOrigin;
    armOrigin.Zero;
    armOrigin.y = -1.0;
    armOrigin = Vector3::Transform(armOrigin, Matrix::CreateRotationZ(Utility::ToRadians(-135.0)));
    Vector3 shaftOrigin;
    shaftOrigin.Zero;
    shaftOrigin.y = -1.0;
    shaftOrigin = Vector3::Transform(shaftOrigin, Matrix::CreateRotationZ(Utility::ToRadians(120.0)));
    shaftOrigin += armOrigin;

    VertexPositionColor shoulder(origin, Colors::White);
    VertexPositionColor hand(armOrigin, Colors::White);
    VertexPositionColor clubHead(shaftOrigin, Colors::White);
    //m_batch->DrawLine(shoulder, hand);
    //m_batch->DrawLine(hand, clubHead);
    Vector3 arm = armOrigin;
    Vector3 shaft = shaftOrigin;

    int swingStepCount = angles.size();
    if (m_arcCount >= swingStepCount)
    {
        m_arcCount = 0;
    }
    ++m_arcCount;

    int impactPoint = pGolf->GetImpactStep();

    for (int i = 0; i < m_arcCount; ++i)
    {
        if (i < impactPoint)
        {
            Vector3 theta = Vector3::Transform(thetaOrigin, Matrix::CreateRotationZ(-angles[i].z));
            VertexPositionColor thetaColor(theta, Colors::Blue);
            m_batch->DrawLine(shoulder, thetaColor);
            Vector3 beta = Vector3::Transform(theta, Matrix::CreateRotationZ(-angles[i].y));
            beta += theta;
            VertexPositionColor betaColor(beta, Colors::Red);
            m_batch->DrawLine(thetaColor, betaColor);
            arm = Vector3::Transform(armOrigin, Matrix::CreateRotationZ(angles[i].z));
            //shaft = Vector3::Transform(shaftOrigin, Matrix::CreateRotationZ(Utility::ToRadians(180.0)));
            //shaft = Vector3::Transform(shaftOrigin, Matrix::CreateRotationZ(-angles[i].y));
            //shaft = Vector3::Transform(armOrigin, Matrix::CreateRotationZ(-angles[i].y));
            shaft = Vector3::Transform(armOrigin, Matrix::CreateRotationZ(-angles[i].y));
            //shaft += arm;
            VertexPositionColor updateHand(arm, Colors::White);
            VertexPositionColor updateClubHead(shaft, Colors::Red);
            //m_batch->DrawLine(updateHand, shoulder);
            //m_batch->DrawLine(thetaColor, updateClubHead);
        }
    }

    /*
    Vector3 top(0.0f, 1.0f, 0.0f);
    Vector3 bottom(0.0f, -1.0f, 0.0f);
    Vector3 left(-1.0f, 0.0f, 0.0f);
    Vector3 right(1.0f, 0.0f, 0.0f);
    VertexPositionColor vTop(top, Colors::Red);
    VertexPositionColor vBottom(bottom, Colors::Red);
    VertexPositionColor vLeft(left, Colors::Red);
    VertexPositionColor vRight(right, Colors::Red);
    m_batch->DrawLine(vTop, vBottom);
    m_batch->DrawLine(vLeft, vRight);
    /////////********* Start swing draw
    int impactPoint = pGolf->GetImpactStep();
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
        Vector3 thetaPoint = thetaVec[i];
        clubHead -= hand;
        VertexPositionColor body(swingOrigin, Colors::Red);
        VertexPositionColor arm(hand, Colors::Green);
        VertexPositionColor shaft(clubHead, Colors::Blue);
        VertexPositionColor thetaVertex(thetaPoint, Colors::White);
        m_batch->DrawLine(shoulder,arm);
    }
    */
}

void Game::DrawWorld()
{
    // draw world grid
    Vector3 xAxis(2.f, 0.f, 0.f);
    Vector3 zAxis(0.f, 0.f, 2.f);
    Vector3 origin = Vector3::Zero;
    size_t divisions = 20;
    for (size_t i = 0; i <= divisions + 10; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;
        Vector3 scale = xAxis * fPercent + origin;
        if (scale.x == 0.0f)
        {
            VertexPositionColor v1(scale - zAxis, Colors::Green);
            VertexPositionColor v2(scale + zAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - zAxis, Colors::Green);
            VertexPositionColor v2(scale + zAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }

    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = zAxis * fPercent + origin;

        if (scale.z == 0.0f)
        {
            //VertexPositionColor v1(scale - xaxis, Colors::Red);
            //VertexPositionColor v2(scale + xaxis, Colors::Red);
            //m_batch->DrawLine(v1, v2);
        }
        else
        {
            VertexPositionColor v1(scale - xAxis, Colors::Green);
            VertexPositionColor v2(scale + xAxis, Colors::Green);
            m_batch->DrawLine(v1, v2);
        }
    }
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
    m_titleFont.reset();
    m_textFont.reset();
    m_spriteBatch.reset();
    m_keyboard.reset();
    m_kbStateTracker.reset();
    //Powerbar
    m_powerFrameTexture.Reset();
    m_powerMeterTexture.Reset();
    m_powerImpactTexture.Reset();
    m_powerBackswingTexture.Reset();

    // Charcter
    m_character.reset();
    m_characterTexture.Reset();
    m_character0.reset();
    m_character0Texture.Reset();
    m_character1.reset();
    m_character1Texture.Reset();
    m_character2.reset();
    m_character2Texture.Reset();
    m_characterBackgroundTexture.Reset();
    // end
    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}