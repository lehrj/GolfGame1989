//
// Game.h
//

#pragma once
#include "StepTimer.h"
#include "Golf.h"
#include "Keyboard.h"
#include "AnimatedTexture.h"
#include "SpriteSheet.h"
#include "WICTextureLoader.h"
#include "Camera.h"


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
    void Clear();
    void CreateDevice();
    void CreateResources();

    void DrawCameraFocus();
    void DrawIntroScreen();
    void DrawIntroScreen2();
    void DrawMenuCharacterSelect();
    void DrawMenuEnvironmentSelect();
    void DrawMenuMain();
    void DrawPowerBarUI();
    void DrawProjectile();
    void DrawProjectileRealTime();
    void DrawShotTimerUI();
    void DrawStartScreen();
    void DrawSwing();
    void DrawSwingUI();
    void DrawUI();
    void DrawWorld();

    void OnDeviceLost();
    void Present();
    void Render();
    void SetGameCamera(int aCamera);

    void Update(DX::StepTimer const& timer);
    void UpdateCamera(DX::StepTimer const& timer);
    void UpdateInput();

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
    DX::StepTimer                                   m_flightStepTimer;
    float                                           m_projectileTimer = 0.0f;

    // WLJ add start
    using VertexType = DirectX::VertexPositionColor;

    std::unique_ptr<DirectX::CommonStates>          m_states;
    std::unique_ptr<DirectX::BasicEffect>           m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;

    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    // End

    //world start 
    DirectX::SimpleMath::Matrix                     m_world;
    DirectX::SimpleMath::Matrix                     m_view;
    DirectX::SimpleMath::Matrix                     m_proj;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_raster; // WLJ anti-aliasing
    //world end

    // WLJ added for drawing projectile and swing
    Golf*                                           pGolf;
    GolfPlay*                                       pPlay;
    int                                             m_swingPathStep = 0;
    int                                             m_projectilePathStep = 0;

    // WLJ add for mouse and keybord interface
    std::unique_ptr<DirectX::Keyboard>              m_keyboard;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;
    DirectX::Keyboard::KeyboardStateTracker         m_kbStateTracker;

    // WLJ added for displaying text
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

    // WLJ swing power bar UI
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

    // WLJ Intro screen logos
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bmwLogoTexture;
    DirectX::SimpleMath::Vector2 m_bmwLogoPos;
    DirectX::SimpleMath::Vector2 m_bmwLogoOrigin;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_jiLogoTexture;
    DirectX::SimpleMath::Vector2 m_jiLogoPos;
    DirectX::SimpleMath::Vector2 m_jiLogoOrigin;


    // WLJ variable to hold selected menu item
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
    GameState                                       m_currentState;

    enum class GameCamera
    {
        GAMECAMERA_DEFAULT,
        GAMECAMERA_CAMERA1,
        GAMECAMERA_CAMERA2,
        GAMECAMERA_CAMERA3,
        GAMECAMERA_CAMERA4,
        GAMECAMERA_CAMERA5,
        GAMECAMERA_CAMERA6,
        GAMECAMERA_TOSWINGVIEW,
        GAMECAMERA_CAMERACLASS,
        GAMECAMERA_PRESWINGVIEW,
        GAMECAMERA_PROJECTILEFLIGHTVIEW,
        GAMECAMERA_SWINGVIEW,
    };

    CameraState                                 m_cState;
    GameCamera                                  m_currentCamera;
    void                                        SetGameCamera(GameCamera aCameraState);
    float                                       m_cameraRotationX = 2.0;
    float                                       m_cameraRotationY = 2.0;
    DirectX::SimpleMath::Vector3                m_cameraTarget = DirectX::SimpleMath::Vector3::Zero;
    //float                                       m_cameraTargetX = 0.0;
    //float                                       m_cameraTargetY = 0.0;
    //float                                       m_cameraTargetZ = 0.0;
    float                                       m_cameraZoom = 0.0f;
    float                                       m_cameraMovementSpeed = 0.01;
    
    Camera*                                     pCamera;

    DirectX::SimpleMath::Vector3                m_ballPos = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3                m_shootOrigin = DirectX::SimpleMath::Vector3(-2.f, .0f, 0.f);
    DirectX::SimpleMath::Vector3                m_swingOrigin = DirectX::SimpleMath::Vector3(-2.0087f, .04f, 0.f);
};



/*

void Game::DrawIntroScreen2()
{
    float startDelay = 5.f;
    float logoDisplayGap = 4.f;
    float logoDisplayDuration = 10.f;
    //float timeStamp = m_timer.GetTotalSeconds() - startDelay;
    float timeStamp = m_timer.GetTotalSeconds();
    float fadeDuration = 4.f;

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

        //if (timeStamp < fadeDuration)  // fade in
        if (timeStamp < fadeInEnd1)  // fade in
        {
            //float colorIntensity = timeStamp / fadeDuration;
            float colorIntensity = (timeStamp - startDelay) / fadeDuration;
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_jiLogoTexture.Get(), m_jiLogoPos, nullptr, fadeColor, 0.f, m_jiLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        //else if (timeStamp > logoDisplayDuration - fadeDuration) // fade out
        else if (timeStamp < fadeOutEnd1) // fade out
        {
            float startFade = logoDisplayDuration - fadeDuration;

            //float colorIntensity = (timeStamp - startFade) / (logoDisplayDuration - startFade);
            float colorIntensity = (timeStamp - startFade - startDelay) / (logoDisplayDuration - startFade);
            colorIntensity = 1.f - colorIntensity;
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
    else if (timeStamp < fadeInEnd2)
    {
        std::string textLine = "A Baltimore Magic Werks Production";
        float textLinePosX = m_bitwiseFontPos.x;
        float textLinePosY = m_bitwiseFontPos.y + 450;
        DirectX::SimpleMath::Vector2 textLinePos(textLinePosX, textLinePosY);
        DirectX::SimpleMath::Vector2 textLineOrigin = m_bitwiseFont->MeasureString(textLine.c_str()) / 2.f;
        //if (timeStamp < fadeDuration + logoDisplayDuration)  // fade in
        if (timeStamp < fadeInEnd2)  // fade in
        {
            float colorIntensity = (timeStamp - logoDisplayDuration) / fadeDuration;
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        //else if (timeStamp > fadeOutStart2) // fade out
        else if (timeStamp > fadeOutEnd2) // fade out
        {
            float startFade = logoDisplayDuration + logoDisplayDuration - fadeDuration;

            float colorIntensity = (timeStamp - startFade) / (logoDisplayDuration + logoDisplayDuration - startFade);
            colorIntensity = 1.f - colorIntensity;
            fadeColor.f[0] = colorIntensity;
            fadeColor.f[1] = colorIntensity;
            fadeColor.f[2] = colorIntensity;
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
        else // display at full intesity
        {
            m_spriteBatch->Draw(m_bmwLogoTexture.Get(), m_bmwLogoPos, nullptr, fadeColor, 0.f, m_bmwLogoOrigin);
            m_bitwiseFont->DrawString(m_spriteBatch.get(), textLine.c_str(), textLinePos, fadeColor, 0.f, textLineOrigin);
        }
    }
    if (timeStamp > logoDisplayDuration + logoDisplayDuration)
    {
        m_currentState = GameState::GAMESTATE_STARTSCREEN;
    }
}

*/