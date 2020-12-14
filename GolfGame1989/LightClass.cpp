#include "pch.h"
#include "LightClass.h"


LightClass::LightClass()
{
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = DirectX::XMFLOAT4(red, green, blue, alpha);
	return;
}


void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
	return;
}


void LightClass::SetDirection(float x, float y, float z)
{
	m_direction = DirectX::XMFLOAT3(x, y, z);
	return;
}


void LightClass::SetPosition(float x, float y, float z)
{
	m_position = DirectX::XMFLOAT3(x, y, z);
	return;
}


DirectX::XMFLOAT4 LightClass::GetAmbientColor()
{
	return m_ambientColor;
}


DirectX::XMFLOAT4 LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}


DirectX::XMFLOAT3 LightClass::GetDirection()
{
	return m_direction;
}


DirectX::XMFLOAT3 LightClass::GetPosition()
{
	return m_position;
}