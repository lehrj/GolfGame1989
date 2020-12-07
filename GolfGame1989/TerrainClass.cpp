////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "TerrainClass.h"


TerrainClass::TerrainClass()
{
	m_terrainFilename = 0;
	m_colorMapFilename = 0;
	m_heightMap = 0;
	m_terrainModel = 0;
	m_TerrainCells = 0;
}

TerrainClass::TerrainClass(const TerrainClass& other)
{
}

TerrainClass::~TerrainClass()
{
}

bool TerrainClass::Initialize(ID3D11Device* device, char* setupFilename)
{
	bool result;

	// Get the terrain filename, dimensions, and so forth from the setup file.
	result = LoadSetupFile(setupFilename);
	if (!result)
	{
		return false;
	}

	
	// Initialize the terrain height map with the data from the raw file.
	result = LoadRawHeightMap();
	if (!result)
	{
		return false;
	}
	

	// Load in the height map for the terrain.
	/*
	result = LoadHeightMap(m_terrainFilename);
	if (!result)
	{
		return false;
	}
	*/

	// Setup the X and Z coordinates for the height map as well as scale the terrain height by the height scale value.
	SetTerrainCoordinates();

	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Load in the color map for the terrain.
	result = LoadColorMap();
	if (!result)
	{
		return false;
	}

	// Now build the 3D model of the terrain.
	result = BuildTerrainModel();
	if (!result)
	{
		return false;
	}

	// We can now release the height map since it is no longer needed in memory once the 3D terrain model has been built.
	ShutdownHeightMap();

	// Calculate the tangent and binormal for the terrain model.
	CalculateTerrainVectors();

	// Create and load the cells with the terrain data.
	result = LoadTerrainCells(device);
	if (!result)
	{
		return false;
	}

	// Release the terrain model now that the terrain cells have been loaded.
	ShutdownTerrainModel();

	return true;
}

void TerrainClass::Shutdown()
{
	// Release the terrain cells.
	ShutdownTerrainCells();

	// Release the terrain model.
	ShutdownTerrainModel();

	// Release the height map.
	ShutdownHeightMap();

	return;
}

void TerrainClass::Frame()
{
	m_renderCount = 0;
	m_cellsDrawn = 0;
	m_cellsCulled = 0;
	return;
}

bool TerrainClass::LoadSetupFile(char* filename)
{
	int stringLength;
	std::ifstream fin;
	char input;

	// Initialize the strings that will hold the terrain file name and the color map file name.
	stringLength = 256;

	m_terrainFilename = new char[stringLength];
	if (!m_terrainFilename)
	{
		return false;
	}

	m_colorMapFilename = new char[stringLength];
	if (!m_colorMapFilename)
	{
		return false;
	}

	// Open the setup file.  If it could not open the file then exit.
	fin.open(filename);
	if (fin.fail())
	{
		return false;
	}

	// Read up to the terrain file name.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain file name.
	fin >> m_terrainFilename;

	// Read up to the value of terrain height.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain height.
	fin >> m_terrainHeight;

	// Read up to the value of terrain width.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain width.
	fin >> m_terrainWidth;

	// Read up to the value of terrain height scaling.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain height scaling.
	fin >> m_heightScale;

	// Read up to the color map file name.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the color map file name.
	fin >> m_colorMapFilename;

	// Close the setup file.
	fin.close();

	return true;
}

bool TerrainClass::LoadHeightMap(char* filename)
{
	FILE* filePtr;
	int error;
	size_t count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_terrainWidth = bitmapInfoHeader.biWidth;
	m_terrainHeight = bitmapInfoHeader.biHeight;
	
	// Calculate the size of the bitmap image data.
	imageSize = m_terrainWidth * m_terrainHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the height map.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			k += 3;
		}
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

bool TerrainClass::LoadRawHeightMap()
{
	int error, i, j, index;
	FILE* filePtr;
	unsigned long long imageSize, count;
	unsigned short* rawImage;

	// Create the float array to hold the height map data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	// Open the 16 bit raw height map file for reading in binary.
	error = fopen_s(&filePtr, m_terrainFilename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Calculate the size of the raw image data.
	imageSize = m_terrainHeight * m_terrainWidth;

	// Allocate memory for the raw image data.
	rawImage = new unsigned short[imageSize];
	if (!rawImage)
	{
		return false;
	}

	// Read in the raw image data.
	count = fread(rawImage, sizeof(unsigned short), imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Copy the image data into the height map array.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainWidth * j) + i;

			// Store the height at this point in the height map array.
			m_heightMap[index].y = (float)rawImage[index];
		}
	}

	// Release the bitmap image data.
	delete[] rawImage;
	rawImage = 0;

	// Release the terrain filename now that it has been read in.
	delete[] m_terrainFilename;
	m_terrainFilename = 0;

	return true;
}

void TerrainClass::ShutdownHeightMap()
{
	// Release the height map array.
	if (m_heightMap)
	{
		delete[] m_heightMap;
		m_heightMap = 0;
	}

	return;
}

void TerrainClass::SetTerrainCoordinates()
{
	int i, j, index;

	std::vector<DirectX::XMFLOAT3> testTerrain;
	testTerrain.clear();

	// Loop through all the elements in the height map array and adjust their coordinates correctly.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainWidth * j) + i;

			// Set the X and Z coordinates.
			m_heightMap[index].x = (float)i;
			m_heightMap[index].z = -(float)j;

			// Move the terrain depth into the positive range.  For example from (0, -256) to (256, 0).
			m_heightMap[index].z += (float)(m_terrainHeight - 1);


			//m_heightMap[index].z *= -1; // testing fliping z axis


			// Scale the height.
			m_heightMap[index].y /= m_heightScale;

			DirectX::XMFLOAT3 pos;
			pos.x = m_heightMap[index].x;
			pos.y = m_heightMap[index].y;
			pos.z = m_heightMap[index].z;
			testTerrain.push_back(pos);
		}
	}

	int test = 0;
	return;
}

bool TerrainClass::CalculateNormals()
{
	int i, j, index1, index2, index3, index;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;

	// Create a temporary array to hold the face normal vectors.
	normals = new VectorType[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
	if (!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = ((j + 1) * m_terrainWidth) + i;      // Bottom left vertex.
			index2 = ((j + 1) * m_terrainWidth) + (i + 1);  // Bottom right vertex.
			index3 = (j * m_terrainWidth) + i;          // Upper left vertex.

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;

			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;

			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainWidth - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

			// Calculate the length.
			length = (float)sqrt((normals[index].x * normals[index].x) + (normals[index].y * normals[index].y) +
				(normals[index].z * normals[index].z));

			// Normalize the final value for this face using the length.
			normals[index].x = (normals[index].x / length);
			normals[index].y = (normals[index].y / length);
			normals[index].z = (normals[index].z / length);
		}
	}

	// Now go through all the vertices and take a sum of the face normals that touch this vertex.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainWidth - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainWidth - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainWidth - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Upper right face.
			if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainWidth - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Calculate the length of this normal.
			length = (float)sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainWidth) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

bool TerrainClass::LoadColorMap()
{
	int error, imageSize, i, j, k, index;
	FILE* filePtr;
	unsigned long long count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char* bitmapImage;

	// Open the color map file in binary.
	error = fopen_s(&filePtr, m_colorMapFilename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Make sure the color map dimensions are the same as the terrain dimensions for easy 1 to 1 mapping.
	if ((bitmapInfoHeader.biWidth != m_terrainWidth) || (bitmapInfoHeader.biHeight != m_terrainHeight))
	{
		return false;
	}

	// Calculate the size of the bitmap image data.
	// Since this is non-divide by 2 dimensions (eg. 257x257) need to add extra byte to each line.
	imageSize = m_terrainHeight * ((m_terrainWidth * 3) + 1);

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the color map portion of the height map structure.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Bitmaps are upside down so load bottom to top into the array.
			index = (m_terrainWidth * (m_terrainHeight - 1 - j)) + i;

			m_heightMap[index].b = (float)bitmapImage[k] / 255.0f;
			m_heightMap[index].g = (float)bitmapImage[k + 1] / 255.0f;
			m_heightMap[index].r = (float)bitmapImage[k + 2] / 255.0f;

			k += 3;
		}

		// Compensate for extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257).
		k++;
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

	// Release the color map filename now that is has been read in.
	delete[] m_colorMapFilename;
	m_colorMapFilename = 0;

	return true;
}

bool TerrainClass::BuildTerrainModel()
{
	int i, j, index, index1, index2, index3, index4;

	// Calculate the number of vertices in the 3D terrain model.
	m_vertexCount = (m_terrainHeight - 1) * (m_terrainWidth - 1) * 6;

	// Create the 3D terrain model array.
	m_terrainModel = new ModelType[m_vertexCount];
	if (!m_terrainModel)
	{
		return false;
	}

	// Initialize the index into the height map array.
	index = 0;

	std::vector<DirectX::XMFLOAT3> testTerrain;
	testTerrain.clear();

	// Load the 3D terrain model with the height map terrain data.
	// We will be creating 2 triangles for each of the four points in a quad.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			// Get the indexes to the four points of the quad.
			index1 = (m_terrainWidth * j) + i;          // Upper left.
			index2 = (m_terrainWidth * j) + (i + 1);      // Upper right.
			index3 = (m_terrainWidth * (j + 1)) + i;      // Bottom left.
			index4 = (m_terrainWidth * (j + 1)) + (i + 1);  // Bottom right.

			// Now create two triangles for that quad.
			// Triangle 1 - Upper left.
			m_terrainModel[index].x = m_heightMap[index1].x;
			m_terrainModel[index].y = m_heightMap[index1].y;
			m_terrainModel[index].z = m_heightMap[index1].z;
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 0.0f;
			m_terrainModel[index].nx = m_heightMap[index1].nx;
			m_terrainModel[index].ny = m_heightMap[index1].ny;
			m_terrainModel[index].nz = m_heightMap[index1].nz;
			m_terrainModel[index].r = m_heightMap[index1].r;
			m_terrainModel[index].g = m_heightMap[index1].g;
			m_terrainModel[index].b = m_heightMap[index1].b;

			DirectX::XMFLOAT3 pos;
			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;

			// Triangle 1 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 0.0f;
			m_terrainModel[index].nx = m_heightMap[index2].nx;
			m_terrainModel[index].ny = m_heightMap[index2].ny;
			m_terrainModel[index].nz = m_heightMap[index2].nz;
			m_terrainModel[index].r = m_heightMap[index2].r;
			m_terrainModel[index].g = m_heightMap[index2].g;
			m_terrainModel[index].b = m_heightMap[index2].b;

			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;

			// Triangle 1 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 1.0f;
			m_terrainModel[index].nx = m_heightMap[index3].nx;
			m_terrainModel[index].ny = m_heightMap[index3].ny;
			m_terrainModel[index].nz = m_heightMap[index3].nz;
			m_terrainModel[index].r = m_heightMap[index3].r;
			m_terrainModel[index].g = m_heightMap[index3].g;
			m_terrainModel[index].b = m_heightMap[index3].b;

			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;

			// Triangle 2 - Bottom left.
			m_terrainModel[index].x = m_heightMap[index3].x;
			m_terrainModel[index].y = m_heightMap[index3].y;
			m_terrainModel[index].z = m_heightMap[index3].z;
			m_terrainModel[index].tu = 0.0f;
			m_terrainModel[index].tv = 1.0f;
			m_terrainModel[index].nx = m_heightMap[index3].nx;
			m_terrainModel[index].ny = m_heightMap[index3].ny;
			m_terrainModel[index].nz = m_heightMap[index3].nz;
			m_terrainModel[index].r = m_heightMap[index3].r;
			m_terrainModel[index].g = m_heightMap[index3].g;
			m_terrainModel[index].b = m_heightMap[index3].b;

			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;

			// Triangle 2 - Upper right.
			m_terrainModel[index].x = m_heightMap[index2].x;
			m_terrainModel[index].y = m_heightMap[index2].y;
			m_terrainModel[index].z = m_heightMap[index2].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 0.0f;
			m_terrainModel[index].nx = m_heightMap[index2].nx;
			m_terrainModel[index].ny = m_heightMap[index2].ny;
			m_terrainModel[index].nz = m_heightMap[index2].nz;
			m_terrainModel[index].r = m_heightMap[index2].r;
			m_terrainModel[index].g = m_heightMap[index2].g;
			m_terrainModel[index].b = m_heightMap[index2].b;

			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;

			// Triangle 2 - Bottom right.
			m_terrainModel[index].x = m_heightMap[index4].x;
			m_terrainModel[index].y = m_heightMap[index4].y;
			m_terrainModel[index].z = m_heightMap[index4].z;
			m_terrainModel[index].tu = 1.0f;
			m_terrainModel[index].tv = 1.0f;
			m_terrainModel[index].nx = m_heightMap[index4].nx;
			m_terrainModel[index].ny = m_heightMap[index4].ny;
			m_terrainModel[index].nz = m_heightMap[index4].nz;
			m_terrainModel[index].r = m_heightMap[index4].r;
			m_terrainModel[index].g = m_heightMap[index4].g;
			m_terrainModel[index].b = m_heightMap[index4].b;

			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			index++;
		}
	}

	int test = 0;
	test++;
	return true;
}

void TerrainClass::ShutdownTerrainModel()
{
	// Release the terrain model data.
	if (m_terrainModel)
	{
		delete[] m_terrainModel;
		m_terrainModel = 0;
	}

	return;
}

void TerrainClass::CalculateTerrainVectors()
{
	int faceCount, i, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;

	// Calculate the number of faces in the terrain model.
	faceCount = m_vertexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	std::vector<DirectX::XMFLOAT3> testTerrain;
	testTerrain.clear();

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (i = 0; i < faceCount; i++)
	{


		// Get the three vertices for this face from the terrain model.
		vertex1.x = m_terrainModel[index].x;
		vertex1.y = m_terrainModel[index].y;
		vertex1.z = m_terrainModel[index].z;
		vertex1.tu = m_terrainModel[index].tu;
		vertex1.tv = m_terrainModel[index].tv;
		vertex1.nx = m_terrainModel[index].nx;
		vertex1.ny = m_terrainModel[index].ny;
		vertex1.nz = m_terrainModel[index].nz;
		index++;

		vertex2.x = m_terrainModel[index].x;
		vertex2.y = m_terrainModel[index].y;
		vertex2.z = m_terrainModel[index].z;
		vertex2.tu = m_terrainModel[index].tu;
		vertex2.tv = m_terrainModel[index].tv;
		vertex2.nx = m_terrainModel[index].nx;
		vertex2.ny = m_terrainModel[index].ny;
		vertex2.nz = m_terrainModel[index].nz;
		index++;

		vertex3.x = m_terrainModel[index].x;
		vertex3.y = m_terrainModel[index].y;
		vertex3.z = m_terrainModel[index].z;
		vertex3.tu = m_terrainModel[index].tu;
		vertex3.tv = m_terrainModel[index].tv;
		vertex3.nx = m_terrainModel[index].nx;
		vertex3.ny = m_terrainModel[index].ny;
		vertex3.nz = m_terrainModel[index].nz;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the tangent and binormal for this face back in the model structure.
		m_terrainModel[index - 1].tx = tangent.x;
		m_terrainModel[index - 1].ty = tangent.y;
		m_terrainModel[index - 1].tz = tangent.z;
		m_terrainModel[index - 1].bx = binormal.x;
		m_terrainModel[index - 1].by = binormal.y;
		m_terrainModel[index - 1].bz = binormal.z;

		m_terrainModel[index - 2].tx = tangent.x;
		m_terrainModel[index - 2].ty = tangent.y;
		m_terrainModel[index - 2].tz = tangent.z;
		m_terrainModel[index - 2].bx = binormal.x;
		m_terrainModel[index - 2].by = binormal.y;
		m_terrainModel[index - 2].bz = binormal.z;

		m_terrainModel[index - 3].tx = tangent.x;
		m_terrainModel[index - 3].ty = tangent.y;
		m_terrainModel[index - 3].tz = tangent.z;
		m_terrainModel[index - 3].bx = binormal.x;
		m_terrainModel[index - 3].by = binormal.y;
		m_terrainModel[index - 3].bz = binormal.z;

		DirectX::XMFLOAT3 pos;
		pos.x = m_terrainModel[index].x;
		pos.y = m_terrainModel[index].y;
		pos.z = m_terrainModel[index].z;
		testTerrain.push_back(pos);
	}

	return;
}

void TerrainClass::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;

	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of the tangent.
	length = (float)sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the tangent and then store it.
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of the binormal.
	length = (float)sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the binormal and then store it.
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}

bool TerrainClass::LoadTerrainCells(ID3D11Device* device)
{
	int cellHeight, cellWidth, cellRowCount, i, j, index;
	bool result;

	// Set the height and width of each terrain cell to a fixed 33x33 vertex array.
	//cellHeight = 33;
	//cellWidth = 33;
	cellHeight = 33;
	cellWidth = 33;
	// Calculate the number of cells needed to store the terrain data.
	cellRowCount = (m_terrainWidth - 1) / (cellWidth - 1);
	m_cellCount = cellRowCount * cellRowCount;

	// Create the terrain cell array.
	m_TerrainCells = new TerrainCellClass[m_cellCount];
	if (!m_TerrainCells)
	{
		return false;
	}

	std::vector<DirectX::XMFLOAT3> testTerrain;
	testTerrain.clear();

	// Loop through and initialize all the terrain cells.
	for (j = 0; j < cellRowCount; j++)
	{
		for (i = 0; i < cellRowCount; i++)
		{
			index = (cellRowCount * j) + i;

			DirectX::XMFLOAT3 pos;
			pos.x = m_terrainModel[index].x;
			pos.y = m_terrainModel[index].y;
			pos.z = m_terrainModel[index].z;
			testTerrain.push_back(pos);

			result = m_TerrainCells[index].Initialize(device, m_terrainModel, i, j, cellHeight, cellWidth, m_terrainWidth);
			if (!result)
			{
				return false;
			}

		}
	}
	
	int testXX = 0;
	testXX++;
	return true;
}

void TerrainClass::ShutdownTerrainCells()
{
	int i;

	// Release the terrain cell array.
	if (m_TerrainCells)
	{
		for (i = 0; i < m_cellCount; i++)
		{
			m_TerrainCells[i].Shutdown();
		}

		delete[] m_TerrainCells;
		m_TerrainCells = 0;
	}

	return;
}

//bool TerrainClass::RenderCell(ID3D11DeviceContext* deviceContext, int cellId, FrustumClass* Frustum)
bool TerrainClass::RenderCell(ID3D11DeviceContext* deviceContext, int cellId)
{
	float maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth;
	bool result;

	// Get the dimensions of the terrain cell.
	m_TerrainCells[cellId].GetCellDimensions(maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth);

	// Check if the cell is visible.  If it is not visible then just return and don't render it.
	//result = Frustum->CheckRectangle2(maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth);
	result = true;
	if (!result)
	{
		// Increment the number of cells that were culled.
		m_cellsCulled++;

		return false;
	}

	// If it is visible then render it.
	m_TerrainCells[cellId].Render(deviceContext);

	// Add the polygons in the cell to the render count.
	m_renderCount += (m_TerrainCells[cellId].GetVertexCount() / 3);

	// Increment the number of cells that were actually drawn.
	m_cellsDrawn++;

	return true;
}

void TerrainClass::RenderCellLines(ID3D11DeviceContext* deviceContext, int cellId)
{
	m_TerrainCells[cellId].RenderLineBuffers(deviceContext);
	return;
}

int TerrainClass::GetCellIndexCount(int cellId)
{
	return m_TerrainCells[cellId].GetIndexCount();
}

int TerrainClass::GetCellLinesIndexCount(int cellId)
{
	return m_TerrainCells[cellId].GetLineBuffersIndexCount();
}

int TerrainClass::GetCellCount()
{
	return m_cellCount;
}

int TerrainClass::GetRenderCount()
{
	return m_renderCount;
}

int TerrainClass::GetCellsDrawn()
{
	return m_cellsDrawn;
}

int TerrainClass::GetCellsCulled()
{
	return m_cellsCulled;
}

bool TerrainClass::GetHeightAtPosition(float inputX, float inputZ, float& height)
{
	int i, cellId, index;
	float vertex1[3], vertex2[3], vertex3[3];
	bool foundHeight;
	float maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth;

	// Loop through all of the terrain cells to find out which one the inputX and inputZ would be inside.
	cellId = -1;
	for (i = 0; i < m_cellCount; i++)
	{
		// Get the current cell dimensions.
		m_TerrainCells[i].GetCellDimensions(maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth);

		// Check to see if the positions are in this cell.
		if ((inputX < maxWidth) && (inputX > minWidth) && (inputZ < maxDepth) && (inputZ > minDepth))
		{
			cellId = i;
			i = m_cellCount;
		}
	}

	// If we didn't find a cell then the input position is off the terrain grid.
	if (cellId == -1)
	{
		return false;
	}

	// If this is the right cell then check all the triangles in this cell to see what the height of the triangle at this position is.
	for (i = 0; i < (m_TerrainCells[cellId].GetVertexCount() / 3); i++)
	{
		index = i * 3;

		vertex1[0] = m_TerrainCells[cellId].m_vertexList[index].x;
		vertex1[1] = m_TerrainCells[cellId].m_vertexList[index].y;
		vertex1[2] = m_TerrainCells[cellId].m_vertexList[index].z;
		index++;

		vertex2[0] = m_TerrainCells[cellId].m_vertexList[index].x;
		vertex2[1] = m_TerrainCells[cellId].m_vertexList[index].y;
		vertex2[2] = m_TerrainCells[cellId].m_vertexList[index].z;
		index++;

		vertex3[0] = m_TerrainCells[cellId].m_vertexList[index].x;
		vertex3[1] = m_TerrainCells[cellId].m_vertexList[index].y;
		vertex3[2] = m_TerrainCells[cellId].m_vertexList[index].z;

		// Check to see if this is the polygon we are looking for.
		foundHeight = CheckHeightOfTriangle(inputX, inputZ, height, vertex1, vertex2, vertex3);
		if (foundHeight)
		{
			return true;
		}
	}

	return false;
}

bool TerrainClass::CheckHeightOfTriangle(float x, float z, float& height, float v0[3], float v1[3], float v2[3])
{
	float startVector[3], directionVector[3], edge1[3], edge2[3], normal[3];
	float Q[3], e1[3], e2[3], e3[3], edgeNormal[3], temp[3];
	float magnitude, D, denominator, numerator, t, determinant;

	// Starting position of the ray that is being cast.
	startVector[0] = x;
	startVector[1] = 0.0f;
	startVector[2] = z;

	// The direction the ray is being cast.
	directionVector[0] = 0.0f;
	directionVector[1] = -1.0f;
	directionVector[2] = 0.0f;

	// Calculate the two edges from the three points given.
	edge1[0] = v1[0] - v0[0];
	edge1[1] = v1[1] - v0[1];
	edge1[2] = v1[2] - v0[2];

	edge2[0] = v2[0] - v0[0];
	edge2[1] = v2[1] - v0[1];
	edge2[2] = v2[2] - v0[2];

	// Calculate the normal of the triangle from the two edges.
	normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
	normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
	normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

	magnitude = (float)sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));
	normal[0] = normal[0] / magnitude;
	normal[1] = normal[1] / magnitude;
	normal[2] = normal[2] / magnitude;

	// Find the distance from the origin to the plane.
	D = ((-normal[0] * v0[0]) + (-normal[1] * v0[1]) + (-normal[2] * v0[2]));

	// Get the denominator of the equation.
	denominator = ((normal[0] * directionVector[0]) + (normal[1] * directionVector[1]) + (normal[2] * directionVector[2]));

	// Make sure the result doesn't get too close to zero to prevent divide by zero.
	if (fabs(denominator) < 0.0001f)
	{
		return false;
	}

	// Get the numerator of the equation.
	numerator = -1.0f * (((normal[0] * startVector[0]) + (normal[1] * startVector[1]) + (normal[2] * startVector[2])) + D);

	// Calculate where we intersect the triangle.
	t = numerator / denominator;

	// Find the intersection vector.
	Q[0] = startVector[0] + (directionVector[0] * t);
	Q[1] = startVector[1] + (directionVector[1] * t);
	Q[2] = startVector[2] + (directionVector[2] * t);

	// Find the three edges of the triangle.
	e1[0] = v1[0] - v0[0];
	e1[1] = v1[1] - v0[1];
	e1[2] = v1[2] - v0[2];

	e2[0] = v2[0] - v1[0];
	e2[1] = v2[1] - v1[1];
	e2[2] = v2[2] - v1[2];

	e3[0] = v0[0] - v2[0];
	e3[1] = v0[1] - v2[1];
	e3[2] = v0[2] - v2[2];

	// Calculate the normal for the first edge.
	edgeNormal[0] = (e1[1] * normal[2]) - (e1[2] * normal[1]);
	edgeNormal[1] = (e1[2] * normal[0]) - (e1[0] * normal[2]);
	edgeNormal[2] = (e1[0] * normal[1]) - (e1[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v0[0];
	temp[1] = Q[1] - v0[1];
	temp[2] = Q[2] - v0[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Calculate the normal for the second edge.
	edgeNormal[0] = (e2[1] * normal[2]) - (e2[2] * normal[1]);
	edgeNormal[1] = (e2[2] * normal[0]) - (e2[0] * normal[2]);
	edgeNormal[2] = (e2[0] * normal[1]) - (e2[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v1[0];
	temp[1] = Q[1] - v1[1];
	temp[2] = Q[2] - v1[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Calculate the normal for the third edge.
	edgeNormal[0] = (e3[1] * normal[2]) - (e3[2] * normal[1]);
	edgeNormal[1] = (e3[2] * normal[0]) - (e3[0] * normal[2]);
	edgeNormal[2] = (e3[0] * normal[1]) - (e3[1] * normal[0]);

	// Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
	temp[0] = Q[0] - v2[0];
	temp[1] = Q[1] - v2[1];
	temp[2] = Q[2] - v2[2];

	determinant = ((edgeNormal[0] * temp[0]) + (edgeNormal[1] * temp[1]) + (edgeNormal[2] * temp[2]));

	// Check if it is outside.
	if (determinant > 0.001f)
	{
		return false;
	}

	// Now we have our height.
	height = Q[1];

	return true;
}
