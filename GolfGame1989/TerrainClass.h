#pragma once


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


class TerrainClass
{
    // height map testing
private:
    
    struct VertexType
    {       
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT3 normal;
    };
    
    struct HeightMapType
    {
        DirectX::VertexPositionNormalColor vertPosNormColor;
        //float x, y, z;
        //float tu, tv;
        //float nx, ny, nz;
    };
    /*
    struct VectorType
    {
        float x, y, z;
    };
    struct HeightMapInfo {        // Heightmap structure
        int terrainWidth;        // Width of heightmap
        int terrainHeight;        // Height (Length) of heightmap
        DirectX::XMFLOAT3* heightMap;    // Array to store terrain's vertex positions
    };
    */

public:
    TerrainClass();
    TerrainClass(const TerrainClass&);
    ~TerrainClass();

    bool Initialize(ID3D11Device*, char*);
    void Shutdown();
    void Render(ID3D11DeviceContext*);

    int GetIndexCount();

private:
    bool LoadHeightMap(char*);
    void NormalizeHeightMap();
    void ShutdownHeightMap();

    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext*);

private:
    int m_terrainWidth, m_terrainHeight;
    int m_vertexCount, m_indexCount;
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    //HeightMapType* m_heightMap;

    DirectX::VertexPositionNormalColor* m_heightMap;

    std::vector < DirectX::VertexPositionNormalColor> m_testVertVec;
};

