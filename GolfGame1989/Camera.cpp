#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	m_homePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
	m_target = DirectX::SimpleMath::Vector3::Zero;
	m_up = DirectX::SimpleMath::Vector3::UnitY;
	m_homePitch = 0.0f;
	m_homeYaw = 0.0f;
	Reset();
}

Camera::Camera(int aWidth, int aHeight)
{
	m_clientWidth = (int)aWidth;
	m_clientHeight = aHeight;
	m_homePosition = DirectX::SimpleMath::Vector3(0.0f, 0.4f, 0.0f);
	m_target = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	m_up = DirectX::SimpleMath::Vector3::UnitY;
	m_position = DirectX::SimpleMath::Vector3(-2.0f, 0.2f, 0.0f);
	m_homePitch = 0.0f;
	m_homeYaw = 0.0f;

	m_nearPlane = 0.1f;
	m_farPlane = 10.0f;

	m_cameraState = CameraState::CAMERASTATE_PRESWINGVIEW;
	Reset();
	InitializeViewMatrix();
	InitializeProjectionMatrix();
	InitializeOrthoganalMatrix();
}

DirectX::SimpleMath::Vector3 Camera::GetPreSwingCamPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees)
{
	DirectX::SimpleMath::Vector3 newCamPosition = DirectX::SimpleMath::Vector3::Transform(m_preSwingCamPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;
	return newCamPosition;
}

DirectX::SimpleMath::Vector3 Camera::GetPreSwingTargPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees)
{
	DirectX::SimpleMath::Vector3 newTargetPosition = DirectX::SimpleMath::Vector3::Transform(m_preSwingTargetPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;
	return newTargetPosition;
}

DirectX::SimpleMath::Vector3 Camera::GetSwingCamPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees)
{
	DirectX::SimpleMath::Vector3 newCamPosition = DirectX::SimpleMath::Vector3::Transform(m_swingCamPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;
	return newCamPosition;
}

DirectX::SimpleMath::Vector3 Camera::GetSwingTargPos(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees)
{
	DirectX::SimpleMath::Vector3 newTargetPosition = DirectX::SimpleMath::Vector3::Transform(m_swingTargetPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;
	return newTargetPosition;
}

void Camera::InitializeOrthoganalMatrix()
{
	m_orthogonalMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic((float)m_clientWidth, (float)m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::InitializeProjectionMatrix()
{
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight), m_nearPlane, m_farPlane);
}

void Camera::InintializePreSwingCamera(DirectX::SimpleMath::Vector3 aPosition, float aDirectionDegrees)
{	
	DirectX::SimpleMath::Vector3 newCamPosition = DirectX::SimpleMath::Vector3::Transform(m_preSwingCamPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;

	DirectX::SimpleMath::Vector3 newTargetPosition = DirectX::SimpleMath::Vector3::Transform(m_preSwingTargetPosOffset,
		DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aDirectionDegrees))) + aPosition;

	SetTargetPos(newTargetPosition);
	SetPos(newCamPosition);
}

void Camera::InitializeViewMatrix()
{
	m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_target, m_up);
}

bool Camera::IsCameraAtDestination()
{
	if (m_position == m_destinationPosition || m_isCameraAtDestination == true)
	{
		return true;
	}
	else
	{
		if (m_position == m_destinationPosition)
		{
			m_isCameraAtDestination = true;
			return true;
		}
		else
		{
			return false;
		}
	}
}

void Camera::OnResize(uint32_t aWidth, uint32_t aHeight)
{
	m_clientWidth = aWidth;
	m_clientHeight = aHeight;
	UpdateOrthoganalMatrix();
	UpdateProjectionMatrix();
	UpdateViewMatrix();
}

void Camera::Reset()
{
	m_position = m_homePosition;
	m_up = DirectX::SimpleMath::Vector3::UnitY; 
	m_pitch = m_homePitch;
	m_yaw = m_homeYaw;
}

void Camera::ResetCameraTransition(DX::StepTimer const& aTimer)
{
	DirectX::SimpleMath::Vector3 cameraStartPos = m_cameraStartPos;

	DirectX::SimpleMath::Vector3 cameraEndPos = m_cameraEndPos;

	float cameraDistance = DirectX::SimpleMath::Vector3::Distance(cameraStartPos, cameraEndPos);
	DirectX::SimpleMath::Vector3 cameraDirection = cameraEndPos - cameraStartPos;

	cameraDirection.Normalize();

	DirectX::SimpleMath::Vector3 targetStartPos = m_targetStartPos;
	DirectX::SimpleMath::Vector3 targetEndPos = m_targetEndPos;

	float targetDistance = DirectX::SimpleMath::Vector3::Distance(targetStartPos, targetEndPos);
	DirectX::SimpleMath::Vector3 targetDirection = targetEndPos - targetStartPos;
	targetDirection.Normalize();

	double elapsedTime = double(aTimer.GetElapsedSeconds());
	float cameraSpeed = m_cameraTransitionSpeed;

	float targetSpeed;
	if (abs(cameraDistance > 0.0)) // prevent divide by zero if camera position doesn't change
	{
		targetSpeed = cameraSpeed * (targetDistance / cameraDistance);
	}
	else
	{
		targetSpeed = cameraSpeed;
	}

	m_position += cameraDirection * cameraSpeed * static_cast<float>(elapsedTime);

	if (targetDistance > 0.0f)
	{
		m_target += targetDirection * targetSpeed * static_cast<float>(elapsedTime);
	}

	m_up = DirectX::SimpleMath::Vector3::UnitY;

	if (DirectX::SimpleMath::Vector3::Distance(cameraStartPos, m_position) >= cameraDistance && DirectX::SimpleMath::Vector3::Distance(targetStartPos, m_target) >= targetDistance)
	{
		m_position = cameraEndPos;
		m_isCameraAtDestination = true;
	}
	else
	{

	}
}

void Camera::ReverseTransitionDirection()
{
	std::swap(m_cameraStartPos, m_cameraEndPos);
	std::swap(m_targetStartPos, m_targetEndPos);
}

void Camera::Rotate(DirectX::SimpleMath::Vector3 aAxis, float aDegrees)
{
	if (aAxis == DirectX::SimpleMath::Vector3::Zero || aDegrees == 0.0f)
	{
		return;
	}

	DirectX::SimpleMath::Vector3 lookAtTarget = m_target - m_position;
	DirectX::SimpleMath::Vector3 lookAtUp = m_up - m_position;
	lookAtTarget = DirectX::SimpleMath::Vector3::Transform(lookAtTarget, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(aAxis, static_cast<float>(Utility::ToRadians(aDegrees))));
	lookAtUp = DirectX::SimpleMath::Vector3::Transform(lookAtUp, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(aAxis, static_cast<float>(Utility::ToRadians(aDegrees))));

	m_target = DirectX::SimpleMath::Vector3(m_position + lookAtTarget);
	m_up = DirectX::SimpleMath::Vector3(m_position + lookAtUp);

	this->InitializeViewMatrix();
}

void Camera::RotateAtSpeed(float aDx, float aDy)
{
	m_yaw = Utility::WrapAngle(m_yaw + aDx * m_rotationTravelSpeed);
	float rotation = m_pitch + aDy * m_rotationTravelSpeed;
	float min = 0.995f * static_cast<float>(-Utility::GetPi()) / 2.0f;
	float max = 0.995f * static_cast<float>(Utility::GetPi()) / 2.0f;

	m_pitch = std::clamp(rotation, min, max);
}

void Camera::RotateCounterClockWise()
{
	float spinRate = 3.0f;
	m_yaw = Utility::WrapAngle(m_yaw + spinRate * m_rotationTravelSpeed);
	UpdateViewMatrix();
}

void Camera::RotateClockWise()
{
	float spinRate = 3.0f;
	m_yaw = Utility::WrapAngle(m_yaw - spinRate * m_rotationTravelSpeed);
	UpdateViewMatrix();
}

void Camera::SetCameraEndPos(DirectX::SimpleMath::Vector3 aEndPos)
{
	m_cameraEndPos = aEndPos;
}

void Camera::SetCameraStartPos(DirectX::SimpleMath::Vector3 aStartPos)
{
	m_cameraStartPos = aStartPos;
}

void Camera::SetCameraState(const CameraState aCameraState)
{
	m_cameraState = aCameraState;
}

void Camera::SetDestinationPos(const DirectX::SimpleMath::Vector3 aDestPos)
{
	if (aDestPos == m_target)
	{
		// add error handling to prevent crash
		//std::cerr << "Error in Camera::UpdatePosition, updated position = current target position";
		return;
	}
	m_destinationPosition = aDestPos;
}

void Camera::SetHomePos(DirectX::SimpleMath::Vector3 aHomePos)
{
	if (aHomePos == m_target)
	{
		// add error handling to prevent crash
		//std::cerr << "Error in Camera::UpdatePosition, updated position = current target position";
		return;
	}
	m_position = aHomePos;
}

void Camera::SetPos(DirectX::SimpleMath::Vector3 aPos)
{
	if (aPos == m_target)
	{
		// add error handling to prevent crash
		//std::cerr << "Error in Camera::UpdatePosition, updated position = current target position";
		return;
	}
	m_position = aPos;
}

void Camera::SetTargetEndPos(DirectX::SimpleMath::Vector3 aEndPos)
{
	m_targetEndPos = aEndPos;
}

void Camera::SetTargetStartPos(DirectX::SimpleMath::Vector3 aStartPos)
{
	m_targetStartPos = aStartPos;
}

void Camera::SetTargetPos(const DirectX::SimpleMath::Vector3 aTarget)
{
	if (aTarget == m_position)
	{
		// add error handling to prevent crash
		//std::cerr << "Error in Camera::SetTargetPos, updated target position = current camera position";
		return;
	}
	m_target = aTarget;
}

void Camera::SetUpPos(const DirectX::SimpleMath::Vector3 aPos)
{
	m_up = aPos;
}

void Camera::UpdateTransitionCamera(DX::StepTimer const& aTimer)
{
	DirectX::SimpleMath::Vector3 cameraStartPos = m_cameraStartPos;

	DirectX::SimpleMath::Vector3 cameraEndPos = m_cameraEndPos;

	float cameraDistance = DirectX::SimpleMath::Vector3::Distance(cameraStartPos, cameraEndPos);
	DirectX::SimpleMath::Vector3 cameraDirection = cameraEndPos - cameraStartPos;

	cameraDirection.Normalize();

	DirectX::SimpleMath::Vector3 targetStartPos = m_targetStartPos;
	DirectX::SimpleMath::Vector3 targetEndPos = m_targetEndPos;

	float targetDistance = DirectX::SimpleMath::Vector3::Distance(targetStartPos, targetEndPos);
	DirectX::SimpleMath::Vector3 targetDirection = targetEndPos - targetStartPos;
	targetDirection.Normalize();

	double elapsedTime = double(aTimer.GetElapsedSeconds());
	float cameraSpeed = m_cameraTransitionSpeed;

	float targetSpeed;
	if (abs(cameraDistance > 0.0)) // prevent divide by zero if camera position doesn't change
	{
		targetSpeed = cameraSpeed * (targetDistance / cameraDistance);
	}
	else
	{
		targetSpeed = cameraSpeed;
	}

	m_position += cameraDirection * cameraSpeed * static_cast<float>(elapsedTime);

	if (targetDistance > 0.0f)
	{
		m_target += targetDirection * targetSpeed * static_cast<float>(elapsedTime);
	}

	m_up = DirectX::SimpleMath::Vector3::UnitY;

	if (DirectX::SimpleMath::Vector3::Distance(cameraStartPos, m_position) >= cameraDistance && DirectX::SimpleMath::Vector3::Distance(targetStartPos, m_target) >= targetDistance)
	{
		m_position = cameraEndPos;
		m_isCameraAtDestination = true;
	}
	else
	{

	}
}

void Camera::TranslateAtSpeed(DirectX::SimpleMath::Vector3 aTranslation)
{
	DirectX::XMStoreFloat3(&aTranslation, DirectX::XMVector3Transform(
		DirectX::XMLoadFloat3(&aTranslation),
		DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f) *
		DirectX::XMMatrixScaling(m_posTravelSpeed, m_posTravelSpeed, m_posTravelSpeed)
	));
	m_position = { m_position.x + aTranslation.x, m_position.y + aTranslation.y, m_position.z + aTranslation.z };
}

void Camera::UpdateCamera(DX::StepTimer const& aTimer)
{
	UpdateTimer(aTimer);
	if (m_cameraState == CameraState::CAMERASTATE_FIRSTPERSON)
	{
		UpdateFirstPersonCamera();
	}
	if (m_cameraState == CameraState::CAMERASTATE_SWINGVIEW)
	{
		//m_target = DirectX::SimpleMath::Vector3(-2.f, 0.0, 0.0);
		//m_position = DirectX::SimpleMath::Vector3(-2.f, 0.02f, .2f);
	}
	if (m_cameraState == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
	{
		// no update needed in current state
	}
	if (m_cameraState == CameraState::CAMERASTATE_PRESWINGVIEW)
	{
		//m_position = DirectX::SimpleMath::Vector3(-2.9f, .5f, 0.0f);
		//m_target = DirectX::SimpleMath::Vector3(-2.f, 0.3f, 0.0f);
		//m_up = DirectX::SimpleMath::Vector3::UnitY;
	}
	if (m_cameraState == CameraState::CAMERASTATE_TRANSITION)
	{
		if (IsCameraAtDestination() == false)
		{
			UpdateTransitionCamera(aTimer);
		}
		else
		{
			m_cameraState = CameraState::CAMERASTATE_SWINGVIEW;
			m_isCameraAtDestination = false;
		}
	}
	if (m_cameraState == CameraState::CAMERASTATE_RESET)
	{
		if (IsCameraAtDestination() == false)
		{
			UpdateTransitionCamera(aTimer);
		}
		else
		{
			m_cameraState = CameraState::CAMERASTATE_PRESWINGVIEW;
			m_isCameraAtDestination = false;
		}
	}
	if (m_cameraState == CameraState::CAMERASTATE_TRANSTONEWSHOT)
	{
		if (IsCameraAtDestination() == false)
		{
			UpdateTransitionCamera(aTimer);
		}
		else
		{

			SetCameraStartPos(GetPos());     
			SetCameraEndPos(GetPreSwingCamPos(GetPos(), 0.0));
			SetTargetStartPos(GetTargetPos());
			SetTargetEndPos(GetPreSwingTargPos(GetPos(), 0.0));

			m_cameraState = CameraState::CAMERASTATE_PRESWINGVIEW;
			m_isCameraAtDestination = false;
		}
	}

	m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_target, m_up);
}

void Camera::UpdateFirstPersonCamera()
{
	m_rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
	m_target = DirectX::XMVector3TransformCoord(m_defaultForward, m_rotationMatrix);
	m_target.Normalize();

	DirectX::XMMATRIX rotateYTempMatrix;
	rotateYTempMatrix = DirectX::XMMatrixRotationY(m_yaw);

	m_right = DirectX::XMVector3TransformCoord(m_defaultRight, rotateYTempMatrix);

	m_forward = DirectX::XMVector3TransformCoord(m_defaultForward, rotateYTempMatrix);
	m_up = DirectX::XMVector3TransformCoord(m_up, rotateYTempMatrix);

	m_position += DirectX::operator*(m_moveLeftRight, m_right);
	m_position += DirectX::operator*(m_moveBackForward, m_forward);
	m_position += DirectX::operator*(m_moveUpDown, m_up);

	m_moveLeftRight = 0.0f;
	m_moveBackForward = 0.0f;
	m_moveUpDown = 0.0f;

	m_target = m_position + m_target;
	//m_viewMatrix = DirectX::XMMatrixLookAtLH(m_position, m_target, m_up);
}

void Camera::UpdatePitchYaw(const float aPitch, const float aYaw)
{
	m_pitch += aPitch * m_rotationTravelSpeed;
	m_yaw += aYaw * m_rotationTravelSpeed;
}

void Camera::UpdatePos(const float aX, const float aY, const float aZ)
{
	m_moveLeftRight += aX * m_posTravelSpeed;
	m_moveBackForward += aZ * m_posTravelSpeed;
	m_moveUpDown += aY * m_posTravelSpeed;
}

void Camera::UpdateOrthoganalMatrix()
{
	m_orthogonalMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic((float)m_clientWidth, (float)m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::UpdateProjectionMatrix()
{
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, (float)m_clientWidth / (float)m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::UpdateViewMatrix()
{
	DirectX::SimpleMath::Vector3 newPosition = DirectX::SimpleMath::Vector3::Transform(m_position, DirectX::SimpleMath::Matrix::CreateRotationY(m_yaw));
	m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(newPosition, m_target, m_up);
}
