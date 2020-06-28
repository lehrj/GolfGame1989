#include "pch.h"
#include "Golf.h"
#include <fstream>

Golf::Golf()
{
    //BuildVector();
    pEnvironment = new Environment();
    pEnvironment->SetDefaultEnvironment();
    pSwing = new GolfSwing();
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    
    pSwing->UpdateGolfSwingValues();
    pBall = new GolfBall();
    pBall->SetDefaultBallValues(pEnvironment);
    pPlay = new GolfPlay();
    BuildVector();
    BuildUIstrings();
}

Golf::~Golf()
{
    delete pBall;
    delete pSwing;
    delete pEnvironment;
    delete pPlay;
}

void Golf::BuildVector()
{
    pBall->FireProjectile(pSwing->CalculateLaunchVector(), pEnvironment);
    InputData();
    ScaleCordinates();
}

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

void Golf::SelectInputClub(int aInput)
{
    pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pBall->ResetBallData();
    pSwing->CycleInputClub(aInput);
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();
    BuildUIstrings();
}

void Golf::BuildUIstrings()
{
    m_uiStrings.clear();
    m_uiStrings.push_back("Air Density = " + std::to_string(pEnvironment->GetAirDensity()) + " kg/m cubed"); // WLJ need to find sweet font that supports "^"
    m_uiStrings.push_back("Wind X = " + std::to_string(pEnvironment->GetWindX()) + " m/s");
    m_uiStrings.push_back("Wind Y = " + std::to_string(pEnvironment->GetWindY()) + " m/s");
    m_uiStrings.push_back("Wind Z = " + std::to_string(pEnvironment->GetWindZ()) + " m/s");
    m_uiStrings.push_back("Shot Power = " + std::to_string(pSwing->GetBackSwingPercentage()) + " percent");
    m_uiStrings.push_back("Club Name = " + pSwing->GetClubName());
    m_uiStrings.push_back("Club Angle = " + std::to_string(pSwing->GetClubAngle()) + " degrees");
    m_uiStrings.push_back("Club Length = " + std::to_string(pSwing->GetClubLength()) + " meters");
    m_uiStrings.push_back("Club Mass = " + std::to_string(pSwing->GetClubMass()) + " kg");
    m_uiStrings.push_back("Launch Angle = " + std::to_string(pSwing->GetLaunchAngle()) + " degrees");
    m_uiStrings.push_back("Launch Velocity = " + std::to_string(pSwing->GetLaunchVelocity()) + " m/s");
    m_uiStrings.push_back("Initial Spin Rate = " + std::to_string(pBall->GetInitialSpinRate()) + " m/s");
    m_uiStrings.push_back("Landing Spin Rate = " + std::to_string(pBall->GetLandingSpinRate()) + " m/s");
    m_uiStrings.push_back("Travel Distance = " + std::to_string(pBall->GetShotDistance()) + " meters");
    m_uiStrings.push_back("Max Height = " + std::to_string(pBall->GetMaxHeight()) + " meters");
    m_uiStrings.push_back("Landing Height = " + std::to_string(pBall->GetLandingHeight()) + " meters");
    m_uiStrings.push_back("Bounce Count = " + std::to_string(pBall->GetBounceCount()));
}

void Golf::InputData()
{
    m_shotPath.clear();
    CopyShotPath(pBall->OutputShotPath());
}

int Golf::GetDrawColorIndex()
{
    return pBall->GetColorIndex();
}

std::vector<int> Golf::GetDrawColorVector()
{
    return pBall->GetColorVector();
}

//Transform shotpath to start at edge of world grid
void Golf::TransformCordinates(const int aIndex)
{
    m_shotPath[aIndex].x -= 2;
}

void Golf::ScaleCordinates()
{
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::Identity;
    double scaleFactor = .02;
    double sX = scaleFactor;
    double sY = scaleFactor;
    double sZ = scaleFactor;
    scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(sX, sY, sZ);

    for (int i = 0; i < m_shotPath.size(); ++i)
    {
        m_shotPath[i] = DirectX::SimpleMath::Vector3::Transform(m_shotPath[i], scaleMatrix);
        TransformCordinates(i);
    }  
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

void Golf::CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath)
{
    m_shotPath = aPath;
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