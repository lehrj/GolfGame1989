#pragma once

//#include <d3d11.h>
//#include <directxmath.h>
//using namespace DirectX;


class TerrainCellClass
{
private:
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
		float r, g, b;
	};

	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
		DirectX::XMFLOAT3 color;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct ColorVertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

public:
	TerrainCellClass();
	TerrainCellClass(const TerrainCellClass&);
	~TerrainCellClass();

	bool Initialize(ID3D11Device*, void*, int, int, int, int, int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	void RenderLineBuffers(ID3D11DeviceContext*);

	int GetVertexCount();
	int GetIndexCount();
	int GetLineBuffersIndexCount();
	void GetCellDimensions(float&, float&, float&, float&, float&, float&);

private:
	bool InitializeBuffers(ID3D11Device*, int, int, int, int, int, ModelType*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	void CalculateCellDimensions();
	bool BuildLineBuffers(ID3D11Device*);
	void ShutdownLineBuffers();

public:
	VectorType* m_vertexList;

private:
	int m_vertexCount, m_indexCount, m_lineIndexCount;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer, * m_lineVertexBuffer, * m_lineIndexBuffer;;
	float m_maxWidth, m_maxHeight, m_maxDepth, m_minWidth, m_minHeight, m_minDepth;
	float m_positionX, m_positionY, m_positionZ;
};

