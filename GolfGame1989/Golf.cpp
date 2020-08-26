#include "pch.h"
#include "Golf.h"
#include <fstream>
#include <sstream>

Golf::Golf()
{
    //BuildVector();
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
    //BuildVector(); // WLJ turn off to disable auto draw of projectile without powerbar input 
    BuildUIstrings();
    BuildEnvironSelectStrings();
}

Golf::~Golf()
{
    delete pBall;
    delete pSwing;
    delete pEnvironment;
    delete pCharacter;
    delete pPlay;
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
    inVal.str(std::string());
    inVal << std::fixed << std::to_string(pBall->GetLandingSpinRate());
    //m_uiStrings.push_back("Landing Spin Rate = " + inVal.str() + " rads per s");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetShotDistance();
    m_uiStrings.push_back("Travel Distance = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetMaxHeight();
    m_uiStrings.push_back("Max Height = " + inVal.str() + " meters");
    inVal.str(std::string());
    inVal << std::fixed << pBall->GetLandingHeight();
    //m_uiStrings.push_back("Landing Height = " + inVal.str() + " meters");
    //m_uiStrings.push_back("Bounce Count = " + std::to_string(pBall->GetBounceCount()));
    //m_uiStrings.push_back("Character Name = " + pCharacter->GetName(m_selectedCharacter));
}

void Golf::BuildVector()
{
    pBall->FireProjectile(pSwing->CalculateLaunchVector());
    InputData();
    ScaleCordinates();
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

void Golf::CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath)
{
    m_shotPath = aPath;
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
    }
    else
    {
        m_selectedEnvironment = aIndex;
        pEnvironment->UpdateEnvironment(m_selectedEnvironment);
        pSwing->UpdateGravityDependants(pEnvironment->GetGravity());
        pBall->SetDefaultBallValues(pEnvironment);
        BuildUIstrings();
    }
}

void Golf::ScaleCordinates()
{
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::Identity;

    float scaleFactor = .02;
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

/*
void Golf::SelectNextClub()
{
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pBall->ResetBallData();
    pSwing->CycleClub();
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();

    BuildUIstrings();
}
*/

void Golf::SelectInputClub(int aInput)
{

    pBall->ResetBallData();
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());

    pSwing->InputClub(aInput);
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();
    BuildUIstrings();
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

//Transform shotpath to start at edge of world grid
void Golf::TransformCordinates(const int aIndex)
{
    m_shotPath[aIndex].x -= 2;
}

void Golf::UpdateImpact(Utility::ImpactData aImpact)
{
    pSwing->UpdateImpactData(aImpact);
    pBall->ResetBallData();
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();
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
