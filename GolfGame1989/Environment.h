#pragma once
#include "Utility.h"

struct Environ
{
    std::string                         name;
    DirectX::XMVECTORF32                terrainColor;
    double                              airDensity;        // in kg/m^3
    std::string                         airDensityStr;
    double                              gravity;           // in m/s^2
    std::string                         gravityStr;
    double                              windX;             // in m/s
    std::string                         windXStr;
    double                              windY;             // in m/s
    std::string                         windYStr;
    double                              windZ;             // in m/s
    std::string                         windZStr;
    double                              landingFrictionScale;
    std::string                         landingFrictionScaleStr;
    double                              landingHardnessScale;
    std::string                         landingHardnessScaleStr;
};

class Environment
{
public:
    Environment();

    double GetAirDensity() const { return m_currentEnviron.airDensity; };
    std::string GetAirDensityString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].airDensityStr; };
    double GetAirDensity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].airDensity; };
    double GetGravity() const { return m_currentEnviron.gravity; };
    std::string GetGravityString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].gravityStr; };
    double GetGravity(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].gravity; };
    double GetLandingHeight() const { return m_landingHeight; };
    double GetLauchHeight() const { return m_launchHeight; };
    DirectX::XMVECTORF32 GetEnvironColor() const { return m_currentEnviron.terrainColor; };
    std::string GetEnvironName(const int aEnvironmentIndex) const {return m_environs[aEnvironmentIndex].name;};
    int GetNumerOfEnvirons() const { return m_environsAvailable; };
    int GetNumberOfEnvironSelectDisplayVariables() const { return m_environSelectDisplayDataPoints; };
    double GetWindX() const { return m_currentEnviron.windX; };
    std::string GetWindXString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windXStr; };
    double GetWindX(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windX; };
    double GetWindY() const { return m_currentEnviron.windY; };
    std::string GetWindYString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windYStr; };
    double GetWindY(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windY; };
    double GetWindZ() const { return m_currentEnviron.windZ; };
    std::string GetWindZString(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windZStr; };
    double GetWindZ(const int aEnvironmentIndex) const { return m_environs[aEnvironmentIndex].windZ; };
    void UpdateEnvironment(const int aIndex);

private:
    void CreateDataStrings();
    void LoadEnvironmentData();
    void SetLandingHeight(double aLandingHeight);
    void SetLauchHeight(double aLaunchHeight);

    double                              m_landingHeight = 0.0;     // in meters
    double                              m_launchHeight = 0.0;      // in meters

    // variables for bounce and roll functionality not yet implemented
    double                              m_landingFriction;
    double                              m_landingHardness;
    double                              m_landingXslope;
    double                              m_landingZslope;

    // min max consts
    const double                        m_minAirDensity = 0.0;
    const double                        m_maxAirDensity = 68.0; // just above the air density of Venus
    const double                        m_minGravity = 0.1;
    const double                        m_maxGravity = 28.0; // approximate value for the mass of the sun
    const double                        m_minMaxHeight = 450.0; // Launch & Landing min/max heights is just above the largest elevation change (>400 meters) of any real golf course which is the Extreme 19 in Limpopo Province South Africa
    const double                        m_minMaxWind = 667.0;// highest know wind speed on Neptune

    std::vector<Environ>                m_environs;
    Environ                             m_currentEnviron;
    const int                           m_environsAvailable = 3;
    const int                           m_environSelectDisplayDataPoints = 5;
};

