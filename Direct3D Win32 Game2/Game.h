//
// Game.h
//

#pragma once
#include "StepTimer.h"
#include "Golf.h"
#include "Keyboard.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:
    Game() noexcept;
    //~Game() = default;
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    void ResetPowerMeter() { m_powerMeterBarRect.left = m_powerMeterImpactPoint; m_powerMeterBarRect.right = m_powerMeterImpactPoint; };
    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:

    void DrawSwing();
    void DrawSwing2();
    void DrawProjectile();
    void DrawWorld();
    void DrawStartScreen();
    void Update(DX::StepTimer const& timer);
    void UpdateCamera(DX::StepTimer const& timer);
    void Render();
    void RenderUI();
    void DrawSwingUI();
    void RenderUIPowerBar();
    void SetGameCamera(int aCamera);

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

    // WLJ add start
    using VertexType = DirectX::VertexPositionColor;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::BasicEffect> m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    // End

    //world start 
    DirectX::SimpleMath::Matrix m_world;

    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster; // WLJ anti-aliasing

    int m_gameCamera = 1;
    float m_cameraRotationX = 2.0;
    float m_cameraRotationY = 0.0;
    //world end

    // WLJ added for drawing projectile and swing
    Golf* pGolf;
    GolfPlay* pPlay;
    int arcCount = 0;

    // WLJ added for displaying text
    std::unique_ptr<DirectX::SpriteFont> m_font;
    std::unique_ptr<DirectX::SpriteFont> m_titleFont;
    DirectX::SimpleMath::Vector2 m_fontPos;
    DirectX::SimpleMath::Vector2 m_fontPos2;
    DirectX::SimpleMath::Vector2 m_fontPosDebug;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

    // WLJ add for mouse and keybord interface
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    std::unique_ptr<DirectX::Mouse> m_mouse;

    //Keyboard::KeyboardStateTracker m_kbStateTracker;

    // WLJ swing power bar UI
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerFrameTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerMeterTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerImpactTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerBackswingTexture;
    DirectX::SimpleMath::Vector2 m_powerBarFramePos;
    DirectX::SimpleMath::Vector2 m_powerBarMeterPos;
    DirectX::SimpleMath::Vector2 m_powerBarImpactPos;
    DirectX::SimpleMath::Vector2 m_powerBarBackswingPos;
    DirectX::SimpleMath::Vector2 m_powerBarFrameOrigin;
    DirectX::SimpleMath::Vector2 m_powerBarMeterOrigin;
    DirectX::SimpleMath::Vector2 m_powerBarImpactOrigin;
    DirectX::SimpleMath::Vector2 m_powerBarBackswingOrigin;

    RECT m_powerMeterBarRect;
    RECT m_powerMeterFrameRect;
    RECT m_powerMeterImpactRect;
    RECT m_powerMeterBackswingRect;

    float m_powerMeterSize;
    float m_powerMeterImpactPoint;
    float m_powerMeterBarScale;

    int m_gameState = 0;
};