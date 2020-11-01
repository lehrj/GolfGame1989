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
    const int startEnviron = 1;  // ToDo: add error checking 
    m_currentEnviron = m_environs[startEnviron];
    BuildFlagVertex(m_environs[startEnviron].holePosition);
    BuildHoleVertex(m_environs[startEnviron].holePosition);
    LoadFixtureBucket();
}

void Environment::BuildFlagVertex(DirectX::SimpleMath::Vector3 aPos)
{
    const float poleHeight = .1;
    const float flagWidth = .02;
    const float flagHeight = .01;
    const DirectX::XMVECTORF32 flagColor = DirectX::Colors::Red;
    const DirectX::XMVECTORF32 poleColor = DirectX::Colors::White;

    DirectX::SimpleMath::Vector3 poleBase = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 poleTop = poleBase;
    poleTop.y += poleHeight;
    DirectX::SimpleMath::Vector3 flagTip = poleTop;
    flagTip.y -= flagHeight;
    flagTip.x -= flagWidth;
    flagTip.z -= flagWidth;
    double windDirection = GetWindDirection();
    flagTip = DirectX::SimpleMath::Vector3::Transform(flagTip, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(windDirection)));

    DirectX::SimpleMath::Vector3 flagBottom = poleTop;
    flagBottom.y -= flagHeight + flagHeight;

    DirectX::VertexPositionColor poleBaseVertex(poleBase + aPos, poleColor);
    DirectX::VertexPositionColor poleTopVertex(poleTop + aPos, poleColor);
    DirectX::VertexPositionColor flagTopVertex(poleTop + aPos, flagColor);
    DirectX::VertexPositionColor flagTipVertex(flagTip + aPos, flagColor);
    DirectX::VertexPositionColor flagBottomVertex(flagBottom + aPos, flagColor);

    m_flagVertex.clear();
    m_flagVertex.push_back(poleBaseVertex);
    m_flagVertex.push_back(poleTopVertex);
    m_flagVertex.push_back(flagTopVertex);
    m_flagVertex.push_back(flagTipVertex);
    m_flagVertex.push_back(flagBottomVertex);
}

void Environment::BuildHoleVertex(DirectX::SimpleMath::Vector3 aPos)
{
    m_holeVertex.clear();
    const int vertexCount = m_holeResolution;
    const float holeRadius= static_cast<float>(m_holeRadius * m_currentEnviron.scale);
    for (int i = 0; i <= vertexCount; ++i)
    {
        double t = Utility::GetPi() * 2 * i / vertexCount;
        m_holeVertex.push_back(DirectX::VertexPositionColor(DirectX::SimpleMath::Vector3(static_cast<float>((holeRadius * cos(t))), static_cast<float>(m_landingHeight), static_cast<float>((holeRadius * -sin(t)))) + aPos, DirectX::Colors::White));
    }
}

void Environment::CreateDataStrings()
{
    std::stringstream inVal;
    inVal.precision(Utility::GetNumericalPrecisionForUI());

    for (int i = 0; i < m_environs.size(); ++i)
    {
        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].airDensity;
        m_environs[i].airDensityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].gravity;
        m_environs[i].gravityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.x;
        m_environs[i].windXStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.y;
        m_environs[i].windYStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.z;
        m_environs[i].windZStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingFrictionScale;
        m_environs[i].landingFrictionScaleStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingHardnessScale;
        m_environs[i].landingHardnessScaleStr = inVal.str();
    }
}

// While this could be done once per environment update, future updates could have moment to moment wind changes
double Environment::GetWindDirection() const
{ 
    DirectX::SimpleMath::Vector3 windVec = m_currentEnviron.wind;
    DirectX::SimpleMath::Vector3 zeroDirection(0.0, 0.0, -1.0);

    double direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(windVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(windVec, zeroDirection)) > 0.0f)
    {
        direction = -direction;
    }

    return direction;
}

void Environment::LoadEnvironmentData()
{
    m_environs.clear();
    m_environs.resize(m_environsAvailable);

    int i = 0;

    m_environs[i].name = "Calm";   
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(-1.7f, 0.0f, -1.001f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, -1.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    ++i;
    m_environs[i].name = "Breezy";    
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, .0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(-10.0f, 0.0f, 0.0f);

    ++i;
    m_environs[i].name = "Non Terrestrial (Alien Golf!!)";    
    m_environs[i].airDensity = 11.2;
    m_environs[i].gravity = -5.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 45.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Blue;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(3.0f, 0.0f, 0.69f);
}

void Environment::LoadFixtureBucket()
{
    m_fixtureBucket.clear();
    int i = 0;
    float variationMin = 0;
    float variationMax = 69;
    int min = 0;
    int max = 10;
    float fMin = 0;
    float fMax = 1;

    Fixture inputFixture;

    inputFixture.idNumber = i;
    inputFixture.position = DirectX::SimpleMath::Vector3(-1.1f, 0.0f, 0.0f);
    inputFixture.fixtureType = FixtureType::FIXTURETYPE_TREE01;
    //inputFixture.animationVariation = 0.0f;  // temp, use random function once implemented
    //inputFixture.animationVariation = Utility::GetAnimationTimeSeed(variationMax);
    float animVar = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    inputFixture.animationVariation = animVar;
    inputFixture.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(inputFixture.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(inputFixture);

    ++i;
    inputFixture.idNumber = i;
    inputFixture.position = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.3f);
    inputFixture.fixtureType = FixtureType::FIXTURETYPE_TREE02;
    //inputFixture.animationVariation = 0.2f;   
    //inputFixture.animationVariation = Utility::GetAnimationTimeSeed(variationMax);
    float animVar1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    inputFixture.animationVariation = animVar1;
    inputFixture.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(inputFixture.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(inputFixture);

    ++i;
    inputFixture.idNumber = i;
    inputFixture.position = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, -0.3f);
    inputFixture.fixtureType = FixtureType::FIXTURETYPE_TREE03;
    //inputFixture.animationVariation = 0.9f;
    //inputFixture.animationVariation = Utility::GetAnimationTimeSeed(variationMax);
    float animVar2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    inputFixture.animationVariation = animVar2;
    inputFixture.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(inputFixture.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(inputFixture);

    float a = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float c = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);


    //m_fixtureBucket[0].animationVariation = a;
    //m_fixtureBucket[1].animationVariation = b;
    //m_fixtureBucket[2].animationVariation = c;



    for (int j = 0; j < m_fixtureBucket.size(); ++j)
    {
        float testVar = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        m_fixtureBucket[j].animationVariation = rand();
        m_fixtureBucket[j].position = DirectX::SimpleMath::Vector3::Zero;
    }


    m_fixtureBucket.clear();
    float varMax = 10.0;
    float posMin = -2.0;
    float posMax = 4.0;
    int fixtureTypeNumMin = 1;
    int fixtureTypeNumMax = 11;

    for (int j = 0; j < 100; ++j)
    {
        float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX/ posMax)) -2.0;
        float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/ posMax);
        float z = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX/ posMax)) - 2.0;
        //float z = static_cast <float> (fixtureTypeNumMin + rand()) / static_cast <float> (RAND_MAX / (posMax - posMin));
        float aVar = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/ varMax);

        Fixture fixt;

        fixt.idNumber = j;
        fixt.position = DirectX::SimpleMath::Vector3(x, 0.0, z);
        //fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;

        //float r3 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
        int fixtureNum = fixtureTypeNumMin + rand() / (RAND_MAX / (fixtureTypeNumMax - fixtureTypeNumMin));
        if (fixtureNum == 1)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE01;
        }
        else if (fixtureNum == 2)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE02;
        }
        else if (fixtureNum == 3)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE03;
        }
        else if (fixtureNum == 4)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE04;
        }
        else if (fixtureNum == 5)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE05;
        }
        else if (fixtureNum == 6)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE06;
        }
        else if (fixtureNum == 7)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE07;
        }
        else if (fixtureNum == 8)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE08;
        }
        else if (fixtureNum == 9)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }
        else if (fixtureNum == 10)
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE10;
        }
        else
        {
            fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        }

        fixt.animationVariation = aVar;
        fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);


        //fixt.fixtureType = FixtureType::FIXTURETYPE_TREE09;
        
        m_fixtureBucket.push_back(fixt);
    }
}

void Environment::SetLandingHeight(double aLandingHeight)
{
    m_landingHeight = aLandingHeight;
}

void Environment::SetLauchHeight(double aLaunchHeight)
{
    m_launchHeight = aLaunchHeight;
}

void Environment::SortFixtureBucketByDistance()
{
    std::sort(m_fixtureBucket.begin(), m_fixtureBucket.end(),
        [](const auto& i, const auto& j) {return i.distanceToCamera > j.distanceToCamera; });
    
}

void Environment::UpdateEnvironment(const int aIndex)
{
    m_currentEnviron = m_environs[aIndex];
    m_landingFriction = m_environs[aIndex].landingFrictionScale;
    m_landingHardness = m_environs[aIndex].landingHardnessScale;
    BuildFlagVertex(m_environs[aIndex].holePosition);
    BuildHoleVertex(m_environs[aIndex].holePosition);
}

void Environment::UpdateFixtures(const DirectX::SimpleMath::Vector3 &aPos)
{

    UpdateFixtureDistanceToCamera(aPos);
    SortFixtureBucketByDistance();

}

void Environment::UpdateFixtureDistanceToCamera(const DirectX::SimpleMath::Vector3 &aCameraPos)
{
    for (int i = 0; i < m_fixtureBucket.size(); ++i)
    {
        m_fixtureBucket[i].distanceToCamera = DirectX::SimpleMath::Vector3::Distance(m_fixtureBucket[i].position, aCameraPos);
    }
}