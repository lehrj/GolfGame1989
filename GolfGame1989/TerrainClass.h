////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <fstream>
//#include <d3d11.h>
//#include <DirectXMath.h>
#include <stdio.h>
//using namespace DirectX;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "terraincellclass.h"
#include "frustumclass.h"


typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

#pragma pack(2) 
typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, * LPBITMAPFILEHEADER, * PBITMAPFILEHEADER;
#pragma pack() 

////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture; ////////
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 color;
		//DirectX::VertexPositionNormalColorTexture test;
	};

	struct HeightMapType
	{
		float x, y, z;
		float tu, tv; /////
		float nx, ny, nz;
		float r, g, b;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
		float r, g, b;
	};
	struct VectorType
	{
		float x, y, z;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device*, char*);
	void Shutdown();
	void Frame();

	//bool RenderCell(ID3D11DeviceContext*, int, FrustumClass*);
	bool RenderCell(ID3D11DeviceContext*, int);
	void RenderCellLines(ID3D11DeviceContext*, int);

	int GetCellIndexCount(int);
	int GetCellLinesIndexCount(int);
	int GetCellCount();
	int GetRenderCount();
	int GetCellsDrawn();
	int GetCellsCulled();

	bool GetHeightAtPosition(float, float, float&);

private:
	bool LoadSetupFile(char*);
	bool LoadRawHeightMap();
	bool LoadHeightMap(char* filename); // sorry ODB, we are taking a pass on liking it raw for now, Bitmaps will work better for what we need
	void ShutdownHeightMap();
	void SetTerrainCoordinates();
	bool CalculateNormals();
	bool LoadColorMap();
	bool BuildTerrainModel();
	void ShutdownTerrainModel();
	void CalculateTerrainVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);
	bool LoadTerrainCells(ID3D11Device*);
	void ShutdownTerrainCells();
	bool CheckHeightOfTriangle(float, float, float&, float[3], float[3], float[3]);

private:
	int m_terrainHeight, m_terrainWidth, m_vertexCount;
	float m_heightScale;
	char* m_terrainFilename, * m_colorMapFilename;
	HeightMapType* m_heightMap;
	ModelType* m_terrainModel;
	TerrainCellClass* m_TerrainCells;
	int m_cellCount, m_renderCount, m_cellsDrawn, m_cellsCulled;
};


#endif