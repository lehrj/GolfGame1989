#include "pch.h"
#include "Lighting.h"


Lighting::Lighting()
{

}

Lighting::LightingState Lighting::GetLightingState()
{
    return m_currentLightingState;
}

void Lighting::SetLightControlDirection(DirectX::SimpleMath::Vector3 aDirection)
{
    aDirection.Normalize();
    m_lightControlDirection = aDirection;
}

void Lighting::SetLighting(LightingState aLightState)
{
    m_currentLightingState = aLightState;
}

void Lighting::UpdateLighting(std::shared_ptr<DirectX::BasicEffect> aEffect, const double aTimer)
{
    const float timeStamp = static_cast<float>(aTimer);

    aEffect->EnableDefaultLighting();

    if (m_currentLightingState == LightingState::LIGHTINGSTATE_STARTSCREEN)
    {
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            //ilights->EnableDefaultLighting();
            ilights->SetLightEnabled(0, true);
            ilights->SetLightEnabled(1, true);
            ilights->SetLightEnabled(2, true);

            auto time = static_cast<float>(aTimer);

            float yaw = time * 0.4f;
            float pitch = time * 0.7f;
            float roll = time * 1.1f;

            auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, pitch, 0.0);
            auto quat1 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, yaw, 0.0);
            auto quat2 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, roll, 0.0);

            auto quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, -roll, 0.0);

            DirectX::SimpleMath::Vector3 axis = DirectX::SimpleMath::Vector3::UnitZ;

            DirectX::SimpleMath::Vector3 light0 = XMVector3Rotate(axis, quat0);
            DirectX::SimpleMath::Vector3 light1 = XMVector3Rotate(axis, quat1);
            DirectX::SimpleMath::Vector3 light2 = XMVector3Rotate(axis, quat2);
            light0.x += 1.0;
            light0.Normalize();
            light1.x += 1.0;
            light1.Normalize();
            light2.x += 1.0;
            light2.Normalize();

            quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(roll, 0.0, 0.0);
            DirectX::SimpleMath::Vector3 light = XMVector3Rotate(axis, quat);

            light0 = light;
            light1 = light;
            light2 = light;

            ilights->SetLightDirection(0, light0);
            ilights->SetLightDirection(1, light1);
            ilights->SetLightDirection(2, light2);

            m_lightPos0 = light0;
            m_lightPos1 = light1;
            m_lightPos2 = light2;
        }
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_MANUAL)
    {
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightDirection(0, m_lightPos0);
            ilights->SetLightDirection(1, m_lightPos1);
            ilights->SetLightDirection(2, m_lightPos2);
        }
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_NULL)
    {       
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightEnabled(0, true);
            ilights->SetLightEnabled(1, true);
            ilights->SetLightEnabled(2, true);

            DirectX::SimpleMath::Vector3 light = m_lightControlDirection;

            DirectX::SimpleMath::Vector3 light0 = light;
            DirectX::SimpleMath::Vector3 light1 = light;
            DirectX::SimpleMath::Vector3 light2 = light;

            ilights->SetLightDirection(0, light0);
            ilights->SetLightDirection(1, light1);
            ilights->SetLightDirection(2, light2);

            m_lightPos0 = light0;
            m_lightPos1 = light1;
            m_lightPos2 = light2;
        }       
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_)
    {
        aEffect->EnableDefaultLighting();

        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightEnabled(0, true);
            ilights->SetLightEnabled(1, true);
            ilights->SetLightEnabled(2, true);

            auto time = static_cast<float>(aTimer);

            float yaw = time * 0.4f;
            float pitch = time * 0.7f;
            float roll = time * 11.1f;

            auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, pitch, 0.0);
            auto quat1 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, yaw, 0.0);
            auto quat2 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, roll, 0.0);

            auto quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, -roll, 0.0);

            DirectX::SimpleMath::Vector3 axis = -DirectX::SimpleMath::Vector3::UnitZ;

            DirectX::SimpleMath::Vector3 light0 = XMVector3Rotate(axis, quat0);
            DirectX::SimpleMath::Vector3 light1 = XMVector3Rotate(axis, quat1);
            DirectX::SimpleMath::Vector3 light2 = XMVector3Rotate(axis, quat2);
            float offset = 7.0;
            light0.x -= offset;
            light0.Normalize();
            light1.x -= offset;
            light1.Normalize();
            light2.x -= offset;
            light2.Normalize();

            auto light = -DirectX::SimpleMath::Vector3::UnitX;

            light0 = light;
            light1 = light;
            light2 = light;

            ilights->SetLightDirection(0, light0);
            ilights->SetLightDirection(1, light1);
            ilights->SetLightDirection(2, light2);

            m_lightPos0 = light0;
            m_lightPos1 = light1;
            m_lightPos2 = light2;
        }
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_TEST01)
    {
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightEnabled(0, false);
            ilights->SetLightEnabled(1, false);
            ilights->SetLightEnabled(2, true);
            auto time = static_cast<float>(aTimer);
            float yaw = cosf(-timeStamp * 1.2f);
            float roll = cosf(-timeStamp * 1.2f);
            auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(-yaw, 0.0, 0.0);
            auto quat1 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yaw, 0.0, 0.0);
            auto quat2 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, 0.0, roll);
            auto light0 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat0);
            auto light1 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat1);
            auto light2 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat2);

            float roll2 = time * 3.1f;
            auto quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, roll2, 0.0);
            DirectX::SimpleMath::Vector3 axis = -DirectX::SimpleMath::Vector3::UnitZ;
            DirectX::SimpleMath::Vector3 light = XMVector3Rotate(axis, quat);
            light.x += 1.0;

            light.Normalize();
            light0 = light;

            ilights->SetLightDirection(0, light0);
            ilights->SetLightDirection(1, light1);
            ilights->SetLightDirection(2, light2);
            m_lightPos0 = light0;
            m_lightPos1 = light1;
            m_lightPos2 = light2;
        }
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_TESTSUNMOVE)
    {
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightEnabled(0, false);
            ilights->SetLightEnabled(1, false);
            ilights->SetLightEnabled(2, true);
            auto time = static_cast<float>(aTimer);
            float yaw = -timeStamp * 1.0f;
            float roll = -timeStamp * 1.0f;
            auto quat0 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(-yaw, 0.0, 0.0);
            auto quat1 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yaw, 0.0, 0.0);
            auto quat2 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, 0.0, roll);
            auto quat3 = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, 0.0, roll);
            auto light0 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat0);
            auto light1 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat1);
            auto light2 = XMVector3Rotate(DirectX::SimpleMath::Vector3::UnitX, quat2);
            DirectX::SimpleMath::Vector3 light3 = XMVector3Rotate(m_lightControlDirection, quat3);
          
            float roll2 = time * 3.1f;
            auto quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.0, roll2, 0.0);
            DirectX::SimpleMath::Vector3 axis = -DirectX::SimpleMath::Vector3::UnitZ;
            DirectX::SimpleMath::Vector3 light = XMVector3Rotate(axis, quat);

            light3.y -= 0.8;
            light3.Normalize();
            light0 = light3;
            light1 = light3;
            light2 = light3;
            ilights->SetLightDirection(0, light0);
            ilights->SetLightDirection(1, light1);
            ilights->SetLightDirection(2, light2);
            m_lightPos0 = light0;
            m_lightPos1 = light1;
            m_lightPos2 = light2;
        }
    }
    else if (m_currentLightingState == LightingState::LIGHTINGSTATE_TESTSUNMOVE2)
    {
        auto ilights = dynamic_cast<DirectX::IEffectLights*>(aEffect.get());
        if (ilights)
        {
            ilights->SetLightEnabled(0, true);
            ilights->SetLightEnabled(1, true);
            ilights->SetLightEnabled(2, true);

            float pointAngle = Utility::ToRadians(45.0f);
            float axisAngle = pointAngle - Utility::ToRadians(90.0f);
            DirectX::SimpleMath::Vector3 axis = DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitY, DirectX::SimpleMath::Matrix::CreateRotationZ(axisAngle));
            DirectX::SimpleMath::Vector3 point = DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitY, DirectX::SimpleMath::Matrix::CreateRotationZ(pointAngle));
            float pitch = (static_cast<float>(aTimer) * 0.2f) - 5.0f;
            pitch = Utility::ToRadians(175.0f);
            DirectX::SimpleMath::Vector3 lightDirection = DirectX::SimpleMath::Vector3::Transform(point, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(axis, pitch));
            
            lightDirection.Normalize();

            ilights->SetLightDirection(0, lightDirection);
            ilights->SetLightDirection(1, lightDirection);
            ilights->SetLightDirection(2, lightDirection);
            m_lightPos0 = lightDirection;
            m_lightPos1 = lightDirection;
            m_lightPos2 = lightDirection;
        }
    }
}
