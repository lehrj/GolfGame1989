#pragma once
#include <fstream>
#include "Utility.h"

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


struct Environ
{
    std::string                         name;
    double                              airDensity;        // in kg/m^3
    std::string                         airDensityStr;
    DirectX::SimpleMath::Vector3        holePosition;
    double                              gravity;           // in m/s^2
    std::string                         gravityStr;
    double                              landingFrictionScale;
    std::string                         landingFrictionScaleStr;
    double                              landingHardnessScale;
    std::string                         landingHardnessScaleStr;
    int                                 par;
    float                               scale;
    float                               teeDirection;       // start direction for first shot
    DirectX::SimpleMath::Vector3        teePosition;        // start position for first shot
    DirectX::XMVECTORF32                terrainColor;
    DirectX::SimpleMath::Vector3        wind;               // in m/s
    std::string                         windXStr;
    std::string                         windYStr;
    std::string                         windZStr;
};

enum class FixtureType
{
    FIXTURETYPE_FLAGSTICK,
    FIXTURETYPE_TEEBOX,
    FIXTURETYPE_BRIDGE,
    //FIXTURETYPE_TREE01,
    //FIXTURETYPE_TREE02,
    FIXTURETYPE_TREE03,     // V tree
    FIXTURETYPE_TREE04,     // Circle pink tree
    FIXTURETYPE_TREE05,     // Circle white tree
    FIXTURETYPE_TREE06,     // A shape tree w black right shadow with delta base
    FIXTURETYPE_TREE07,     // A shape tree chevron shape
    FIXTURETYPE_TREE08,
    FIXTURETYPE_TREE09,     // A shape tree chevron shape with white highlights
    FIXTURETYPE_TREE10,
};

struct Fixture
{
    unsigned int                    idNumber;
    DirectX::SimpleMath::Vector3    position;
    FixtureType                     fixtureType;  // think of a better name later
    float                           animationVariation;
    float                           distanceToCamera;
};

// Class to handle environment and gameplay world needs
class Environment
{
public:
    Environment();

    double GetAirDensity() const { return m_currentEnviron.airDensity; };
    std::string GetAirDensityString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].airDensityStr; };
    double GetAirDensity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].airDensity; };
    DirectX::XMVECTORF32 GetEnvironColor() const { return m_currentEnviron.terrainColor; };
    std::string GetEnvironName(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].name; };
    std::vector<Fixture> GetFixtureBucket() { return m_fixtureBucket; };

    double GetGravity() const { return m_currentEnviron.gravity; };
    std::string GetGravityString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].gravityStr; };
    double GetGravity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].gravity; };
    DirectX::SimpleMath::Vector3 GetGroundHeight(DirectX::SimpleMath::Vector3 aPos);
    std::vector<DirectX::VertexPositionColor> GetFlagVertex() const { return m_flagVertex; };
    
    DirectX::SimpleMath::Vector3 GetHolePosition() const { return m_currentEnviron.holePosition; };
    double GetHoleRadius() const { return m_holeRadius * m_currentEnviron.scale; };
    std::vector<DirectX::VertexPositionColor> GetHoleVertex() const { return m_holeVertex; };
    double GetLandingHeight() const { return m_landingHeight; };
    double GetLauchHeight() const { return m_launchHeight; };
    int GetNumerOfEnvirons() const { return m_environsAvailable; };
    int GetNumberOfEnvironSelectDisplayVariables() const { return m_environSelectDisplayDataPoints; };
    int GetPar() const { return m_currentEnviron.par; };
    float GetScale() const { return m_currentEnviron.scale; };
    float GetTeeDirectionDegrees() const { return m_currentEnviron.teeDirection; };

    DirectX::SimpleMath::Vector3 GetTeePosition() const { return m_currentEnviron.teePosition; };
    std::vector<DirectX::VertexPositionColor> GetTerrainColorVertex();
    std::vector<DirectX::VertexPositionNormalColor> GetTerrainPositionNormalColorVertex();
    float GetTerrainHeightAtPos(DirectX::XMFLOAT3 aPos) const;
    float GetTerrainHeightAtPos2(DirectX::XMFLOAT3 aPos) const;
    DirectX::SimpleMath::Vector3 GetTerrainNormal(DirectX::SimpleMath::Vector3 aPos) const;
    DirectX::XMFLOAT3 GetTerrainPosition(DirectX::XMFLOAT3 aPos);
    bool SetPosToTerrainWithCheck(DirectX::XMFLOAT3& aPos);
    void SetPosToTerrain(DirectX::XMFLOAT3& aPos);
    bool CheckTerrainTriangleHeight(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const;
    bool CheckTerrainTriangleHeight2(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const;
    bool CheckTerrainTriangleHeight3(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const;
    bool CheckTerrainTriangleHeight4(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const;
    double GetWindDirection() const;
    DirectX::SimpleMath::Vector3 GetWindVector() const { return m_currentEnviron.wind; };
    double GetWindX() const { return m_currentEnviron.wind.x; };
    std::string GetWindXString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windXStr; };
    double GetWindY() const { return m_currentEnviron.wind.y; };
    std::string GetWindYString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windYStr; };
    double GetWindZ() const { return m_currentEnviron.wind.z; };
    std::string GetWindZString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windZStr; };
    bool InitializeTerrain();
    
    void SortFixtureBucketByDistance();
    void UpdateEnvironment(const int aIndex);
    void UpdateFixtureDistanceToCamera(const DirectX::SimpleMath::Vector3& aCameraPos);
    void UpdateFixtures(const DirectX::SimpleMath::Vector3& aPos);

private:

    void BuildFlagVertex(DirectX::SimpleMath::Vector3 aPos);
    void BuildHoleVertex(DirectX::SimpleMath::Vector3 aPos);
    bool BuildTerrainModel();
    bool CalculateTerrainNormals();
    void CreateDataStrings();
    void LoadEnvironmentData();
    void LoadFixtureBucket();

    bool LoadHeightMap();
    void SetLandingHeight(double aLandingHeight);
    void SetLauchHeight(double aLaunchHeight);
    void ScaleTerrain();

    Environ                             m_currentEnviron;
    std::vector<Environ>                m_environs;
    const int                           m_environsAvailable = 3;
    const int                           m_environSelectDisplayDataPoints = 5;

    std::vector<Fixture>                m_fixtureBucket;

    std::vector<DirectX::VertexPositionColor> m_flagVertex;

    std::vector<DirectX::VertexPositionColor> m_holeVertex;
    const int                           m_holeResolution = 30;          // number of vertices used to draw hole circle
    //const double                        m_holeRadius = 0.10795;       // Radius of the hole, future updates could include addition of "big cup" or "tiny cup" hole sizes
    const double                        m_holeRadius = .50795;

    double                              m_landingHeight = 0.0;     // in meters
    double                              m_launchHeight = 0.0;      // in meters
    double                              m_testHeight = 0.0;        // in meters, for testing ground height functions
   
    std::vector<DirectX::VertexPositionNormal> m_heightMap;
    const float                         m_heightScale = 0.015;

    int                                 m_terrainHeight = 0;
    int                                 m_terrainLength = 0;
    int                                 m_terrainWidth = 0;
    std::vector<DirectX::VertexPositionNormal> m_terrainModel;

    // variables for bounce and roll functionality not yet implemented
    double                              m_landingFriction;
    double                              m_landingHardness;
    double                              m_landingXslope;
    double                              m_landingZslope;

    // min max consts
    const double                        m_minAirDensity = 0.0;
    const double                        m_maxAirDensity = 68.0; // just above the air density of Venus
    const double                        m_minGravity = 0.1;
    const double                        m_maxGravity = 28.0;    // approximate value for the mass of the sun
    const double                        m_minMaxHeight = 450.0; // Launch & Landing min/max heights is just above the largest elevation change (>400 meters) of any real golf course which is the Extreme 19 in Limpopo Province South Africa
    const double                        m_minMaxWind = 667.0;   // highest know wind speed on Neptune


};



