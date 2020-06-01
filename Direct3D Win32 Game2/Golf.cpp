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
    BuildVector();

    BuildUIstrings();
}

Golf::~Golf()
{
    delete pBall;
    delete pSwing;
    delete pEnvironment;

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
}

void Golf::CalculateData()
{
    Vector4d swingVector = pSwing->CalculateLaunchVector();
    pBall->FireProjectile(swingVector, pEnvironment);
}

std::vector<double> Golf::GetVect(const int aInput)
{
    if (aInput == 0)
    {
        //return m_xVals;
        return m_xNorm;
    }
    if(aInput == 1)
    {
        //return m_yVals;
        return m_yNorm;
    }
    if (aInput == 2)
    {
        return m_zNorm;
    }
}

void Golf::InputData()
{
    
    m_xVals.clear();
    m_yVals.clear();
    m_zVals.clear();
    
    
    CopyXvec(pBall->OutputXvals());
    CopyYvec(pBall->OutputYvals());
    CopyZvec(pBall->OutputZvals());
}

/*
Vector4d Golf::GetLaunchVector()
{
    Vector4d launchVector(pSwing->GetArmLength(), pSwing->GetClubLength(), pSwing->GetLaunchAngle(), pSwing->GetLaunchVelocity());
    return launchVector;
}
*/

std::vector<Vector4d> Golf::GetSwingData()
{
    return pSwing->OutputSwingData();
}

void Golf::TransformCordinates()
{
    //m_xNorm.clear();
    //m_yNorm.clear();
    //m_zNorm.clear();
    DirectX::SimpleMath::Vector4 oldVec;
    DirectX::SimpleMath::Vector4 newVec;
    double sX;
    double sY;
    double sZ;

    //DirectX::SimpleMath::Matrix scaleMatrix = 
    DirectX::SimpleMath::Matrix transMatrix = DirectX::SimpleMath::Matrix::Identity;
    transMatrix._14 = 2.0f;
    transMatrix._24 = 2.0f;
    transMatrix._34 = 2.0f;
    for (int i = 0; i < m_xNorm.size(); ++i)
    {
        /*
        oldVec.x = m_xNorm[i];
        oldVec.y = m_yNorm[i];
        oldVec.z = m_zNorm[i];
        oldVec.w = 1;

        //oldVec *= transMatrix;
        //newVec = oldVec * transMatrix;

        //DirectX::SimpleMath::Vector4 aTestVec = DirectX::SimpleMath::Vector4::Transform(oldVec, transMatrix);
        newVec = DirectX::SimpleMath::Vector4::Transform(oldVec, transMatrix);
        m_xNorm[i] = newVec.x;
        m_yNorm[i] = newVec.y;
        m_zNorm[i] = newVec.z;
        //m_xNorm.push_back(newVec.x);
        //m_yNorm.push_back(newVec.y);
        //m_zNorm.push_back(newVec.z);
        */
        m_xNorm[i] -= 2;
    }
}

void Golf::ScaleCordinates()
{
    m_xNorm.clear();
    m_yNorm.clear();
    m_zNorm.clear();
    DirectX::SimpleMath::Vector4 oldVec;
    DirectX::SimpleMath::Vector4 newVec;
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


    for (int i = 0; i < m_xVals.size(); ++i)
    {
        oldVec.x = m_xVals[i];
        oldVec.y = m_yVals[i];
        oldVec.z = m_zVals[i];
        oldVec.w = 1;

        //oldVec *= transMatrix;
        //newVec = oldVec * transMatrix;

        //DirectX::SimpleMath::Vector4 aTestVec = DirectX::SimpleMath::Vector4::Transform(oldVec, transMatrix);
        newVec = DirectX::SimpleMath::Vector4::Transform(oldVec, scaleMatrix);
        //m_xNorm[i] = newVec.x;
        //m_yNorm[i] = newVec.y;
        //m_zNorm[i] = newVec.z;
        m_xNorm.push_back(newVec.x);
        m_yNorm.push_back(newVec.y);
        m_zNorm.push_back(newVec.z);
    }
    int testint = 0;
    testint++;
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
    
    //ScaleCordinates();
    //m_xWindow = m_maxX + 10; // WLJ need to adjust how this is done 
//m_yWindow = m_maxY + 10;

    /*
        SetMaxX();
    SetMaxY();
    SetMaxZ();
    //m_xWindow = m_maxX + 10; // WLJ need to adjust how this is done 
    //m_yWindow = m_maxY + 10;

    
    m_xWindow = m_maxX + 10; // WLJ need to adjust how this is done 
    m_yWindow = m_maxY + 10;
    m_zWindow = m_maxZ + 10;
    
    //m_xNorm.resize(m_xVals.size());
    m_xNorm.clear();
    for (int i = 0; i < m_xVals.size(); ++i)
    {
        double val = (((m_xVals[i] / m_xWindow) * 2) - 1.5);
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
    for (int i = 0; i < m_xVals.size(); ++i)
    {
        if (maxX < m_xVals[i])
        {
            maxX = m_xVals[i];
        }
    }
    m_maxX = maxX;
}

void Golf::SetMaxY()
{
    double maxY = 0.0;
    for (int i = 0; i < m_yVals.size(); ++i)
    {
        if (maxY < m_yVals[i])
        {
            maxY = m_yVals[i];
        }
    }
    m_maxY = maxY;
}

void Golf::SetMaxZ()
{
    double maxZ = 0.0;
    for (int i = 0; i < m_zVals.size(); ++i)
    {
        if (maxZ < m_zVals[i])
        {
            maxZ = m_zVals[i];
        }
    }
    m_maxZ = maxZ;
}

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