#include "GCamera.h"
#include "pch.h"
//using namespace Game
//{
void GCamera::initViewMatrix()
{
	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mTarget),
		XMLoadFloat3(&this->Up())));
}

/*
GCamera::GCamera(void)
{
    mPosition		= XMFLOAT3(0.0f, 0.0f, -1.0f);
    mTarget			= XMFLOAT3(0.0f, 0.0f, 0.0f);
    mUp				= GMathVF(GMathFV(mPosition) + GMathFV(XMFLOAT3(0, 1, 0)));
	this->initViewMatrix();

	mAngle			= 0.0f;
	mClientWidth	= 0.0f;
	mClientHeight	= 0.0f;
	mNearest		= 0.0f;
	mFarthest		= 0.0f;

	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mProj, XMMatrixIdentity());
	XMStoreFloat4x4(&mOrtho, XMMatrixIdentity());
}

GCamera::GCamera(const GCamera& camera)
{
	*this = camera;
}

GCamera& GCamera::operator=(const GCamera& camera)
{
    mPosition		= camera.mPosition;
    mTarget			= camera.mTarget;
    mUp				= camera.mUp;

	mAngle			= camera.mAngle;
	mClientWidth	= camera.mClientWidth;
	mClientHeight	= camera.mClientHeight;
	mNearest		= camera.mNearest;
	mFarthest		= camera.mFarthest;

    mView			= camera.mView;
	mProj			= camera.mProj;
	mOrtho			= camera.mOrtho;
    return *this;
}

void GCamera::initViewMatrix()
{
	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mTarget), 
		XMLoadFloat3(&this->Up())));
}

void GCamera::InitProjMatrix(const float angle, const float client_width, const float client_height, 
								const float near_plane, const float far_plane)
{
	mAngle = angle;
	mClientWidth = client_width;
	mClientHeight = client_height;
	mNearest = near_plane;
	mFarthest = far_plane;
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(angle, client_width/client_height, 
		near_plane, far_plane));
}

void GCamera::Move(XMFLOAT3 direction)
{
	mPosition = GMathVF(XMVector3Transform(GMathFV(mPosition), 
		XMMatrixTranslation(direction.x, direction.y, direction.z)));
	mTarget = GMathVF(XMVector3Transform(GMathFV(mTarget), 
		XMMatrixTranslation(direction.x, direction.y, direction.z)));
	mUp = GMathVF(XMVector3Transform(GMathFV(mUp), 
		XMMatrixTranslation(direction.x, direction.y, direction.z)));

	this->initViewMatrix();
}

void GCamera::Rotate(XMFLOAT3 axis, float degrees)
{
	if (XMVector3Equal(GMathFV(axis), XMVectorZero()) ||
		degrees == 0.0f)
		return;

	// rotate vectors
	XMFLOAT3 look_at_target = GMathVF(GMathFV(mTarget) - GMathFV(mPosition));
	XMFLOAT3 look_at_up = GMathVF(GMathFV(mUp) - GMathFV(mPosition));
	look_at_target = GMathVF(XMVector3Transform(GMathFV(look_at_target), 
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(degrees))));
	look_at_up = GMathVF(XMVector3Transform(GMathFV(look_at_up), 
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(degrees))));

	// restore vectors's end points mTarget and mUp from new rotated vectors
	mTarget = GMathVF(GMathFV(mPosition) + GMathFV(look_at_target));
	mUp = GMathVF(GMathFV(mPosition) + GMathFV(look_at_up));

	this->initViewMatrix();
}

void GCamera::Target(XMFLOAT3 new_target)
{
	if (XMVector3Equal(GMathFV(new_target), GMathFV(mPosition)) ||
		XMVector3Equal(GMathFV(new_target), GMathFV(mTarget)))
		return;

	XMFLOAT3 old_look_at_target = GMathVF(GMathFV(mTarget) - GMathFV(mPosition));	
	XMFLOAT3 new_look_at_target = GMathVF(GMathFV(new_target) - GMathFV(mPosition));
	float angle = XMConvertToDegrees(XMVectorGetX(
		XMVector3AngleBetweenNormals(XMVector3Normalize(GMathFV(old_look_at_target)), 
		XMVector3Normalize(GMathFV(new_look_at_target)))));
	if (angle != 0.0f && angle != 360.0f && angle != 180.0f)
	{
		XMVECTOR axis = XMVector3Cross(GMathFV(old_look_at_target), GMathFV(new_look_at_target));
		Rotate(GMathVF(axis), angle);
	}
	mTarget = new_target;
	this->initViewMatrix();
}

// Set camera position
void GCamera::Position(XMFLOAT3& new_position)
{
	XMFLOAT3 move_vector = GMathVF(GMathFV(new_position) - GMathFV(mPosition));
	XMFLOAT3 target = mTarget;
	this->Move(move_vector);
	this->Target(target);
}

void GCamera::Angle(float angle)
{
	mAngle = angle;
	InitProjMatrix(mAngle, mClientWidth, mClientHeight, mNearest, mFarthest);
}

void GCamera::NearestPlane(float nearest)
{
	mNearest = nearest;
	OnResize(mClientWidth, mClientHeight);
}

void GCamera::FarthestPlane(float farthest)
{
	mFarthest = farthest;
	OnResize(mClientWidth, mClientHeight);
}

void GCamera::InitOrthoMatrix(const float clientWidth, const float clientHeight,
		const float nearZ, const float fartherZ)
{
	XMStoreFloat4x4(&mOrtho, XMMatrixOrthographicLH(clientWidth, clientHeight, 0.0f, fartherZ));
}

void GCamera::OnResize(uint32_t new_width, uint32_t new_height)
{
	mClientWidth = new_width;
	mClientHeight = new_height;
	InitProjMatrix(mAngle, static_cast<float>(new_width), static_cast<float>(new_height), mNearest, mFarthest);
	InitOrthoMatrix(static_cast<float>(new_width), static_cast<float>(new_height), 0.0f, mFarthest);
}

*/
//}