//
// Game.h
//

#pragma once
#include "Golf.h"
#include "Lighting.h"
#include "Keyboard.h"
#include "AnimatedTexture.h"
#include "SpriteSheet.h"
#include "WICTextureLoader.h"
#include "Camera.h"
#include "sounds.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:
    Game() noexcept;
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

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

    // Audio
    void OnNewAudioDevice() { m_retryAudio = true; };

private:
    void AudioPlayMusic(XACT_WAVEBANK_AUDIOBANK aSFX);
    void AudioPlaySFX(XACT_WAVEBANK_AUDIOBANK aSFX);
    void Clear();
    void CreateDevice();
    void CreateResources();

    void DrawBridge(const DirectX::SimpleMath::Vector3 aPos, const float aRotation);
    void DrawDebugLines();
    void DrawCameraFocus();
    void DrawFlagAndHole();
    void DrawFlagHoleFixture(const DirectX::SimpleMath::Vector3 aPos, const float aVariation);
    void DrawHydraShot();
    void DrawIntroScreen();
    void DrawMenuCharacterSelect();
    void DrawMenuEnvironmentSelect();
    void DrawMenuMain();
    void DrawPowerBarUI();
    void DrawProjectile();
    void DrawProjectileRealTime();
    void DrawSand();
    void DrawShotAimArrow();
    void DrawShotAimCone();
    void DrawShotTimerUI();
    void DrawStartScreen();
    void DrawSwing();
    void DrawSwingUI();
    void DrawTeeBox();
    void DrawTeeBoxFixture(const DirectX::SimpleMath::Vector3 aPos, const float aVariation);
    void DrawTree03(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);  // V column tree
    void DrawTree04(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);  // circle tree
    void DrawTree05(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);
    void DrawTree06(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);  // tri
    void DrawTree07(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);  // invert V
    void DrawTree08(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);
    void DrawTree09(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);  // invert v
    void DrawTree10(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);
    void DrawTree11(const DirectX::SimpleMath::Vector3 aTreePos, const float aVariation);
    void DrawUI();
    void DrawWater();
    void DrawWorld();
    void DrawWorld12thHole();

    bool InitializeTerrainArray();
    void OnDeviceLost();
    void Present();
    void Render();
    void ResetGamePlay();
    void ResetPowerMeter() { m_powerMeterBarRect.left = static_cast<long>(m_powerMeterImpactPoint); m_powerMeterBarRect.right = static_cast<long>(m_powerMeterImpactPoint); };
    void Update(DX::StepTimer const& aTimer);
    void UpdateInput(DX::StepTimer const& aTimer);

    Camera*                                         pCamera;
    Golf*                                           pGolf;
    GolfPlay*                                       pPlay;
    
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

    // Rendering loop timers
    DX::StepTimer                                   m_timer;
    DX::StepTimer                                   m_flightStepTimer;
    double                                          m_projectileTimer = 0.0;

    // variables for lighting rendering
    using VertexTypeLighting = DirectX::VertexPositionNormalColor;
    //std::shared_ptr<DirectX::NormalMapEffect>       m_effectNormColorLighting;
    std::shared_ptr<DirectX::BasicEffect>       m_effectNormColorLighting;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexTypeLighting>> m_batchNormColorLighting;

    using VertexType2 = DirectX::VertexPositionNormalColor;
    std::unique_ptr<DirectX::BasicEffect>           m_effect2;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType2>> m_batch2;

    using VertexType = DirectX::VertexPositionColor;
    std::unique_ptr<DirectX::CommonStates>          m_states;
    std::unique_ptr<DirectX::BasicEffect>           m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    std::unique_ptr<DirectX::GeometricPrimitive>    m_shape;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    
    DirectX::SimpleMath::Matrix                     m_world;
    DirectX::SimpleMath::Matrix                     m_view;
    DirectX::SimpleMath::Matrix                     m_proj;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_raster; // WLJ anti-aliasing

    // for drawing projectile and swing  
    int                                             m_swingPathStep = 0;
    int                                             m_projectilePathStep = 0;

    // Mouse and keybord interface
    std::unique_ptr<DirectX::Keyboard>              m_keyboard;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;
    DirectX::Keyboard::KeyboardStateTracker         m_kbStateTracker;

    // for displaying text
    std::unique_ptr<DirectX::SpriteFont>            m_font;
    std::unique_ptr<DirectX::SpriteFont>            m_titleFont;
    std::unique_ptr<DirectX::SpriteFont>            m_bitwiseFont;
    DirectX::SimpleMath::Vector2                    m_fontPos;
    DirectX::SimpleMath::Vector2                    m_fontPos2;
    DirectX::SimpleMath::Vector2                    m_fontPosDebug;
    DirectX::SimpleMath::Vector2                    m_fontMenuPos;
    DirectX::SimpleMath::Vector2                    m_bitwiseFontPos;
    std::unique_ptr<DirectX::SpriteBatch>           m_spriteBatch;

    // Golf Character Texture
    std::unique_ptr<AnimatedTexture>                m_character;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_characterTexture;
    DirectX::SimpleMath::Vector2                    m_characterPos;

    std::unique_ptr<AnimatedTexture>                m_character0;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_character0Texture;
    DirectX::SimpleMath::Vector2                    m_character0Pos;

    std::unique_ptr<AnimatedTexture>                m_character1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_character1Texture;
    DirectX::SimpleMath::Vector2                    m_character1Pos;

    std::unique_ptr<AnimatedTexture>                m_character2;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_character2Texture;
    DirectX::SimpleMath::Vector2                    m_character2Pos;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_characterBackgroundTexture;
    DirectX::SimpleMath::Vector2                    m_characterBackgroundPos;
    DirectX::SimpleMath::Vector2                    m_characterBackgroundOrigin;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_environSelectCalmTexture;
    DirectX::SimpleMath::Vector2                    m_environSelectCalmPos;
    DirectX::SimpleMath::Vector2                    m_environSelectCalmOrigin;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_environSelectBreezyTexture;
    DirectX::SimpleMath::Vector2                    m_environSelectBreezyPos;
    DirectX::SimpleMath::Vector2                    m_environSelectBreezyOrigin;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_environSelectAlienTexture;
    DirectX::SimpleMath::Vector2                    m_environSelectAlienPos;
    DirectX::SimpleMath::Vector2                    m_environSelectAlienOrigin;

    // Swing power bar UI
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerFrameTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerMeterTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerImpactTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_powerBackswingTexture;
    DirectX::SimpleMath::Vector2                    m_powerBarFramePos;
    DirectX::SimpleMath::Vector2                    m_powerBarMeterPos;
    DirectX::SimpleMath::Vector2                    m_powerBarImpactPos;
    DirectX::SimpleMath::Vector2                    m_powerBarBackswingPos;
    DirectX::SimpleMath::Vector2                    m_powerBarFrameOrigin;
    DirectX::SimpleMath::Vector2                    m_powerBarMeterOrigin;
    DirectX::SimpleMath::Vector2                    m_powerBarImpactOrigin;
    DirectX::SimpleMath::Vector2                    m_powerBarBackswingOrigin;

    RECT                                            m_powerMeterBarRect;
    RECT                                            m_powerMeterFrameRect;
    RECT                                            m_powerMeterImpactRect;
    RECT                                            m_powerMeterBackswingRect;

    float                                           m_powerMeterSize;
    float                                           m_powerMeterImpactPoint;
    float                                           m_powerMeterBarScale;

    // Intro screen logos
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bmwLogoTexture;
    DirectX::SimpleMath::Vector2                    m_bmwLogoPos;
    DirectX::SimpleMath::Vector2                    m_bmwLogoOrigin;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_jiLogoTexture;
    DirectX::SimpleMath::Vector2                    m_jiLogoPos;
    DirectX::SimpleMath::Vector2                    m_jiLogoOrigin;

    // Variable to hold selected menu item
    int                                             m_menuSelect = 0;

    enum class GameState
    {
        GAMESTATE_INTROSCREEN,
        GAMESTATE_STARTSCREEN,
        GAMESTATE_MAINMENU,
        GAMESTATE_CHARACTERSELECT,
        GAMESTATE_COURSESELECT,
        GAMESTATE_ENVIRONTMENTSELECT,
        GAMESTATE_GAMEPLAY
    };
    GameState                                   m_currentGameState;

    enum class UiState
    {
        UISTATE_SWING,
        UISTATE_SHOT,
        UISTATE_SCORE
    };
    UiState                                     m_currentUiState;

    // Audio 
    std::unique_ptr<DirectX::AudioEngine>       m_audioEngine;
    bool                                        m_retryAudio;
    float                                       m_musicVolume = 0.9f;
    float                                       m_sfxVolume = 0.5f;
    std::unique_ptr<DirectX::WaveBank>          m_audioBank;
    std::unique_ptr<DirectX::SoundStreamInstance> m_audioMusicStream;
    std::unique_ptr<DirectX::SoundStreamInstance> m_audioEffectStream;

    const bool                                  m_isInDebugMode = true;

    std::vector<DirectX::VertexPositionColor>   m_terrainVector;

    DirectX::VertexPositionColor*               m_terrainVertexArray;
    DirectX::VertexPositionColor*               m_terrainVertexArrayBase;
    int                                         m_terrainVertexCount;

    float m_debugHeight = 0.0;
    float m_debugXpoint = 0.0;
    float m_debugZpoint = 0.0;
};