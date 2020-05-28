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
    BuildUIdata();
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
    NormalizeData();
}


void Golf::BuildUIdata()
{
    m_uiData.push_back(pEnvironment->GetAirDensity());
    m_uiData.push_back(pEnvironment->GetWindZ());
    m_uiData.push_back(pSwing->GetClubAngle());
    m_uiData.push_back(pSwing->GetClubLength());
    m_uiData.push_back(pSwing->GetClubMass());
    m_uiData.push_back(pSwing->GetLaunchAngle());
    m_uiData.push_back(pSwing->GetLaunchVelocity());
    m_uiData.push_back(69);
    m_uiData.push_back(69);
}


void Golf::BuildUIstrings()
{
    m_uiStrings.clear();
    m_uiStrings.push_back("Air Density = " + std::to_string(pEnvironment->GetAirDensity()) + " kg/m^3");
    m_uiStrings.push_back("Wind Z = " + std::to_string(pEnvironment->GetWindZ()) + " m/s");
    m_uiStrings.push_back("Club Angle = " + std::to_string(pSwing->GetClubAngle()) + " degrees");
    m_uiStrings.push_back("Club Length = " + std::to_string(pSwing->GetClubLength()) + " meters");
    m_uiStrings.push_back("Club Mass = " + std::to_string(pSwing->GetClubMass()) + " meters");
    m_uiStrings.push_back("Launch Angle = " + std::to_string(pSwing->GetLaunchAngle()) + " degrees");
    m_uiStrings.push_back("Launch Velocity = " + std::to_string(pSwing->GetLaunchVelocity()) + " m/s");
    m_uiStrings.push_back("Travel Distance = xx.x meters");
    m_uiStrings.push_back("Max Height = xx.x meters");
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
    /*
    m_xVals = pBall->OutputXvals();
    m_yVals = pBall->OutputYvals();
    */
    /*
    int xSize = pBall->GetXvecSize();
    for (int i = 0; i < xSize; ++i)
    {
        m_xVals.push_back(pBall->GetIndexX(i));
    }

    int ySize = pBall->GetYvecSize();
    for (int i = 0; i < ySize; ++i)
    {
        m_yVals.push_back(pBall->GetIndexY(i));
    }
    */

    CopyXvec(pBall->OutputXvals());
    CopyYvec(pBall->OutputYvals());
    CopyZvec(pBall->OutputZvals());
    int temp;
    /*
    int zSize = pBall->GetZvecSize();
    for (int i = 0; i < zSize; ++i)
    {
        m_zVals.push_back(pBall->GetIndexZ(i));
    }
    */
    /*
    double input;
    std::ifstream inFileX("xVals.txt");
    if (inFileX.is_open())
    {
        //while(std::getline(inFile, input))
        while(inFileX >> input)
        {
            m_xVals.push_back(input);
        }
    }
    inFileX.close();

    std::ifstream inFileY("yVals.txt");
    if (inFileY.is_open())
    {
        while (inFileY >> input)
        {
            m_yVals.push_back(input);
        }
    }
    inFileY.close();
    */
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

    m_xNorm.resize(m_xVals.size());
    for (int i = 0; i < m_xVals.size(); ++i)
    {
        double val = (((m_xVals[i] / m_xWindow) * 2) - 1);
        //double val = (((m_xVals[i] / m_xWindow) * 2) - 1);
        //double val = (((m_xVals[i] / m_xWindow))-1);
        m_xNorm[i] = val;
    }

    m_yNorm.resize(m_yVals.size());
    for (int i = 0; i < m_yVals.size(); ++i)
    {
        //double val = (((m_yVals[i] / m_yWindow) * 2) - 1);
        double val = (((m_yVals[i] / m_yWindow)));
        m_yNorm[i] = val;
    }

    m_zNorm.resize(m_zVals.size());
    for (int i = 0; i < m_zVals.size(); ++i)
    {
        double val = (((m_zVals[i] / m_zWindow)));
        //double val = m_zVals[i];
        m_zNorm[i] = val;
    }
   
    int test;

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