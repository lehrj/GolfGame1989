#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	m_homePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
	m_target = DirectX::SimpleMath::Vector3::Zero;
	//m_up = m_homePosition + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
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
	//m_homePosition = DirectX::SimpleMath::Vector3::Zero;
	m_target = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	//m_up = m_homePosition + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	m_up = DirectX::SimpleMath::Vector3::UnitY;
	m_position = DirectX::SimpleMath::Vector3(-2.0f, 0.2f, 0.0f);
	m_homePitch = 0.0f;
	m_homeYaw = 0.0f;
	//m_homeYaw = Utility::ToRadians(-90.0);

	m_nearPlane = 0.1f;
	m_farPlane = 10.0f;

	//m_cameraState = CameraState::CAMERASTATE_DEFAULT;
	//m_cameraState = CameraState::CAMERASTATE_FIRSTPERSON;
	m_cameraState = CameraState::CAMERASTATE_TRANSITION,
	Reset();
	InitializeViewMatrix();
	InitializeProjectionMatrix();
	InitializeOrthoganalMatrix();
}

/*
Camera::Camera(DirectX::XMFLOAT3 aHomePos, float aHomePitch, float aHomeYaw) noexcept
	:
	m_homePosition(aHomePos),
	m_homePitch(aHomePitch),
	m_homeYaw(aHomeYaw)
{
	Reset();
}
*/

void Camera::InitializeOrthoganalMatrix()
{
	m_orthogonalMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic((float)m_clientWidth, (float)m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::InitializeProjectionMatrix()
{
	//m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, m_clientWidth / m_clientHeight, 0.1f, 10.f);
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, m_clientWidth / m_clientHeight, m_nearPlane, m_farPlane);
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
	//m_up = m_position + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	m_up = DirectX::SimpleMath::Vector3::UnitY;   //(0.0f, 1.0f, 0.0f);
	m_pitch = m_homePitch;
	m_yaw = m_homeYaw;
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
	lookAtTarget = DirectX::SimpleMath::Vector3::Transform(lookAtTarget, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(aAxis, Utility::ToRadians(aDegrees)));
	lookAtUp = DirectX::SimpleMath::Vector3::Transform(lookAtUp, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(aAxis, Utility::ToRadians(aDegrees)));

	m_target = DirectX::SimpleMath::Vector3(m_position + lookAtTarget);
	m_up = DirectX::SimpleMath::Vector3(m_position + lookAtUp);

	this->InitializeViewMatrix();
}

void Camera::RotateAtSpeed(float aDx, float aDy)
{
	m_yaw = Utility::WrapAngle(m_yaw + aDx * m_rotationTravelSpeed);
	float rotation = m_pitch + aDy * m_rotationTravelSpeed;
	float min = 0.995f * -Utility::GetPi() / 2.0f;
	float max = 0.995f * Utility::GetPi() / 2.0f;
	float pi = Utility::GetPi();
	//m_pitch = std::clamp(m_pitch + aDy * m_rotationTravelSpeed, 0.995f * -Utility::GetPi() / 2.0f, 0.995f * Utility::GetPi() / 2.0f);
	//m_pitch = std::clamp(m_pitch + aDy * m_rotationTravelSpeed, 0.995f * -pi / 2.0f, 0.995f * pi / 2.0f);
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
	UpdateUp();
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
	UpdateUp();
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
	//this->InitializeViewMatrix();
}

void Camera::SetUpPos(const DirectX::SimpleMath::Vector3 aPos)
{
	m_up = aPos;
}

void Camera::UpdateTransitionCamera(DX::StepTimer const& aTimer)
{
	DirectX::SimpleMath::Vector3 testVec = m_target;
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

	//double elapsedTime = double(m_cameraTimer.GetElapsedSeconds());
	double elapsedTime = double(aTimer.GetElapsedSeconds());
	float cameraSpeed = m_cameraTransitionSpeed;
	float targetSpeed = cameraSpeed * (targetDistance / cameraDistance);

	m_position += cameraDirection * cameraSpeed * elapsedTime;
	m_target += targetDirection * targetSpeed * elapsedTime;

	m_up = DirectX::SimpleMath::Vector3::UnitY;

	if (DirectX::SimpleMath::Vector3::Distance(cameraStartPos, m_position) >= cameraDistance)
	{
		m_position = cameraEndPos;
		//m_target = targetEndPos;

		m_isCameraAtDestination = true;
		//m_cameraState = CameraState::CAMERASTATE_DEFAULT; // WLJ switch to non-trasition camera
		//m_cameraState = CameraState::CAMERASTATE_CAMERA2;
		ReverseTransitionDirection();
	}
	else
	{
		//m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_target, DirectX::SimpleMath::Vector3::UnitY);
		//m_world = DirectX::SimpleMath::Matrix::Identity;
		//m_effect->SetView(m_view);
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
	if (m_cameraState == CameraState::CAMERASTATE_TRANSITION)
	{
		UpdateTransitionCamera(aTimer);
	}
	if (m_cameraState == CameraState::CAMERASTATE_DEFAULT)
	{
		//m_position = DirectX::SimpleMath::Vector3(-2.f, .2f, 0.0);
		//m_target = DirectX::SimpleMath::Vector3::Zero;
	}
	if (m_cameraState == CameraState::CAMERASTATE_CAMERA1)
	{
		UpdateFirstPersonCamera();
	}
	if (m_cameraState == CameraState::CAMERASTATE_CAMERA2)
	{

	}
	if (m_cameraState == CameraState::CAMERASTATE_SWINGVIEW)
	{

	}
	if (m_cameraState == CameraState::CAMERASTATE_PROJECTILEFLIGHTVIEW)
	{

	}

	m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_target, m_up);
	//m_viewMatrix = DirectX::XMMatrixLookAtLH(m_position, m_target, m_up);
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

	//m_up = m_position;
	//m_up.y += 1.f;

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

void Camera::UpdateUp()
{
	m_up = m_position + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
}

void Camera::UpdateTrackCamera()
{

}

void Camera::UpdateViewMatrix()
{
	DirectX::SimpleMath::Vector3 axis;
	axis.Zero;
	//axis.x = 1.0f;
	axis.y = 1.0f;
	
	//DirectX::SimpleMath::Vector3 newPosition;
	//newPosition.Zero;

	//DirectX::SimpleMath::Vector3 newPosition = DirectX::SimpleMath::Vector3::Transform(m_position, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(axis, m_yaw));
	DirectX::SimpleMath::Vector3 newPosition = DirectX::SimpleMath::Vector3::Transform(m_position, DirectX::SimpleMath::Matrix::CreateRotationY(m_yaw));

	//DirectX::SimpleMath::Vector3 beta = DirectX::SimpleMath::Vector3::Transform(theta, DirectX::SimpleMath::Matrix::CreateRotationZ(-angles[i].y));
	//DirectX::SimpleMath::Matrix::CreateFromAxisAngle(aAxis, Utility::ToRadians(aDegrees))
	//m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_target, m_up);
	m_viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(newPosition, m_target, m_up);
}
