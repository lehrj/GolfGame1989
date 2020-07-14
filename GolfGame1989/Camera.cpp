#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	m_homePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
	m_target.Zero;
	m_up = m_homePosition + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	m_homePitch = 0.0f;
	m_homeYaw = 0.0f;
	Reset();
}

Camera::Camera(float aWidth, float aHeight)
{
	m_clientWidth = aWidth;
	m_clientHeight = aHeight;
	m_homePosition = DirectX::SimpleMath::Vector3(-1.0f, 1.0f, 0.0f);
	m_target.Zero;
	m_up = m_homePosition + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	m_homePitch = 0.0f;
	m_homeYaw = 0.0f;

	m_nearPlane = 0.1f;
	m_farPlane = 10.0f;

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
	m_orthogonalMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic(m_clientWidth, m_clientHeight, m_nearPlane, m_farPlane);
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
	m_up = m_position + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	m_pitch = m_homePitch;
	m_yaw = m_homeYaw;
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

void Camera::SetTargetPos(const DirectX::SimpleMath::Vector3 aTarget)
{
	if (aTarget == m_position)
	{
		// add error handling to prevent crash
		//std::cerr << "Error in Camera::SetTargetPos, updated target position = current camera position";
		return;
	}
	m_target = aTarget;
	this->InitializeViewMatrix();
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

void Camera::UpdateOrthoganalMatrix()
{
	m_orthogonalMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic(m_clientWidth, m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::UpdateProjectionMatrix()
{
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI / 4.f, m_clientWidth / m_clientHeight, m_nearPlane, m_farPlane);
}

void Camera::UpdateUp()
{
	m_up = m_position + DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
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
