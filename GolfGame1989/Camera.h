#pragma once
#include <algorithm>
#include "StepTimer.h"
#include "Utility.h"

enum class CameraState
{
    CAMERASTATE_DEFAULT,
    CAMERASTATE_CAMERA1,
    CAMERASTATE_CAMERA2,
    CAMERASTATE_CAMERA3,
    CAMERASTATE_CAMERA4,
    CAMERASTATE_CAMERA5,
    CAMERASTATE_CAMERA6,
    CAMERASTATE_CAMERACLASS,
    CAMERASTATE_PRESWINGVIEW,
    CAMERASTATE_PROJECTILEFLIGHTVIEW,
    CAMERASTATE_SWINGVIEW,
};

class Camera
{
public:
    Camera();
    Camera(int aWidth, int aHeight);

    //Camera(DirectX::XMFLOAT3 aHomePos = { 0.0f,0.0f,0.0f }, float aHomePitch = 0.0f, float aHomeYaw = 0.0f) noexcept; //Chili
    //Camera(DirectX::XMFLOAT3 aHomePos, DirectX::XMFLOAT3 

    CameraState GetCameraState() const { return m_cameraState; };
    DirectX::SimpleMath::Vector3 GetPos() const { return m_position; }; 
    DirectX::SimpleMath::Vector3 GetHomePos() const { return m_homePosition; };
    DirectX::SimpleMath::Matrix GetProjectionMatrix() const { return m_projectionMatrix; };
    DirectX::SimpleMath::Matrix GetViewMatrix() const { return m_viewMatrix; };
    DirectX::SimpleMath::Vector3 GetTargetPos() const { return m_target; };
    DirectX::SimpleMath::Vector3 GetUp() const { return m_up; };
    bool IsCameraAtDestination();
    void OnResize(uint32_t aWidth, uint32_t aHeight);
    void Reset();
    void ResetIsCameraAtDestination() { m_isCameraAtDestination = false; };
    void Rotate(DirectX::SimpleMath::Vector3 aAxis, float aDegrees); //Pavel
    void RotateAtSpeed(float aDx, float aDy);  //Chili
    void RotateCounterClockWise();
    void RotateClockWise();
    
    void SetCameraState(const CameraState aCameraState);
    void SetDestinationPos(const DirectX::SimpleMath::Vector3 aDestPos);
    void SetHomePos(const DirectX::SimpleMath::Vector3 aHomePos);
    void SetTargetPos(const DirectX::SimpleMath::Vector3 aTarget);
    void SetPos(const DirectX::SimpleMath::Vector3 aPos);

    void TranslateAtSpeed(DirectX::SimpleMath::Vector3 aTranslation); //Chili

    void UpdateCamera();
    void UpdatePitchYaw(const float aPitch, const float aYaw);
    void UpdatePos(const float aX, const float aY, const float aZ);
    void SetCameraEndPos(DirectX::SimpleMath::Vector3 aEndPos);   
    void SetCameraStartPos(DirectX::SimpleMath::Vector3 aStartPos);
    void SetTargetEndPos(DirectX::SimpleMath::Vector3 aEndPos);
    void SetTargetStartPos(DirectX::SimpleMath::Vector3 aStartPos);
    
    void TransitionCameraBetweenPos(DX::StepTimer const& aTimer);

private:
    void InitializeOrthoganalMatrix(); //Pavel
    void InitializeProjectionMatrix(); //Pavel
    void InitializeViewMatrix(); //Pavel
    void UpdateUp();
    void UpdateProjectionMatrix();
    void UpdateOrthoganalMatrix();
    void UpdateViewMatrix();

    DirectX::SimpleMath::Vector3    m_destinationPosition;
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

    const float                     m_posTravelSpeed = 0.4f;
    const float                     m_rotationTravelSpeed = 1.4f;

    bool                            m_isCameraAtDestination;

    CameraState                     m_cameraState;

    //////////////////////////////////////////////////////////////

    DirectX::SimpleMath::Matrix     m_rotationMatrix;
    DirectX::XMVECTOR               m_defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR               m_forward =        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::XMVECTOR               m_defaultRight =   DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR               m_right =          DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

    float                           m_moveBackForward = 0.0f;
    float                           m_moveLeftRight = 0.0f;
    float                           m_moveUpDown = 0.0f;

    DirectX::SimpleMath::Vector3    m_cameraStartPos;
    DirectX::SimpleMath::Vector3    m_cameraEndPos;
    DirectX::SimpleMath::Vector3    m_targetStartPos;
    DirectX::SimpleMath::Vector3    m_targetEndPos;
    const float                     m_cameraTransitionSpeed = .5f;
};

