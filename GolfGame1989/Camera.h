#pragma once
#include <algorithm>
#include "Utility.h"

class Camera
{
public:
    Camera();
    Camera(float aWidth, float aHeight);

    //Camera(DirectX::XMFLOAT3 aHomePos = { 0.0f,0.0f,0.0f }, float aHomePitch = 0.0f, float aHomeYaw = 0.0f) noexcept; //Chili
    //Camera(DirectX::XMFLOAT3 aHomePos, DirectX::XMFLOAT3 
    void Reset();
    void RotateAtSpeed(float aDx, float aDy);  //Chili
    void Rotate(DirectX::SimpleMath::Vector3 aAxis, float aDegrees); //Pavel
    void TranslateAtSpeed(DirectX::SimpleMath::Vector3 aTranslation); //Chili
    DirectX::SimpleMath::Vector3 GetPos() const { return m_position; }; 
    DirectX::SimpleMath::Vector3 GetHomePos() const { return m_homePosition; };
    DirectX::SimpleMath::Matrix GetProjectionMatrix() const { return m_projectionMatrix; };
    DirectX::SimpleMath::Vector3 GetTargetPos() const { return m_target; };
    DirectX::SimpleMath::Vector3 GetUp() const { return m_up; };
    void OnResize(uint32_t aWidth, uint32_t aHeight);
    void SetTargetPos(const DirectX::SimpleMath::Vector3 aTarget);
    void SetPos(const DirectX::SimpleMath::Vector3 aPos);
    void SetHomePos(const DirectX::SimpleMath::Vector3 aHomePos);



private:
    void InitializeOrthoganalMatrix(); //Pavel
    void InitializeProjectionMatrix(); //Pavel
    void InitializeViewMatrix(); //Pavel
    void UpdateUp();
    void UpdateProjectionMatrix();


    DirectX::SimpleMath::Vector3    m_position;
    DirectX::SimpleMath::Vector3    m_homePosition;
    DirectX::SimpleMath::Vector3    m_target;
    DirectX::SimpleMath::Vector3    m_up;

    float                           m_homePitch;
    float                           m_pitch;
    float                           m_homeYaw;
    float                           m_yaw;
    //float                           m_frustumAngle;
    int                             m_clientWidth;
    int                             m_clientHeight;
    float                           m_nearPlane;
    float                           m_farPlane;

    DirectX::SimpleMath::Matrix     m_viewMatrix;
    DirectX::SimpleMath::Matrix     m_projectionMatrix;
    DirectX::SimpleMath::Matrix     m_orthogonalMatrix;

    const float                     m_posTravelSpeed = 0.5f;
    const float                     m_rotationTravelSpeed = 0.004f;
};

