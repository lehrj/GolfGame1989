#include "pch.h"
#include "Environment.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

Environment::Environment()
{
    LoadEnvironmentData();
    CreateDataStrings();
    m_currentEnviron = m_environs[0];
}

void Environment::CreateDataStrings()
{
    int precisionVal = 2;
    std::stringstream inVal;
    inVal.precision(precisionVal);

    for (int i = 0; i < m_environs.size(); ++i)
    {
        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].airDensity;
        m_environs[i].airDensityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].gravity;
        m_environs[i].gravityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].windX;
        m_environs[i].windXStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].windY;
        m_environs[i].windYStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].windZ;
        m_environs[i].windZStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingFrictionScale;
        m_environs[i].landingFrictionScaleStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingHardnessScale;
        m_environs[i].landingHardnessScaleStr = inVal.str();
    }
}

void Environment::LoadEnvironmentData()
{
    m_environs.clear();
    m_environs.resize(m_environsAvailable);

    int i = 0;

    m_environs[i].name = "Calm";
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].windX = 0.0;
    m_environs[i].windY = 0.0;
    m_environs[i].windZ = 0.0;
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;

    ++i;
    m_environs[i].name = "Breezy";
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].windX = 0.0;
    m_environs[i].windY = 0.0;
    m_environs[i].windZ = 5.0;
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;

    ++i;
    m_environs[i].name = "Non Terrestrial (Alien Golf!!)";
    m_environs[i].terrainColor = DirectX::Colors::Blue;
    m_environs[i].airDensity = 11.2;
    m_environs[i].gravity = -5.8;
    m_environs[i].windX = 3.0;
    m_environs[i].windY = 1.0;
    m_environs[i].windZ = 0.0;
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
}

void Environment::SetLandingHeight(double aLandingHeight)
{
    m_landingHeight = aLandingHeight;
}

void Environment::SetLauchHeight(double aLaunchHeight)
{
    m_launchHeight = aLaunchHeight;
}

void Environment::UpdateEnvironment(const int aIndex)
{
    m_currentEnviron = m_environs[aIndex];
    m_landingFriction = m_environs[aIndex].landingFrictionScale;
    m_landingHardness = m_environs[aIndex].landingHardnessScale;
}