#pragma once


class Lighting
{
public:
    enum class LightingState
    {
        LIGHTINGSTATE_STARTSCREEN,
        LIGHTINGSTATE_GAMEPLAY,
    };

    Lighting();

    LightingState GetLightingState();
    void SetLighting(LightingState aLightState);
    void UpdateLighting(std::shared_ptr<DirectX::NormalMapEffect> aEffect, const double aTimer);

private:

    LightingState                               m_currentLightingState;

    DirectX::SimpleMath::Vector3                m_cameraFocusPos = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3                m_lightPos0 = DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3                m_lightPos1 = DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3                m_lightPos2 = DirectX::SimpleMath::Vector3::UnitX;

    // light pos for m_effect2
    DirectX::SimpleMath::Vector3                m_lightEffect2Pos0 = DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3                m_lightEffect2Pos1 = DirectX::SimpleMath::Vector3::UnitX;
    DirectX::SimpleMath::Vector3                m_lightEffect2Pos2 = DirectX::SimpleMath::Vector3::UnitX;


};
