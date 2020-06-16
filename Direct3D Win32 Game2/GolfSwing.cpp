#include "pch.h"
#include "GolfSwing.h"
#include <math.h>
#include "Vector4d.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


GolfSwing::GolfSwing()
{
    //this->SetDefaultSwingValues(9.8);
    m_pBag = new GolfBag();
    //Utility::ZeroImpactData(m_impactData);
}

GolfSwing::~GolfSwing()
{
    delete m_pBag;
}

void GolfSwing::CalculateSwingCordinates()
{
    DirectX::SimpleMath::Vector3 alpha;
    DirectX::SimpleMath::Vector3 prevAlpha;
    double alphaAngle = m_alphaBetaThetaVec[0].GetX();
    DirectX::SimpleMath::Vector3 beta;
    DirectX::SimpleMath::Vector3 prevBeta;
    double betaAngle = m_alphaBetaThetaVec[0].GetY();;
    DirectX::SimpleMath::Vector3 theta;
    DirectX::SimpleMath::Vector3 prevTheta;
    double thetaAngle = m_alphaBetaThetaVec[0].GetZ();
    std::vector< DirectX::SimpleMath::Vector3> alphaVec;
    std::vector< DirectX::SimpleMath::Vector3> betaVec;
    std::vector< DirectX::SimpleMath::Vector3> thetaVec;
    double armNorm = 1.0;
    double clubNorm = 1.0;
    DirectX::SimpleMath::Vector3 body(0.0, 1.0, 0.0);
    DirectX::SimpleMath::Vector3 arm;
    DirectX::SimpleMath::Vector4 arm4(0.0, 1.0, 0.0, 0.0);
    DirectX::SimpleMath::Vector3 club;
    DirectX::SimpleMath::Vector4 testV4(0.0, 1.0, 0.0, 1.0);
    DirectX::SimpleMath::Vector4 testV42(0.0, 1.0, 0.0, 1.0);
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle);

    DirectX::SimpleMath::Matrix testMat;
    //testV4.Transform(DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle));

    //testV42 = DirectX::SimpleMath::Matrix::Ro
    //club = DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle);
    DirectX::SimpleMath::Matrix rotMatTest = DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle);
    testV4.Transform(arm4, rotMatTest);
    DirectX::SimpleMath::Vector4 a4Vec = DirectX::SimpleMath::Vector4::Transform(arm4, rotMatTest);
    //DirectX::SimpleMath::Vector3 a4Vec = DirectX::SimpleMath::Vector3::Transform(arm, rotMatTest);
    DirectX::SimpleMath::Vector4 a4Vec2 = a4Vec;
    a4Vec2.Normalize();

    int bStop = 0;
    bStop++;


}

std::vector<DirectX::SimpleMath::Vector3> GolfSwing::GetAlphaCords()
{
    m_alphaCord.clear();
    double alphaAngle = m_alphaBetaThetaVec[0].GetX();
    double prevAlphaAngle = m_alphaBetaThetaVec[0].GetX();
    DirectX::SimpleMath::Vector3 alphaXYZ(0.0, 1.0, 0.0);
    DirectX::SimpleMath::Vector3 arm(0.0, 1.0, 0.0);
    m_alphaCord.push_back(alphaXYZ);
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(alphaAngle);

    for (int i = 0; i < m_alphaBetaThetaVec.size(); ++i)
    {
        alphaAngle = m_alphaBetaThetaVec[i].GetX();
        rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(alphaAngle - prevAlphaAngle);
        alphaXYZ = DirectX::SimpleMath::Vector3::Transform(alphaXYZ, rotMat);
        m_alphaCord.push_back(alphaXYZ);
        prevAlphaAngle = alphaAngle;
    }
    return m_alphaCord;
}

std::vector<DirectX::SimpleMath::Vector3> GolfSwing::GetBetaCords()
{
    m_betaCord.clear();
    double betaAngle = m_alphaBetaThetaVec[0].GetY();
    double prevBetaAngle = m_alphaBetaThetaVec[0].GetY();
    DirectX::SimpleMath::Vector3 betaXYZ(0.0, 1.0, 0.0);
    DirectX::SimpleMath::Vector3 club(0.0, 1.0, 0.0);
    m_betaCord.push_back(betaXYZ);
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(betaAngle);

    for (int i = 0; i < m_alphaBetaThetaVec.size(); ++i)
    {
        betaAngle = m_alphaBetaThetaVec[i].GetY();
        rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(betaAngle - prevBetaAngle);
        betaXYZ = DirectX::SimpleMath::Vector3::Transform(betaXYZ, rotMat);
        m_betaCord.push_back(betaXYZ);
        prevBetaAngle = betaAngle;
    }
    return m_alphaCord;
}

std::vector<DirectX::SimpleMath::Vector3> GolfSwing::GetThetaCords()
{
    m_thetaCord.clear();
    double thetaAngle = m_alphaBetaThetaVec[0].GetZ();
    double prevThetaAngle = m_alphaBetaThetaVec[0].GetZ();
    DirectX::SimpleMath::Vector3 thetaXYZ(0.0, 1.0, 0.0);
    DirectX::SimpleMath::Vector3 body(0.0, 1.0, 0.0);
    m_thetaCord.push_back(thetaXYZ);
    DirectX::SimpleMath::Matrix rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle);

    for (int i = 0; i < m_alphaBetaThetaVec.size(); ++i)
    {
        thetaAngle = m_alphaBetaThetaVec[i].GetZ();
        rotMat = DirectX::SimpleMath::Matrix::CreateRotationZ(thetaAngle - prevThetaAngle);
        thetaXYZ = DirectX::SimpleMath::Vector3::Transform(thetaXYZ, rotMat);
        m_thetaCord.push_back(thetaXYZ);
        prevThetaAngle = thetaAngle;
    }
    return m_thetaCord;
}

Vector4d GolfSwing::CalculateLaunchVector(void)
{
    m_alphaBetaThetaVec.clear();
    m_launchAngle = 0.0;
    m_launchVelocity = 0.0;

    double Vc = 0.0;
    double time = 0.0;
    double dt = 0.0025; // Time delta between frames in seconds
    double    a, at; // stores previous time steps results for alpha and its first derivative
    double    b, bt; // stores previous time steps results for beta and its first derivative

    double    phi; // stores value of theta + beta
    double    Vc2; // square of club head velocity 
    double  ak1, ak2, ak3, ak4; // stores intermediate results of Runge-Kutta integration scheme
    double  bk1, bk2, bk3, bk4; // stores intermediate results of Runge-Kutta integration scheme
    double velocityCapture = 0;
    double launchAngle = 0.0;
    bool isVcFound = false;

    //PrintSwingMechanics(Vc, time);

    //for (int i = 0; i < 200; i++)
    for (int i = 0; i < m_swingStepIncrementCount; i++)
    {
        time += dt;
        if (time >= 0.1)
        {
            m_Qbeta = 0;
        }
        // save results of previous time step
        a = m_alpha;
        b = m_beta;
        at = m_alpha_dot;
        bt = m_beta_dot;
        // integrate alpha'' and beta''

        // The K1 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak1 = m_alpha_dotdot * dt;
        bk1 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak1 / 2;
        m_beta_dot = bt + bk1 / 2;
        // The K2 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak2 = m_alpha_dotdot * dt;
        bk2 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak2 / 2;
        m_beta_dot = bt + bk2 / 2;
        // The K3 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak3 = m_alpha_dotdot * dt;
        bk3 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak3;
        m_beta_dot = bt + bk3;
        // The K3 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak4 = m_alpha_dotdot * dt;
        bk4 = m_beta_dotdot * dt;
        m_alpha_dot = at + (ak1 + 2 * ak2 + 2 * ak3 + ak4) / 6;
        m_beta_dot = bt + (bk1 + 2 * bk2 + 2 * bk3 + bk4) / 6;
        m_alpha = a + m_alpha_dot * dt;
        m_beta = b + m_beta_dot * dt;
        m_theta = m_gamma - m_alpha;

        Vc2 = (m_armLength * m_armLength + m_club.length * m_club.length + 2 * m_armLength * m_club.length * cos(m_beta))
            * (m_alpha_dot * m_alpha_dot) + m_club.length * m_club.length * m_beta_dot * m_beta_dot
            - 2 * (m_club.length * m_club.length + m_armLength * m_club.length * cos(m_beta)) * m_alpha_dot * m_beta_dot;  // Jorgensen equation

        Vc = sqrt(Vc2);

        Vector4d swingAngles(m_alpha, m_beta, m_theta, 0.0);
        m_alphaBetaThetaVec.push_back(swingAngles);

        phi = m_theta + m_beta;
        if (Utility::ToDegrees(phi) < m_ballPlacementAngle)
        {
            if (isVcFound == false)
            {
                //PrintSwingMechanics(Vc, time);

                velocityCapture = Vc;
                isVcFound = true;
                launchAngle = m_club.angle - Utility::ToDegrees(phi);
                double test;
                test = launchAngle;
            }
        }
    }

    m_launchAngle = launchAngle;
    m_launchVelocity = velocityCapture;

    std::cout << "Capture velocity = " << velocityCapture << "\nLaunch angle = " << launchAngle << std::endl;
    
    Vector4d launchVector{ m_launchVelocity, m_launchAngle, m_club.mass, m_club.coefficiantOfRestitution };
    Utility::ImpactData launchImpact{};
    Utility::ZeroImpactData(launchImpact);
    launchImpact.velocity = m_launchVelocity;
    launchImpact.angleY = m_launchAngle;
    launchImpact.mass = m_club.mass;
    launchImpact.cor = m_club.coefficiantOfRestitution;

    return launchVector;
}

Utility::ImpactData GolfSwing::CalculateLaunchVector2()
{
    m_alphaBetaThetaVec.clear();
    m_launchAngle = 0.0;
    m_launchVelocity = 0.0;

    double Vc = 0.0;
    double time = 0.0;
    double dt = 0.0025; // Time delta between frames in seconds
    double    a, at; // stores previous time steps results for alpha and its first derivative
    double    b, bt; // stores previous time steps results for beta and its first derivative

    double    phi; // stores value of theta + beta
    double    Vc2; // square of club head velocity 
    double  ak1, ak2, ak3, ak4; // stores intermediate results of Runge-Kutta integration scheme
    double  bk1, bk2, bk3, bk4; // stores intermediate results of Runge-Kutta integration scheme
    double velocityCapture = 0;
    double launchAngle = 0.0;
    bool isVcFound = false;

    //PrintSwingMechanics(Vc, time);

    //for (int i = 0; i < 200; i++)
    for (int i = 0; i < m_swingStepIncrementCount; i++)
    {
        time += dt;
        if (time >= 0.1)
        {
            m_Qbeta = 0;
        }
        // save results of previous time step
        a = m_alpha;
        b = m_beta;
        at = m_alpha_dot;
        bt = m_beta_dot;
        // integrate alpha'' and beta''

        // The K1 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak1 = m_alpha_dotdot * dt;
        bk1 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak1 / 2;
        m_beta_dot = bt + bk1 / 2;
        // The K2 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak2 = m_alpha_dotdot * dt;
        bk2 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak2 / 2;
        m_beta_dot = bt + bk2 / 2;
        // The K3 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak3 = m_alpha_dotdot * dt;
        bk3 = m_beta_dotdot * dt;
        m_alpha_dot = at + ak3;
        m_beta_dot = bt + bk3;
        // The K3 Step:
        m_alpha_dotdot = ComputeAlphaDotDot();
        m_beta_dotdot = ComputeBetaDotDot();
        ak4 = m_alpha_dotdot * dt;
        bk4 = m_beta_dotdot * dt;
        m_alpha_dot = at + (ak1 + 2 * ak2 + 2 * ak3 + ak4) / 6;
        m_beta_dot = bt + (bk1 + 2 * bk2 + 2 * bk3 + bk4) / 6;
        m_alpha = a + m_alpha_dot * dt;
        m_beta = b + m_beta_dot * dt;
        m_theta = m_gamma - m_alpha;

        Vc2 = (m_armLength * m_armLength + m_club.length * m_club.length + 2 * m_armLength * m_club.length * cos(m_beta))
            * (m_alpha_dot * m_alpha_dot) + m_club.length * m_club.length * m_beta_dot * m_beta_dot
            - 2 * (m_club.length * m_club.length + m_armLength * m_club.length * cos(m_beta)) * m_alpha_dot * m_beta_dot;  // Jorgensen equation

        Vc = sqrt(Vc2);

        Vector4d swingAngles(m_alpha, m_beta, m_theta, 0.0);
        m_alphaBetaThetaVec.push_back(swingAngles);

        phi = m_theta + m_beta;
        if (Utility::ToDegrees(phi) < m_ballPlacementAngle)
        {
            if (isVcFound == false)
            {
                //PrintSwingMechanics(Vc, time);

                velocityCapture = Vc;
                isVcFound = true;
                //launchAngle = m_club.angle - Utility::ToDegrees(phi);
                launchAngle = m_club.angle - Utility::ToDegrees(phi);
            }
        }
    }

    m_launchAngle = launchAngle;
    m_impactData.angleY = launchAngle;
    m_launchVelocity = velocityCapture;
    m_impactData.velocity = velocityCapture;

    m_impactData.mass = m_club.mass;
    m_impactData.cor = m_club.coefficiantOfRestitution;

    return m_impactData;
}

double GolfSwing::ComputeAlphaDotDot(void)
{
    double A = (m_armMassMoI + m_club.massMoI + m_club.mass * m_armLength * m_armLength + 2 * m_armLength * m_club.firstMoment * cos(m_beta));
    double B = -(m_club.massMoI + m_armLength * m_club.firstMoment * cos(m_beta));
    double F = m_Qalpha - (m_beta_dot * m_beta_dot - 2 * m_alpha_dot * m_beta_dot) * m_armLength * m_club.firstMoment * sin(m_beta) + m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta)) + (m_armFirstMoment + m_club.mass * m_armLength)
        * (m_gravity * sin(m_theta) - m_shoulderHorizAccel * cos(m_theta));
    double D = m_club.massMoI;
    double G = m_Qbeta - m_alpha_dot * m_alpha_dot * m_armLength * m_club.firstMoment * sin(m_beta) - m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta));
    return (F - (B * G / D)) / (A - (B * B / D));
}

double GolfSwing::ComputeBetaDotDot(void)
{
    double C = -(m_club.massMoI + m_armLength * m_club.firstMoment * cos(m_beta));
    double D = m_club.massMoI;
    double G = m_Qbeta - m_alpha_dot * m_alpha_dot * m_armLength * m_club.firstMoment * sin(m_beta) - m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta));
    return (G - C * m_alpha_dotdot) / D;
}

void GolfSwing::CycleClub()
{
    ++m_clubIndex;
    if (m_clubIndex >= m_pBag->GetClubCount())
    {
        m_clubIndex = 0;
    }
    m_club = m_pBag->GetClub(m_clubIndex);
    UpdateClubData();
    UpdateGolfSwingValues();
    CalculateLaunchVector();
}

void GolfSwing::CycleInputClub(int aInput)
{
    m_club = m_pBag->GetClub(aInput);
    UpdateClubData();
    UpdateGolfSwingValues();
    CalculateLaunchVector();
}

void GolfSwing::InputSwingValuesBasic()
{
    SelectClub();

    /*
    bool isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Club Angle in degress between " << m_minClubAngle << " and " << m_maxClubAngle << ": ";
        double clubAngle;
        std::cin >> clubAngle;
        if (clubAngle >= m_minClubAngle && clubAngle <= m_maxClubAngle)
        {
            isInputValid = true;
            SetClubAngle(clubAngle);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }
    */

    bool isInputValid = false;
    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Back Swing Percentage in % between " << m_minBackSwingPercentage << " and " << m_maxBackSwingPercentage << ": ";
        double backSwingPercentage;
        std::cin >> backSwingPercentage;
        if (backSwingPercentage >= m_minBackSwingPercentage && backSwingPercentage <= m_maxBackSwingPercentage)
        {
            isInputValid = true;
            SetBackSwingPercentage(backSwingPercentage);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Ball Placement Angle in degrees between " << m_minBallPlacementAngle << " and " << m_maxBallPlacementAngle << ": ";
        double ballPlacement;
        std::cin >> ballPlacement;
        if (ballPlacement >= m_minBallPlacementAngle && ballPlacement <= m_maxBallPlacementAngle)
        {
            isInputValid = true;
            SetBallPlacementAngle(ballPlacement);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    std::cout << "Updating swing input data... \n";
}

void GolfSwing::InputSwingValuesVerbose()
{
    bool isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input arm length in meters between " << m_minArmLength << " and " << m_maxArmLength << ": ";
        double armLength;
        std::cin >> armLength;
        if (armLength >= m_minArmLength && armLength <= m_maxArmLength)
        {
            isInputValid = true;
            SetArmLength(armLength);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Back Swing Percentage in % between " << m_minBackSwingPercentage << " and " << m_maxBackSwingPercentage << ": ";
        double backSwingPercentage;
        std::cin >> backSwingPercentage;
        if (backSwingPercentage >= m_minBackSwingPercentage && backSwingPercentage <= m_maxBackSwingPercentage)
        {
            isInputValid = true;
            SetBackSwingPercentage(backSwingPercentage);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Ball Placement Angle in degrees between " << m_minBallPlacementAngle << " and " << m_maxBallPlacementAngle << ": ";
        double ballPlacement;
        std::cin >> ballPlacement;
        if (ballPlacement >= m_minBallPlacementAngle && ballPlacement <= m_maxBallPlacementAngle)
        {
            isInputValid = true;
            SetBallPlacementAngle(ballPlacement);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Club Angle in degress between " << m_minClubAngle << " and " << m_maxClubAngle << ": ";
        double clubAngle;
        std::cin >> clubAngle;
        if (clubAngle >= m_minClubAngle && clubAngle <= m_maxClubAngle)
        {
            isInputValid = true;
            SetClubAngle(clubAngle);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Club Coefficient of Restitution between (note: values above 0.830 are not legal in USGA play" << m_minClubCoR << " and " << m_maxClubCoR << ": ";
        double clubCoR;
        std::cin >> clubCoR;
        if (clubCoR >= m_minClubCoR && clubCoR <= m_maxClubCoR)
        {
            isInputValid = true;
            SetClubCoR(clubCoR);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Club Length in meters between " << m_minClubLength << " and " << m_maxClubLength << ": ";
        double clubLength;
        std::cin >> clubLength;
        if (clubLength >= m_minClubLength && clubLength <= m_maxClubLength)
        {
            isInputValid = true;
            SetClubLength(clubLength);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Club Mass in kg between " << m_minClubMass << " and " << m_maxClubMass << ": ";
        double clubMass;
        std::cin >> clubMass;
        if (clubMass >= m_minClubMass && clubMass <= m_maxClubMass)
        {
            isInputValid = true;
            SetClubMass(clubMass);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Horizonatal Acceleration prior to application of gravity in m/s^2 between " << m_minShoulderAccel << " and " << m_maxShoulderAccel << ": ";
        double shoulderAccel;
        std::cin >> shoulderAccel;
        if (shoulderAccel >= m_minShoulderAccel && shoulderAccel <= m_maxShoulderAccel)
        {
            isInputValid = true;
            SetShoulderAccel(shoulderAccel);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Shoulder Torque in newton meters between " << m_minQalpha << " and " << m_maxQalpha << ": ";
        double shoulderTorque;
        std::cin >> shoulderTorque;
        if (shoulderTorque >= m_minQalpha && shoulderTorque <= m_maxQalpha)
        {
            isInputValid = true;
            SetQalpha(shoulderTorque);
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Wrist Cock Angle in degrees between " << m_minBeta << " and " << m_maxBeta << ": ";
        double wristCockAngle;
        std::cin >> wristCockAngle;
        if (wristCockAngle >= m_minBeta && wristCockAngle <= m_maxBeta)
        {
            isInputValid = true;
            SetBeta(Utility::ToRadians(wristCockAngle));
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }

    isInputValid = false;
    while (isInputValid == false)
    {
        std::cout << "Input Wrist Torque as a positve value in newton meters between " << m_minQbeta << " and " << m_maxQbeta << ": ";
        double wristTorque;
        std::cin >> wristTorque;
        if (wristTorque >= m_minQbeta && wristTorque <= m_maxQbeta)
        {
            isInputValid = true;
            m_Qbeta = -wristTorque;
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }
}

std::vector<Vector4d> GolfSwing::OutputSwingData()
{
    CalculateSwingCordinates();
    return m_alphaBetaThetaVec;
}

void GolfSwing::PrintSwingInputData()
{
    printf("======================================= Swing Values =======================================\n");
    printf(" Arm Length                                        : %g m \n", m_armLength);
    printf(" Ball Placement Angle                              : %g degrees \n", m_ballPlacementAngle);
    printf(" Club Face Angle                                   : %g degrees\n", m_club.angle);
    printf(" Club Length                                       : %g m \n", m_club.length);
    printf(" Club Mass                                         : %g kg \n", m_club.mass);
    printf(" Swing Power                                       : %g percent \n", m_backSwingPercentage);
    printf("============================================================================================\n");
}

void GolfSwing::PrintSwingMechanics(const double aClubVelocity, const double aTime)
{
    double phiInDegrees = Utility::ToDegrees(m_theta + m_beta);
    printf("==================================== Swing Mechanics =======================================\n");
    printf(" Time                                                         : %g sec \n", aTime);
    printf(" Theta (angle betwen arm rod and verticle axis)               : %g degrees \n", Utility::ToDegrees(m_theta));
    printf(" Alpha (arm Rod angle sweep from initial backswing position)  : %g degrees \n", Utility::ToDegrees(m_alpha));
    printf(" Beta (wrist cock angle                                       : %g degrees \n", Utility::ToDegrees(m_beta));
    printf(" Phi (theta + beta)                                           : %g degrees \n", phiInDegrees);
    printf(" Club Head Velocity                                           : %g m/s \n", aClubVelocity);
    printf(" Club Face Angle at Moment                                    : %g degrees \n", m_club.angle - phiInDegrees);
    printf("============================================================================================\n");
}

void GolfSwing::ReadInSwingValues()
{
    double armLength;
    double backSwingPercentage;
    double ballPlacement;
    double clubAngle;
    double clubCoR; //  coefficient of restitution of club face
    double clubLength;
    double clubMass;
    double shoulderAcceleration; //= 0.1 * g; // Horizontal acceleration of the shoulder in  m/s^2
    double shoulderTorque; //= 100; // Torque applied at the shoulder to the arm rod in N m
    double wristCockAngle; //= RADIANS(120.0); // Wrist cock angle in radians
    double wristTorque; //= -10; // Torque applied at the wrist joint to the club rod in N m

    std::cout << "Warning: If there are any errors in SwingInputData.txt all values will be purged and default values used!\n";
    std::ifstream inFile("SwingInputData.txt");
    inFile >> armLength >> backSwingPercentage >> ballPlacement >> clubAngle >> clubCoR >> clubLength >> clubMass >> shoulderAcceleration >> shoulderTorque >> wristCockAngle >> wristTorque;
    inFile.close();

    std::vector<std::string> errorList;
    bool isInputValid = true;
    if (armLength < m_minArmLength || armLength > m_maxArmLength)
    {
        isInputValid = false;
        std::string errorString = "1 armLength";
        errorList.push_back(errorString);
    }

    if (backSwingPercentage < m_minBackSwingPercentage || backSwingPercentage > m_maxBackSwingPercentage)
    {
        isInputValid = false;
        std::string errorString = "2 backSwingPercentage";
        errorList.push_back(errorString);
    }

    if (ballPlacement < m_minBallPlacementAngle || ballPlacement > m_maxBallPlacementAngle)
    {
        isInputValid = false;
        std::string errorString = "3 ballPlacement ";
        errorList.push_back(errorString);
    }

    if (clubAngle < m_minClubAngle || clubAngle > m_maxClubAngle)
    {
        isInputValid = false;
        std::string errorString = "4 clubAngle ";
        errorList.push_back(errorString);
    }

    if (clubCoR < m_minClubCoR || clubCoR > m_maxClubCoR)
    {
        isInputValid = false;
        std::string errorString = "5 clubCoR";
        errorList.push_back(errorString);
    }

    if (clubLength < m_minClubLength || clubLength > m_maxClubLength)
    {
        isInputValid = false;
        std::string errorString = "6 clubLength ";
        errorList.push_back(errorString);
    }

    if (clubMass < m_minClubMass || clubMass > m_maxClubMass)
    {
        isInputValid = false;
        std::string errorString = "7 clubMass ";
        errorList.push_back(errorString);
    }

    if (shoulderAcceleration < m_minShoulderAccel || shoulderAcceleration > m_maxShoulderAccel)
    {
        isInputValid = false;
        std::string errorString = "8 shoulderAcceleration ";
        errorList.push_back(errorString);
    }

    if (shoulderTorque < m_minQalpha || shoulderTorque > m_maxQalpha)
    {
        isInputValid = false;
        std::string errorString = "9 shoulderTorque ";
        errorList.push_back(errorString);
    }

    if (wristCockAngle < m_minBeta || wristCockAngle > m_maxBeta)
    {
        isInputValid = false;
        std::string errorString = "10 wristCockAngle ";
        errorList.push_back(errorString);
    }

    if (wristTorque < m_minQbeta || wristTorque > m_maxQbeta)
    {
        isInputValid = false;
        std::string errorString = "11 wristTorque ";
        errorList.push_back(errorString);
    }

    if (isInputValid == false)
    {
        std::cout << "Error in SwingInputData.txt, data invalid\n";
        std::cerr << "Error in SwingInputData.txt, data invalid\n";
        for (unsigned int i = 0; i < errorList.size(); i++)
        {
            std::cout << "Error in line : " << errorList[i] << "\n";
            std::cerr << "Error in line : " << errorList[i] << "\n";
        }
        std::cout << "Reverting to default Swing data \n";
        SetDefaultSwingValues(m_defaultGravity);
    }
    else
    {
        SetArmLength(armLength);
        SetBackSwingPercentage(backSwingPercentage);
        SetBallPlacementAngle(ballPlacement);
        SetClubAngle(clubAngle);
        SetClubCoR(clubCoR);
        SetClubLength(clubLength);
        SetClubMass(clubMass);
        SetShoulderAccel(shoulderAcceleration);
        SetQalpha(shoulderTorque);
        SetBeta(Utility::ToRadians(wristCockAngle));
        SetQbeta(wristTorque);
    }
}

void GolfSwing::ResetAlphaBeta()
{
    // Input Variables
    m_alpha = 0.0; // Angle swept by arm rod from initial backswing position in radians
    m_alpha_dot = 0.0;
    m_alpha_dotdot = 0.0;
    //m_armBalancePoint = 0.5;
    //m_armLength = 0.62;
    //m_armMass = 7.3;
    //m_armMassMoI = 1.15; // Mass moment of inertia of the rod representing the arm in kg m^2
    //m_backSwingPercentage = 100.0;
    m_ballPlacementAngle = 5.0;
    m_beta = Utility::ToRadians(120.0); // Wrist cock angle in radians
    m_beta_dot = 0.0;
    m_beta_dotdot = 0.0;
    //m_club.angle = 25.0;
    //m_club.balancePoint = 0.75;
    //m_club.coefficiantOfRestitution = 0.78; // club face coefficiant of restitution, aka club spring face, current USGA rules limit this to .830 in tournemnt play
    //m_club.length = 1.1; // length of club in m
    //m_club.mass = 0.4;
    //m_club.massMoI = 0.08; // Mass moment of inertia of the rod representing the club in kg m^2
    //m_club.clubName = "Custom";
    m_Qalpha = 100; // Torque applied at the shoulder to the arm rod in N m
    m_Qbeta = -10; // Torque applied at the wrist joint to the club rod in N m

    // dependant variables 
    m_armFirstMoment = (m_armMass * m_armLength * m_armBalancePoint); // First moment of the arm rod about the shoulder axis kg m
    m_club.firstMoment = (m_club.mass * m_club.length * m_club.balancePoint); // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_shoulderHorizAccel = 0.1 * m_gravity; // Horizontal acceleration of the shoulder in  m/s^2
    m_gamma = Utility::ToRadians(135.0);
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians  
}

// Select club from GolfBag class and update member variables
void GolfSwing::SelectClub()
{
    m_pBag->PrintClubList();

    bool isInputValid = false;
    while (isInputValid == false)
    {
        int input;
        printf("Please Select Club by Number : ");
        std::cin >> input;
        --input; // undoes offset input index to improve visual of GolfBag print
        if (input >= 0 && input < m_pBag->GetClubCount())
        {
            isInputValid = true;
            m_club = m_pBag->GetClub(input);
            UpdateClubData();
        }
        else
        {
            std::cout << "Input Error, please try again \n";
        }
    }
}

void GolfSwing::SetArmLength(double aLength)
{
    m_armLength = aLength;
}

void GolfSwing::SetBackSwingPercentage(double aPercentage)
{
    m_backSwingPercentage = aPercentage;
    m_impactData.power = aPercentage;
}

void GolfSwing::SetBallPlacementAngle(double aAngle)
{
    m_ballPlacementAngle = aAngle;
}

void GolfSwing::SetBeta(double aBeta)
{
    m_beta = aBeta;
}

void GolfSwing::SetClubAngle(double aAngle)
{
    m_club.angle = aAngle;
}

void GolfSwing::SetClubCoR(double aCoR)
{
    m_club.coefficiantOfRestitution = aCoR;
}

void GolfSwing::SetClubLength(double aLength)
{
    m_club.length = aLength;
}

void GolfSwing::SetClubMass(double aMass)
{
    m_club.mass = aMass;
}

void GolfSwing::SetDefaultSwingValues(double aGravity)
{
    Utility::ZeroImpactData(m_impactData);
    if (aGravity < 0.0) // ensuring that we make gravity pointed down since golf doesn't work if its positive
    {
        m_gravity = -aGravity; // Gravity's acceleration inverted  because the downswing is working in gravity's direction m/s^2 
    }
    else
    {
        m_gravity = aGravity;
    }

    // Input Variables
    m_alpha = 0.0; // Angle swept by arm rod from initial backswing position in radians
    m_alpha_dot = 0.0;
    m_alpha_dotdot = 0.0;
    m_armBalancePoint = 0.5;
    m_armLength = 0.62;
    m_armMass = 7.3;
    m_armMassMoI = 1.15; // Mass moment of inertia of the rod representing the arm in kg m^2
    m_backSwingPercentage = 100.0;
    m_impactData.power = 100.0;
    m_ballPlacementAngle = 5.0;
    m_beta = Utility::ToRadians(120.0); // Wrist cock angle in radians
    m_beta_dot = 0.0;
    m_beta_dotdot = 0.0;
    m_club.angle = 25.0;
    m_impactData.angleX = 25.0;
    m_club.balancePoint = 0.75;
    m_club.coefficiantOfRestitution = 0.78; // club face coefficiant of restitution, aka club spring face, current USGA rules limit this to .830 in tournemnt play
    m_impactData.cor = 0.78;
    m_club.length = 1.1; // length of club in m
    m_club.mass = 0.4;
    m_impactData.mass = 0.4;
    m_club.massMoI = 0.08; // Mass moment of inertia of the rod representing the club in kg m^2
    m_club.clubName = "Custom";
    m_Qalpha = 100; // Torque applied at the shoulder to the arm rod in N m
    m_Qbeta = -10; // Torque applied at the wrist joint to the club rod in N m

    // dependant variables 
    m_armFirstMoment = (m_armMass * m_armLength * m_armBalancePoint); // First moment of the arm rod about the shoulder axis kg m
    m_club.firstMoment = (m_club.mass * m_club.length * m_club.balancePoint); // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_shoulderHorizAccel = 0.1 * m_gravity; // Horizontal acceleration of the shoulder in  m/s^2
    m_gamma = Utility::ToRadians(135.0);
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians  
}

void GolfSwing::SetShoulderAccel(double aShouldAcell)
{
    m_shoulderHorizAccel = aShouldAcell * m_gravity;
}

void GolfSwing::SetQalpha(double aQalpha)
{
    m_Qalpha = aQalpha;
}

void GolfSwing::SetQbeta(double aQbeta)
{
    m_Qbeta = aQbeta;
}

void GolfSwing::UpdateClubData()
{
    //m_clubAngle = m_club.clubAngle;
    //m_club.clubBalancePoint = m_club.clubBalancePoint;
    //m_clubCoR = m_club.clubCoR;
    //m_clubFirstMoment = m_club.clubFirstMoment;
    //m_clubLength = m_club.clubLength;
    //m_clubMass = m_club.clubMass;
    //m_clubMassMoI = m_club.clubMassMoI;
}

void GolfSwing::UpdateGolfSwingValues()
{
    m_club.firstMoment = m_club.mass * m_club.length * m_club.balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_armFirstMoment = m_armMass * m_armLength * m_armBalancePoint; // First moment of the arm rod about the shoulder axis kg m
    //double swingFactor = m_backSwingPercentage * 0.01;
    double swingFactor = m_impactData.power * 0.01;
    m_beta = m_beta * swingFactor;
    m_gamma = m_gamma * swingFactor;
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians
}

void GolfSwing::UpdateImpactData(Utility::ImpactData aImpactData)
{
    m_launchImpact = aImpactData.impactMissOffSet;
    m_impactData.impactMissOffSet = aImpactData.impactMissOffSet;
    m_backSwingPercentage = aImpactData.power;
    m_impactData.power = aImpactData.power;
    UpdateGolfSwingValues();
    //CalculateLaunchVector2();
}

