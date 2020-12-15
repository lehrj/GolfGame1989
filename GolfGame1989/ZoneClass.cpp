#include "pch.h"
#include "ZoneClass.h"


ZoneClass::ZoneClass()
{
	//m_UserInterface = 0;
	//m_Camera = 0;
	m_Light = 0;
	//m_Position = 0;
	m_Frustum = 0;
	m_SkyDome = 0;
	m_Terrain = 0;
}


ZoneClass::ZoneClass(const ZoneClass& other)
{
}


ZoneClass::~ZoneClass()
{
}

//bool ZoneClass::Initialize(ID3D11Device1* Direct3D, HWND hwnd, int screenWidth, int screenHeight, float screenDepth)
bool ZoneClass::Initialize(ID3D11Device1* Direct3D, Camera* aCamera)
{
	bool result;
	m_Camera = aCamera;

	/*
	// Create the user interface object.
	m_UserInterface = new UserInterfaceClass;
	if (!m_UserInterface)
	{
		return false;
	}
	
	// Initialize the user interface object.
	result = m_UserInterface->Initialize(Direct3D, screenHeight, screenWidth);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the user interface object.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera and build the matrices needed for rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();
	m_Camera->RenderBaseViewMatrix();
	*/
	// Create the light object.
	m_Light = new LightClass;
	if (!m_Light)
	{
		return false;
	}

	// Initialize the light object.
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(-0.5f, -1.0f, -0.5f);


	/*
	// Create the position object.
	m_Position = new PositionClass;
	if (!m_Position)
	{
		return false;
	}

	// Set the initial position and rotation.
	m_Position->SetPosition(512.5f, 30.0f, 10.0f);
	m_Position->SetRotation(0.0f, 0.0f, 0.0f);

	// Create the frustum object.
	m_Frustum = new FrustumClass;
	if (!m_Frustum)
	{
		return false;
	}
	
	// Initialize the frustum object.
	//m_Frustum->Initialize(screenDepth);
	//m_Frustum->Initialize(100.f);
	*/
	
	// Create the sky dome object.
	m_SkyDome = new SkyDomeClass;
	if (!m_SkyDome)
	{
		return false;
	}

	// Initialize the sky dome object.
	//result = m_SkyDome->Initialize(Direct3D->GetDevice());
	result = m_SkyDome->Initialize(Direct3D);
	if (!result)
	{
		//MessageBox(hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
		return false;
	}
	

	// Create the terrain object.
	m_Terrain = new TerrainClass;
	if (!m_Terrain)
	{
		return false;
	}
	

	// Initialize the terrain object.
	//result = m_Terrain->Initialize(Direct3D->GetDevice(), "../Engine/data/setup.txt");
	result = m_Terrain->Initialize(Direct3D, "setup.txt");
	if (!result)
	{
		//MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}
	

	// Set the UI to display by default.
	//m_displayUI = true;

	// Set wire frame rendering initially to disabled.
	m_wireFrame = false;

	// Set the rendering of cell lines initially to disabled.
	m_cellLines = false;

	// Set the user locked to the terrain height for movement.
	m_heightLocked = true;

	return true;
}

void ZoneClass::Shutdown()
{
	// Release the terrain object.
	if (m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}


	// Release the sky dome object.
	
	if (m_SkyDome)
	{
		m_SkyDome->Shutdown();
		delete m_SkyDome;
		m_SkyDome = 0;
	}
	

	// Release the frustum object.
	if (m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = 0;
	}

	/*
	// Release the position object.
	if (m_Position)
	{
		delete m_Position;
		m_Position = 0;
	}
	*/

	// Release the light object.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}
	/*
	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the user interface object.
	if (m_UserInterface)
	{
		m_UserInterface->Shutdown();
		delete m_UserInterface;
		m_UserInterface = 0;
	}
	*/
	return;
}

//bool ZoneClass::Frame(ID3D11DeviceContext1* Direct3D, float frameTime, int fps)
//bool Frame(ID3D11DeviceContext1*, ShaderManagerClass*, TextureManagerClass*);
//bool ZoneClass::Frame(ID3D11DeviceContext1* Direct3D)

bool ZoneClass::Frame(ID3D11DeviceContext1* aDirect3D, ShaderManagerClass* aShaderManager, TextureManagerClass* aTextureManager)
{
	bool result, foundHeight;
	float posX, posY, posZ, rotX, rotY, rotZ, height;

	/*
	// Do the frame input processing.
	HandleMovementInput(Input, frameTime);

	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Do the frame processing for the user interface.
	result = m_UserInterface->Frame(Direct3D->GetDeviceContext(), fps, posX, posY, posZ, rotX, rotY, rotZ);
	if (!result)
	{
		return false;
	}
	*/

	// Do the terrain frame processing.
	m_Terrain->Frame();

	// If the height is locked to the terrain then position the camera on top of it.
	/*
	if (m_heightLocked)
	{
		// Get the height of the triangle that is directly underneath the given camera position.
		foundHeight = m_Terrain->GetHeightAtPosition(posX, posZ, height);
		if (foundHeight)
		{
			// If there was a triangle under the camera then position the camera just above it by one meter.
			m_Position->SetPosition(posX, height + 1.0f, posZ);
			m_Camera->SetPosition(posX, height + 1.0f, posZ);
		}
	}
	*/

	// Render the graphics.
	result = Render(aDirect3D, aShaderManager, aTextureManager);
	if (!result)
	{
		return false;
	}

	return true;
}

//void ZoneClass::HandleMovementInput(InputClass* Input, float frameTime)
/*
{
	bool keyDown;
	float posX, posY, posZ, rotX, rotY, rotZ;


	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(frameTime);

	// Handle the input.
	keyDown = Input->IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = Input->IsRightPressed();
	m_Position->TurnRight(keyDown);

	keyDown = Input->IsUpPressed();
	m_Position->MoveForward(keyDown);

	keyDown = Input->IsDownPressed();
	m_Position->MoveBackward(keyDown);

	keyDown = Input->IsAPressed();
	m_Position->MoveUpward(keyDown);

	keyDown = Input->IsZPressed();
	m_Position->MoveDownward(keyDown);

	keyDown = Input->IsPgUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = Input->IsPgDownPressed();
	m_Position->LookDownward(keyDown);

	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_Camera->SetPosition(posX, posY, posZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

	// Determine if the user interface should be displayed or not.
	if (Input->IsF1Toggled())
	{
		m_displayUI = !m_displayUI;
	}

	// Determine if the terrain should be rendered in wireframe or not.
	if (Input->IsF2Toggled())
	{
		m_wireFrame = !m_wireFrame;
	}

	// Determine if we should render the lines around each terrain cell.
	if (Input->IsF3Toggled())
	{
		m_cellLines = !m_cellLines;
	}

	// Determine if we should be locked to the terrain height when we move around or not.
	if (Input->IsF4Toggled())
	{
		m_heightLocked = !m_heightLocked;
	}

	return;
}
*/

//bool ZoneClass::Render(ID3D11DeviceContext1* Direct3D)
bool ZoneClass::Render(ID3D11DeviceContext1* aDirect3D, ShaderManagerClass* aShaderManager, TextureManagerClass* aTextureManager)
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix, baseViewMatrix, orthoMatrix;
	bool result;
	//DirectX::XMFLOAT3 cameraPosition;
	int i;

	worldMatrix = DirectX::XMMatrixIdentity();
	viewMatrix = m_Camera->GetViewMatrix();
	projectionMatrix = m_Camera->GetProjectionMatrix();


	// Render the sky dome using the sky dome shader.
	m_SkyDome->Render(aDirect3D);
	result = aShaderManager->RenderSkyDomeShader(aDirect3D, m_SkyDome->GetIndexCount(), worldMatrix, viewMatrix,
		projectionMatrix, m_SkyDome->GetApexColor(), m_SkyDome->GetCenterColor());
	if (!result)
	{
		return false;
	}

	// Construct the frustum.
	//m_Frustum->ConstructFrustum(projectionMatrix, viewMatrix);

	int cellCount = m_Terrain->GetCellCount();



	// Render the terrain cells (and cell lines if needed).
	for (i = 0; i < m_Terrain->GetCellCount(); i++)
	{
		
		// Render each terrain cell if it is visible only.
		//result = m_Terrain->RenderCell(Direct3D->GetDeviceContext(), i, m_Frustum);
		//result = m_Terrain->RenderCell(Direct3D, i, m_Frustum);

		result = m_Terrain->RenderCell(aDirect3D, i);

		if (result)
		{

			// Render the cell buffers using the terrain shader.
			/*
			result = ShaderManager->RenderTerrainShader(Direct3D->GetDeviceContext(), m_Terrain->GetCellIndexCount(i), worldMatrix, viewMatrix,
				projectionMatrix, TextureManager->GetTexture(0), TextureManager->GetTexture(1),
				m_Light->GetDirection(), m_Light->GetDiffuseColor());
			*/
			
			result = aShaderManager->RenderTerrainShader(aDirect3D, m_Terrain->GetCellIndexCount(i), worldMatrix, viewMatrix,
				projectionMatrix, aTextureManager->GetTexture(0), aTextureManager->GetTexture(1), m_Light->GetDirection(), m_Light->GetDiffuseColor());
			if (!result)
			{
				return false;
			}
			
			// If needed then render the bounding box around this terrain cell using the color shader. 
			
			if (m_cellLines)
			{
				m_Terrain->RenderCellLines(aDirect3D, i);
				aShaderManager->RenderColorShader(aDirect3D, m_Terrain->GetCellLinesIndexCount(i), worldMatrix,
					viewMatrix, projectionMatrix);
				if (!result)
				{
					return false;
				}
			}
			
			//m_Terrain->RenderCellLines(Direct3D, i);
		}
	}

	return true;
}
