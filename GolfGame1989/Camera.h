#pragma once
//#include <algorithm>
#include "StepTimer.h"
#include "Utility.h"


enum class CameraState
{
    CAMERASTATE_TRANSITION,
    CAMERASTATE_FIRSTPERSON,
    CAMERASTATE_PRESWINGVIEW,
    CAMERASTATE_PROJECTILEFLIGHTVIEW,
    CAMERASTATE_SWINGVIEW,
    CAMERASTATE_RESET,
    CAMERASTATE_TRANSTONEWSHOT
};

// Class to manage game camera
class Camera
{
public:
    Camera();
    Camera(int aWidth, int aHeight);

    CameraState GetCameraState() const { return m_cameraState; };
    float GetAimTurnRate() const { return m_aimTurnRate; };

    DirectX::SimpleMath::Vector3 GetHomePos() const { return m_homePosition; };
    double GetPitch() const { return m_pitch; };
    DirectX::SimpleMath::Vector3 GetPos() const { return m_position; };
    DirectX::SimpleMath::Vector3 GetPreSwingCamPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees);
    DirectX::SimpleMath::Vector3 GetPreSwingTargPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees);
    DirectX::SimpleMath::Matrix GetProjectionMatrix() const { return m_projectionMatrix; };
    DirectX::SimpleMath::Vector3 GetSwingCamPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees);
    DirectX::SimpleMath::Vector3 GetSwingTargPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees);
    DirectX::SimpleMath::Matrix GetViewMatrix() const { return m_viewMatrix; };
    DirectX::SimpleMath::Vector3 GetTargetPos() const { return m_target; };
    DirectX::SimpleMath::Vector3 GetUp() const { return m_up; };
    double GetYaw() const { return m_yaw; };
    void InintializePreSwingCamera(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees);
    bool IsCameraAtDestination();
    void OnResize(uint32_t aWidth, uint32_t aHeight);
    void Reset();
    void ResetCameraTransition(DX::StepTimer const& aTimer);
    void ResetIsCameraAtDestination() { m_isCameraAtDestination = false; };
    void ReverseTransitionDirection();
    void Rotate(DirectX::SimpleMath::Vector3 aAxis, float aDegrees); //Pavel
    void RotateAtSpeed(float aDx, float aDy);  //Chili
    void RotateCounterClockWise();
    void RotateClockWise();
    
    void SetCameraState(const CameraState aCameraState);
    void SetDestinationPos(const DirectX::SimpleMath::Vector3 aDestPos);
    void SetHomePos(const DirectX::SimpleMath::Vector3 aHomePos);
    void SetTargetPos(const DirectX::SimpleMath::Vector3 aTarget);
    void SetPos(const DirectX::SimpleMath::Vector3 aPos);
    void SetUpPos(const DirectX::SimpleMath::Vector3 aPos);

    void SetCameraEndPos(DirectX::SimpleMath::Vector3 aEndPos);   
    void SetCameraStartPos(DirectX::SimpleMath::Vector3 aStartPos);
    void SetTargetEndPos(DirectX::SimpleMath::Vector3 aEndPos);
    void SetTargetStartPos(DirectX::SimpleMath::Vector3 aStartPos);

    void TranslateAtSpeed(DirectX::SimpleMath::Vector3 aTranslation); //Chili

    void UpdateCamera(DX::StepTimer const& aTimer);
    void UpdateFirstPersonCamera();
    void UpdatePitchYaw(const float aPitch, const float aYaw);
    void UpdatePos(const float aX, const float aY, const float aZ);
    void UpdateTransitionCamera(DX::StepTimer const& aTimer);
    
    void UpdateTimer(DX::StepTimer const& aTimer) { m_cameraTimer = aTimer; };
    DX::StepTimer GetCameraTimer() { return m_cameraTimer; };

    void YawSpin(float aTurn);
    void TurnAroundPoint(float aTurn, DirectX::SimpleMath::Vector3 aCenterPoint);
    void TurnEndPosAroundPoint(float aTurn, DirectX::SimpleMath::Vector3 aCenterPoint);

private:
    void InitializeOrthoganalMatrix(); //Pavel
    void InitializeProjectionMatrix(); //Pavel
    void InitializeViewMatrix(); //Pavel
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

    const float                     m_posTravelSpeed = 0.6f;
    const float                     m_rotationTravelSpeed = 1.3f;
    const float                     m_aimTurnRate = 0.3f;

    bool                            m_isCameraAtDestination;

    CameraState                     m_cameraState;
    
    DirectX::SimpleMath::Matrix     m_rotationMatrix;

    DirectX::SimpleMath::Vector4    m_defaultForward = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::SimpleMath::Vector4    m_forward        = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::SimpleMath::Vector4    m_defaultRight   = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DirectX::SimpleMath::Vector4    m_right          = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    float                           m_moveBackForward = 0.0f;
    float                           m_moveLeftRight = 0.0f;
    float                           m_moveUpDown = 0.0f;

    const DirectX::SimpleMath::Vector3 m_preSwingCamPosOffset = DirectX::SimpleMath::Vector3(-0.25f, 0.07f, 0.0f);
    const DirectX::SimpleMath::Vector3 m_preSwingTargetPosOffset = DirectX::SimpleMath::Vector3(0.5f, 0.11f, 0.0f);
    const DirectX::SimpleMath::Vector3 m_swingCamPosOffset = DirectX::SimpleMath::Vector3(0.0f, 0.02f, 0.2f);
    const DirectX::SimpleMath::Vector3 m_swingTargetPosOffset = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    DirectX::SimpleMath::Vector3    m_cameraStartPos;
    DirectX::SimpleMath::Vector3    m_cameraEndPos;
    DirectX::SimpleMath::Vector3    m_targetStartPos;
    DirectX::SimpleMath::Vector3    m_targetEndPos;

    const float                     m_cameraTransitionSpeed = .9f;
    DX::StepTimer                   m_cameraTimer;

    bool                            m_isFpYaxisInverted = true; // toggle of turning on/off inverting the firstperson camera y axis control, set to true because I am weirdo that likes an inverted y axis
};

