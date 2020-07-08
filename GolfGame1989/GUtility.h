#pragma once
#include <windows.h>

#include <d3d11.h>
#include <directxmath.h>
#include <iostream>

using namespace DirectX;

inline XMVECTOR GMathFV(XMFLOAT3& val)
{
	return XMLoadFloat3(&val);	
}

inline XMFLOAT3 GMathVF(XMVECTOR& vec)
{
	XMFLOAT3 val;
	XMStoreFloat3(&val, vec);
	return val;
}

inline XMMATRIX GMathFM(XMFLOAT4X4& val)
{
	return XMLoadFloat4x4(&val);
}

inline XMFLOAT4X4 GMathMF(XMMATRIX& matrix)
{
	XMFLOAT4X4 val;
	XMStoreFloat4x4(&val, matrix);
	return val;
}