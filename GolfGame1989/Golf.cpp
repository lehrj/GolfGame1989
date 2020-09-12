#include "pch.h"
#include "Golf.h"
#include <fstream>
#include <sstream>

Golf::Golf()
{
    pEnvironment = new Environment();

    pCharacter = new GolfCharacter();
    pSwing = new GolfSwing();
    
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pSwing->InputClub(1);
    pSwing->UpdateGolfSwingValues();
    pBall = new GolfBall();
    pBall->SetDefaultBallValues(pEnvironment);
    pPlay = new GolfPlay();
    SetCharacter(0);
    pSwing->ZeroDataForUI();
    pBall->ZeroDataForUI();
    //BuildTrajectoryData(); // WLJ turn off to disable auto draw of projectile without powerbar input 
    BuildUIstrings();
    BuildEnvironSelectStrings();
    SetShotStartPos(pEnvironment->GetTeePosition());
    SetBallPosition(GetShotStartPos());
}

Golf::~Golf()
{
    delete pBall;
    delete pSwing;
    delete pEnvironment;
    delete pCharacter;
    delete pPlay;
}

void Golf::BuildEnvironSelectStrings()
{
    m_environSelectStrings.clear();
    const int environCount = pEnvironment->GetNumerOfEnvirons();

    std::vector<std::string> strVec;

    for (int i = 0; i < environCount; ++i)
    {
        std::string inString = pEnvironment->GetEnvironName(i);
        strVec.push_back(inString);
    }
    m_environSelectStrings.push_back(strVec);
    strVec.clear();

    for (int i = 0; i < environCount; ++i)
    {
        std::string inString = "Air Density = " + pEnvironment->GetAirDensityString(i) + " kg/m^3";
        strVec.push_back(inString);
    }
    m_environSelectStrings.push_back(strVec);
    strVec.clear();

    for (int i = 0; i < environCount; ++i)
    {
        std::string inString = "Gravity = " + pEnvironment->GetGravityString(i) + " m/s^2";
        strVec.push_back(inString);
    }
    m_environSelectStrings.push_back(strVec);
    strVec.clear();

    for (int i = 0; i < environCount; ++i)
    {
        std::string inString = "Wind X = " + pEnvironment->GetWindXString(i) + " m/s ";
        strVec.push_back(inString);
    }
    m_environSelectStrings.push_back(strVec);
    strVec.clear();

    for (int i = 0; i < environCount; ++i)
    {
        std::string inString = "Wind Z = " + pEnvironment->GetWindZString(i) + " m/s ";
        strVec.push_back(inString);
    }
    m_environSelectStrings.push_back(strVec);
}

void Golf::BuildHyrdraShotData()
{
    const double hydraAxisTilt = Utility::ToRadians(25.0);
    const double hydraPlaneTurn = Utility::ToRadians(7.0);

    m_hydraData.clear();
    m_hydraTimeSteps.clear();

    m_hydraData.resize(9);
    m_hydraTimeSteps.resize(9);

    pBall->FireProjectile(pSwing->CalculateLaunchVector());

    InputData();
    ScaleCordinates();
    int i = 0;
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();  
    m_hydraData[i] = m_shotPath;
    ++i;

    // 1
    pSwing->UpdateImpactDataPlane(hydraPlaneTurn);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 2
    pSwing->UpdateImpactDataPlane(-hydraPlaneTurn * 2);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 3
    pSwing->UpdateImpactDataPlane(hydraPlaneTurn * 2);
    pSwing->UpdateImpactDataAxis(hydraAxisTilt);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 4
    pSwing->UpdateImpactDataPlane(-hydraPlaneTurn);
    //pSwing->UpdateImpactDataAxis(hydraAxisTilt);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 5
    pSwing->UpdateImpactDataPlane(-hydraPlaneTurn);
    //pSwing->UpdateImpactDataAxis(hydraAxisTilt);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 6
    //pSwing->UpdateImpactDataPlane(-hydraPlaneTurn);
    pSwing->UpdateImpactDataAxis(-hydraAxisTilt * 2);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 7
    pSwing->UpdateImpactDataPlane(hydraPlaneTurn);
    //pSwing->UpdateImpactDataAxis(-hydraAxisTilt * 2);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    // 8
    pSwing->UpdateImpactDataPlane(hydraPlaneTurn);
    //pSwing->UpdateImpactDataAxis(-hydraAxisTilt * 2);
    pSwing->ResetAlphaBeta();
    pBall->ResetBallData();
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
    m_hydraTimeSteps[i] = pBall->GetShotTimeSteps();
    m_hydraData[i] = m_shotPath;
    ++i;

    pSwing->UpdateImpactDataPlane(-hydraPlaneTurn);
    pSwing->UpdateImpactDataAxis(hydraAxisTilt);
   
}

void Golf::BuildTrajectoryData()
{
    pBall->FireProjectile(pSwing->CalculateLaunchVector());

    InputData();
    ScaleCordinates();
}

void Golf::BuildUIstrings()
{
    m_uiStrings.clear();
    std::stringstream inVal;
    inVal.precision(Utility::GetNumericalPrecisionForUI());
    inVal << std::fixed << pEnvironment->GetAirDensity(); 
    m_uiStrings.push_back("Air Density = " + inVal.str() + " kg/m cubed");
    inVal.str(std::string());
    inVal << std::fixed << pEnvironment->GetWindX();
    m_uiStrings.push_back("Wind X = " + inVal.str() + " m/s");
    inVal.str(std::string());
    inVal << std::fixed << pEnvironment->GetWindY();
    m_uiStrings.push_back("Wind Y = " + inVal.str() + " m/s");
    inVal.str(std::string());
    inVal << std::fixed << pEnvironment->GetWindZ();
    m_uiStrings.push_back("Wind Z = " + inVal.str() + " m/s");
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetBackSwingPercentage();
    m_uiStrings.push_back("Shot Power = " + inVal.str() + " percent");
    m_uiStrings.push_back("Club Name = " + pSwing->GetClubName());
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetClubLength();
    m_uiStrings.push_back("Club Length = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetClubMass();
    m_uiStrings.push_back("Club Mass = " + inVal.str() + " kg");
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetClubAngle();
    m_uiStrings.push_back("Club Angle = " + inVal.str() + " degrees");
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetLaunchAngle();
    m_uiStrings.push_back("Launch Angle = " + inVal.str() + " degrees");
    inVal.str(std::string());
    inVal << std::fixed << pSwing->GetLaunchVelocity();
    m_uiStrings.push_back("Launch Velocity = " + inVal.str() + " m/s");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetInitialSpinRate();
    m_uiStrings.push_back("Initial Spin Rate = " + inVal.str() + " rads per s");
    //inVal.str(std::string());
    //inVal << std::fixed << std::to_string(pBall->GetLandingSpinRate());
    //m_uiStrings.push_back("Landing Spin Rate = " + inVal.str() + " rads per s");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetShotDistance();
    m_uiStrings.push_back("Travel Distance = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetShotFlightDistance();
    m_uiStrings.push_back("Flight Distance = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetMaxHeight();
    m_uiStrings.push_back("Max Height = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetLandingHeight();
    //m_uiStrings.push_back("Landing Height = " + inVal.str() + " meters");
    //m_uiStrings.push_back("Bounce Count = " + std::to_string(pBall->GetBounceCount()));
    //m_uiStrings.push_back("Character Name = " + pCharacter->GetName(m_selectedCharacter));

    // For use as needed for debuging
    //m_uiStrings.push_back("Ball Debug 1 = " + std::to_string(pBall->GetDebugValue01()));
    //m_uiStrings.push_back("Ball Debug 2 = " + std::to_string(pBall->GetDebugValue02()));
    //m_uiStrings.push_back("Ball Debug 3 = " + std::to_string(pBall->GetDebugValue03()));
    //m_uiStrings.push_back("Ball Debug 4 = " + std::to_string(pBall->GetDebugValue04()));
}

void Golf::CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath)
{
    m_shotPath = aPath;
}

void Golf::CycleNextClub(const bool aIsCycleClubUp)
{
    pSwing->CycleClub(aIsCycleClubUp);
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pBall->ResetBallData();

    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildTrajectoryData();

    BuildUIstrings();
}

void Golf::InputData()
{
    m_shotPath.clear();
    CopyShotPath(pBall->OutputShotPath());
}

std::string Golf::GetCharacterArmBalancePoint(const int aCharacterIndex) const
{
    std::string armBalancePointString = "Arm Balance Point = " + pCharacter->GetArmBalancePointString(aCharacterIndex) + " %";
    return armBalancePointString;
}

std::string Golf::GetCharacterArmLength(const int aCharacterIndex) const
{
    std::string armLengthString = "Arm Length = " + pCharacter->GetArmLengthString(aCharacterIndex) + " m";
    return armLengthString;
}

std::string Golf::GetCharacterArmMass(const int aCharacterIndex) const
{
    std::string armMassString = "Arm Mass = " + pCharacter->GetArmMassString(aCharacterIndex) + " kg";
    return armMassString;
}

std::string Golf::GetCharacterClubLengthMod(const int aCharacterIndex) const
{
    std::string clubLengthModString = "Club Length = x" + pCharacter->GetClubLenghtModifierString(aCharacterIndex) + " m";
    return clubLengthModString;
}

std::string Golf::GetCharacterArmMassMoI(const int aCharacterIndex) const
{
    std::string armMassMoIString = "Arm MoI = " + pCharacter->GetArmMassMoIString(aCharacterIndex) + " kg Squared";
    return armMassMoIString;
}

std::string Golf::GetCharacterBioLine0(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine0(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine1(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine1(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine2(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine2(aCharacterIndex);
}

std::string Golf::GetCharacterBioLine3(const int aCharacterIndex) const
{
    return pCharacter->GetBioLine3(aCharacterIndex);
}

std::string Golf::GetCharacterName(const int aCharacterIndex) const
{
    return pCharacter->GetName(aCharacterIndex);
}

float Golf::GetDirectionToHoleInRads() const
{
    DirectX::SimpleMath::Vector3 start = m_shotStartPos;
    DirectX::SimpleMath::Vector3 end = pEnvironment->GetHolePosition();

    DirectX::SimpleMath::Vector3 vecToHole;
    vecToHole.x = end.x - start.x;
    vecToHole.y = end.y - start.y;
    vecToHole.z = end.z - start.z;

    DirectX::SimpleMath::Vector3 zeroDirection = DirectX::SimpleMath::Vector3(1.0, 0.0, 0.0);

    float direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(vecToHole), 
        DirectX::XMVector3Normalize(zeroDirection)));

    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(vecToHole, zeroDirection)) > 0.0f)
    {
        direction = -direction;
    }

    return direction;
}

void Golf::LoadCharacterTraits()
{
    pSwing->SetArmBalancePoint(pCharacter->GetArmBalancePoint(m_selectedCharacter));
    pSwing->SetArmLength(pCharacter->GetArmLength(m_selectedCharacter));
    pSwing->SetArmMass(pCharacter->GetArmMass(m_selectedCharacter));
    pSwing->SetArmMassMoI(pCharacter->GetArmMassMoI(m_selectedCharacter));
    pSwing->SetClubLengthModifier(pCharacter->GetClubLenghtModifier(m_selectedCharacter));
    pSwing->UpdateGolfer();
}

void Golf::LoadEnvironment(const int aIndex)
{
    if (aIndex < 0 || aIndex > pEnvironment->GetNumerOfEnvirons() - 1)
    {
        m_selectedEnvironment = 0;
        pEnvironment->UpdateEnvironment(0);
        pSwing->UpdateGravityDependants(pEnvironment->GetGravity());
        pBall->SetDefaultBallValues(pEnvironment);
        BuildUIstrings();
        m_shotStartPos = pEnvironment->GetTeePosition();
    }
    else
    {
        m_selectedEnvironment = aIndex;
        pEnvironment->UpdateEnvironment(m_selectedEnvironment);
        pSwing->UpdateGravityDependants(pEnvironment->GetGravity());
        pBall->SetDefaultBallValues(pEnvironment);
        BuildUIstrings();
        m_shotStartPos = pEnvironment->GetTeePosition();
    }
}

void Golf::ScaleCordinates()
{
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::Identity;

    float scaleFactor = pEnvironment->GetScale();
    float sX = scaleFactor;
    float sY = scaleFactor;
    float sZ = scaleFactor;
    scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(sX, sY, sZ);

    for (int i = 0; i < m_shotPath.size(); ++i)
    {
        m_shotPath[i] = DirectX::SimpleMath::Vector3::Transform(m_shotPath[i], scaleMatrix);
        TransformCordinates(i);
    }  
}

void Golf::SelectInputClub(int aInput)
{
    pBall->ResetBallData();
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());

    pSwing->InputClub(aInput);
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildTrajectoryData();
    BuildUIstrings();
}

void Golf::SetBallPosition(const DirectX::SimpleMath::Vector3 aBallPos)
{
    m_ballPos = aBallPos;
}

void Golf::SetCharacter(const int aCharacterIndex)
{
    if (aCharacterIndex < 0 || aCharacterIndex > pCharacter->GetCharacterCount() - 1)
    {
        m_selectedCharacter = 0;
    }
    else
    {
        m_selectedCharacter = aCharacterIndex;
    }
    LoadCharacterTraits();
}

void Golf::SetEnvironment(const int aEnvironmentIndex)
{
    if (aEnvironmentIndex < 0 || aEnvironmentIndex > pEnvironment->GetNumerOfEnvirons() - 1)
    {
        m_selectedEnvironment = 0;
    }
    else
    {
        m_selectedEnvironment = aEnvironmentIndex;
    }
    LoadEnvironment(aEnvironmentIndex);
}

void Golf::SetShotCordMax()
{
    double maxX = 0.0;
    double maxY = 0.0;
    double maxZ = 0.0;
    for (int i = 0; i < m_shotPath.size(); ++i)
    {
        if (maxX < m_shotPath[i].x)
        {
            maxX = m_shotPath[i].x;
        }
        if (maxY < m_shotPath[i].y)
        {
            maxY = m_shotPath[i].y;
        }
        if (maxZ < m_shotPath[i].z)
        {
            maxZ = m_shotPath[i].z;
        }
    }
    m_maxX = maxX;
    m_maxY = maxY;
    m_maxZ = maxZ;
}

void Golf::SetShotStartPos(const DirectX::SimpleMath::Vector3 aShotStartPos)
{
    m_shotStartPos = aShotStartPos;
}

//Transform shotpath to start at tee position or end position of last shot
void Golf::TransformCordinates(const int aIndex)
{
    m_shotPath[aIndex] += m_shotStartPos;
}

void Golf::UpdateImpact(Utility::ImpactData aImpact)
{
    pSwing->UpdateImpactData(aImpact);
    pBall->ResetBallData();
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildTrajectoryData();
    BuildUIstrings();
}

void Golf::ZeroUIandRenderData()
{
    m_shotPath.clear();
    pBall->ResetBallData();
    pSwing->ResetAlphaBeta();
    pSwing->ZeroDataForUI();
    pBall->ZeroDataForUI();
}
