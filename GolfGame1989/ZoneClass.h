#pragma once

//#include "d3dclass.h"
//#include "inputclass.h"
#include "ShaderManagerClass.h"
#include "TextureManagerClass.h"
//#include "timerclass.h"
//#include "userinterfaceclass.h"
//#include "cameraclass.h"
#include "LightClass.h"
//#include "positionclass.h"
#include "FrustumClass.h"
#include "SkyDomeClass.h"
#include "TerrainClass.h"

class ZoneClass
{
public:
	ZoneClass();
	ZoneClass(const ZoneClass&);
	~ZoneClass();

	//bool Initialize(D3DClass*, HWND, int, int, float);
	bool Initialize(ID3D11Device1*);
	void Shutdown();
	//bool Frame(D3DClass*, InputClass*, ShaderManagerClass*, TextureManagerClass*, float, int);
	//bool Frame(ID3D11DeviceContext1*, float, int);
	bool Frame(ID3D11DeviceContext1*);

private:
	//void HandleMovementInput(InputClass*, float);
	//bool Render(D3DClass*, ShaderManagerClass*, TextureManagerClass*);
	bool RenderOld(ID3D11DeviceContext1*);
	bool Render(ID3D11DeviceContext1*);
private:
	//UserInterfaceClass* m_UserInterface;
	//CameraClass* m_Camera;
	LightClass* m_Light;
	//PositionClass* m_Position;
	FrustumClass* m_Frustum;
	SkyDomeClass* m_SkyDome;
	TerrainClass* m_Terrain;
	bool m_displayUI, m_wireFrame, m_cellLines, m_heightLocked;


};

