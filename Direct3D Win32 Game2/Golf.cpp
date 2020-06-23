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
    CalculateData();
    InputData();
    //NormalizeData();
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
}

void Golf::CalculateData()
{
    //Vector4d swingVector = pSwing->CalculateLaunchVector();
    //pBall->FireProjectile(swingVector, pEnvironment);
    pBall->FireProjectile2(pSwing->CalculateLaunchVector2(), pEnvironment);
}

/*
std::vector<double> Golf::GetVect(const int aInput)
{
    if (aInput == 0)
    {
        std::vector<double> xNorm;
        xNorm.clear();
        for (int i = 0; i < m_shotPathNorm.size(); ++i)
        {
            xNorm.push_back(m_shotPathNorm[i].x);
        }
        return xNorm;
    }
    if(aInput == 1)
    {
        std::vector<double> yNorm;
        yNorm.clear();
        for (int i = 0; i < m_shotPathNorm.size(); ++i)
        {
            yNorm.push_back(m_shotPathNorm[i].y);
        }
        return yNorm;
    }
    if (aInput == 2)
    {
        std::vector<double> zNorm;
        zNorm.clear();
        for (int i = 0; i < m_shotPathNorm.size(); ++i)
        {
            zNorm.push_back(m_shotPathNorm[i].z);
        }
        return zNorm;
    }
}
*/

void Golf::InputData()
{
    m_shotPathRaw.clear();
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
Vector4d Golf::GetLaunchVector()
{
    Vector4d launchVector(pSwing->GetArmLength(), pSwing->GetClubLength(), pSwing->GetLaunchAngle(), pSwing->GetLaunchVelocity());
    return launchVector;
}

std::vector<Vector4d> Golf::GetSwingData()
{
    return pSwing->OutputSwingData();
}

void Golf::TransformCordinates()
{
    //m_xNorm.clear();
    //m_yNorm.clear();
    //m_zNorm.clear();
    DirectX::SimpleMath::Vector3 oldVec;
    DirectX::SimpleMath::Vector3 newVec;
    double sX;
    double sY;
    double sZ;

    //DirectX::SimpleMath::Matrix scaleMatrix = 
    DirectX::SimpleMath::Matrix transMatrix = DirectX::SimpleMath::Matrix::Identity;
    transMatrix._14 = 2.0f;
    transMatrix._24 = 2.0f;
    transMatrix._34 = 2.0f;
    for (int i = 0; i < m_shotPathNorm.size(); ++i)
    {
        m_shotPathNorm[i].x -= 2;
    }
}

void Golf::ScaleCordinates()
{
    m_shotPathNorm.clear();
    DirectX::SimpleMath::Vector3 oldVec;
    DirectX::SimpleMath::Vector3 newVec;
    double scaleFactor = .02;
    double sX = scaleFactor;
    double sY = scaleFactor;
    double sZ = scaleFactor;

    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::Identity;
    /*
    scaleMatrix._11 = sX;
    scaleMatrix._22 = sY;
    scaleMatrix._33 = sZ;
    */
    scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(sX, sY, sZ);

    for (int i = 0; i < m_shotPathRaw.size(); ++i)
    {
        oldVec = m_shotPathRaw[i];

        //oldVec *= transMatrix;
        //newVec = oldVec * transMatrix;

        //DirectX::SimpleMath::Vector4 aTestVec = DirectX::SimpleMath::Vector4::Transform(oldVec, transMatrix);
        newVec = DirectX::SimpleMath::Vector3::Transform(oldVec, scaleMatrix);
        //m_xNorm[i] = newVec.x;
        //m_yNorm[i] = newVec.y;
        //m_zNorm[i] = newVec.z;
        m_shotPathNorm.push_back(newVec);
    }
    TransformCordinates();
}

void Golf::NormalizeData()
{
    SetMaxX();
    SetMaxY();
    SetMaxZ();
    //m_xWindow = m_maxX + 10; // WLJ need to adjust how this is done 
    //m_yWindow = m_maxY + 10;

    m_xWindow = m_maxX + 10; // WLJ need to adjust how this is done 
    m_yWindow = m_maxY + 10;
    m_zWindow = m_maxZ + 10;
    
    m_shotPathNorm.clear();
    for (int i = 0; i < m_shotPathRaw.size(); ++i)
    {
        double valX = (((m_shotPathRaw[i].x / m_xWindow)));
        double valY = (((m_shotPathRaw[i].y / m_yWindow)));
        double valZ = (((m_shotPathRaw[i].z / m_zWindow)));
        DirectX::SimpleMath::Vector3 val(valX, valY, valZ);
        m_shotPathNorm.push_back(val);
    }
    /*
    //m_xNorm.resize(m_xVals.size());
    m_xNorm.clear();
    for (int i = 0; i < m_xVals.size(); ++i)
    {
        double val = (((m_xVals[i] / m_xWindow)));
        //double val = (((m_xVals[i] / m_xWindow) * 2) - 1.5);
        //double val = (((m_xVals[i] / m_xWindow) * 2) - 1);
        //double val = (((m_xVals[i] / m_xWindow))-1);
        //m_xNorm[i] = val;
        m_xNorm.push_back(val);
    }

    //m_yNorm.resize(m_yVals.size());
    m_yNorm.clear();
    for (int i = 0; i < m_yVals.size(); ++i)
    {
        //double val = (((m_yVals[i] / m_yWindow) * 2) - 1);
        double val = (((m_yVals[i] / m_yWindow)));
        //m_yNorm[i] = val;
        m_yNorm.push_back(val);
    }

    //m_zNorm.resize(m_zVals.size());
    m_zNorm.clear();
    for (int i = 0; i < m_zVals.size(); ++i)
    {
        double val = (((m_zVals[i] / m_zWindow)));
        //double val = m_zVals[i];
        //m_zNorm[i] = val;
        m_zNorm.push_back(val);
    }
    */
}

void Golf::SetMaxX()
{
    double maxX = 0.0;
    for (int i = 0; i < m_shotPathRaw.size(); ++i)
    {
        if (maxX < m_shotPathRaw[i].x)
        {
            maxX = m_shotPathRaw[i].x;
        }
    }
    m_maxX = maxX;
}

void Golf::SetMaxY()
{
    double maxY = 0.0;
    for (int i = 0; i < m_shotPathRaw.size(); ++i)
    {
        if (maxY < m_shotPathRaw[i].y)
        {
            maxY = m_shotPathRaw[i].y;
        }
    }
    m_maxY = maxY;
}

void Golf::SetMaxZ()
{
    double maxZ = 0.0;
    for (int i = 0; i < m_shotPathRaw.size(); ++i)
    {
        if (maxZ < m_shotPathRaw[i].z)
        {
            maxZ = m_shotPathRaw[i].z;
        }
    }
    m_maxZ = maxZ;
}

/*
void Golf::CopyXvec(std::vector<double> aVec)
{
    
    std::vector<double> temp(aVec);
    for (int i = 0; i < temp.size(); ++i)
    {
        m_xVals.push_back(temp[i]);
    }
    
}

void Golf::CopyYvec(std::vector<double> aVec)
{
    std::vector<double> temp(aVec);
    for (int i = 0; i < temp.size(); ++i)
    {
        m_yVals.push_back(temp[i]);
    }
}

void Golf::CopyZvec(std::vector<double> aVec)
{
    std::vector<double> temp(aVec);
    for (int i = 0; i < temp.size(); ++i)
    {
        m_zVals.push_back(temp[i]);
    }
}
*/

void Golf::CopyShotPath(std::vector<DirectX::SimpleMath::Vector3> aPath)
{
    m_shotPathRaw = aPath;
}

void Golf::UpdateImpact(Utility::ImpactData aImpact)
{
    pSwing->UpdateImpactData(aImpact);
    //pSwing->UpdateBackSwing(aImpact.power);
    //BuildVector();

    //pSwing->SetDefaultSwingValues(pEnvironment->GetGravity());
    pBall->ResetBallData();
    //pSwing->CycleInputClub(aInput);
    pSwing->ResetAlphaBeta();
    pSwing->UpdateGolfSwingValues();
    BuildVector();

    BuildUIstrings();
}