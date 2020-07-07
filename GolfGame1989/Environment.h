#pragma once

class Environment
{
public:
    const double GetAirDensity() { return m_airDensity; };
    const double GetGravity() { return m_gravity; };
    const double GetLandingHeight() { return m_landingHeight; };
    const double GetLauchHeight() { return m_launchHeight; };
    const double GetWindX() { return m_windX; };
    const double GetWindY() { return m_windY; };
    const double GetWindZ() { return m_windZ; };


    void PrintEnvironmentData();
    void ReadInEnvironmentData();

    void SetDefaultEnvironment();
    
private:
    void InputEnvironmentData();
    void InputEnvironmentDataBasic();

    void SetAirDensity(double aDensity);   
    void SetGravity(double aGravity);
    void SetLandingHeight(double aLandingHeight);
    void SetLauchHeight(double aLaunchHeight);
    void SetWindX(const double& aWindX);
    void SetWindY(const double& aWindY);
    void SetWindZ(const double& aWindZ);

    double                              m_airDensity = 1.225;        // in kg/m^3
    double                              m_gravity = -9.8;           // in m/s^2
    double                              m_landingHeight = 0.0;     // in meters
    double                              m_launchHeight = 0.0;      // in meters
    double                              m_windX = 0.0;             // in m/s
    double                              m_windY = 0.0;             // in m/s
    double                              m_windZ = 0.0;             // in m/s

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
};

