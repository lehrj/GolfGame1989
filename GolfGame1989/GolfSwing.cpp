#include "pch.h"
#include "GolfSwing.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


GolfSwing::GolfSwing()
{
    m_pBag = new GolfBag();
    InputClub(m_clubIndex);
}

GolfSwing::~GolfSwing()
{
    delete m_pBag;
}

Utility::ImpactData GolfSwing::CalculateLaunchVector()
{
    m_alphaBetaThetaVec.clear();
    m_launchAngle = 0.0;
    m_launchVelocity = 0.0;

    float Vc = 0.0;
    double time = 0.0;
    float dt = static_cast<float>(m_timeDelta); // Time delta between frames in seconds
    float a, at; // stores previous time steps results for alpha and its first derivative
    float b, bt; // stores previous time steps results for beta and its first derivative
    float phi; // stores value of theta + beta
    float Vc2; // square of club head velocity 
    float ak1, ak2, ak3, ak4; // stores intermediate results of Runge-Kutta integration scheme
    float bk1, bk2, bk3, bk4; // stores intermediate results of Runge-Kutta integration scheme
    float velocityCapture = 0;
    float launchAngle = 0.0;
    bool isVcFound = false;

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
        
        // No wrist hinge when putting
        if (m_club.clubName != "Putter")
        {
            m_beta = b + m_beta_dot * dt;
        }
        else
        {
            m_beta = 0.09;
        }
        
        m_theta = m_gamma - m_alpha;

        Vc2 = (m_armLength * m_armLength + m_club.length * m_club.length + 2 * m_armLength * m_club.length * cos(m_beta))
            * (m_alpha_dot * m_alpha_dot) + m_club.length * m_club.length * m_beta_dot * m_beta_dot
            - 2 * (m_club.length * m_club.length + m_armLength * m_club.length * cos(m_beta)) * m_alpha_dot * m_beta_dot;  // Jorgensen equation

        Vc = sqrt(Vc2);

        DirectX::SimpleMath::Vector3 swingAngles(static_cast<float>(m_alpha), static_cast<float>(m_beta), static_cast<float>(m_theta));
        m_alphaBetaThetaVec.push_back(swingAngles);

        phi = m_theta + m_beta;
        if (Utility::ToDegrees(phi) < m_ballPlacementAngle)
        {
            if (isVcFound == false)
            {
                m_swingImpactStep = i;
                velocityCapture = Vc;
                isVcFound = true;
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

    // start work for switch to vector usage
    // Change club swing path for Push/Pull effect, ToDo : update with input from gameplay mechanics
    DirectX::SimpleMath::Matrix swingPathRotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationY(m_impactData.swingPlaneAngle);

    m_impactData.vHead.x = static_cast<float>(velocityCapture);    

    m_impactData.vHead = DirectX::SimpleMath::Vector3::Transform(m_impactData.vHead, swingPathRotationMatrix);

    m_impactData.vFaceNormal.x = static_cast<float>(cos(Utility::ToRadians(launchAngle)));
    m_impactData.vFaceNormal.y = static_cast<float>(sin(Utility::ToRadians(launchAngle)));
    m_impactData.vFaceNormal.z = 0.0; // WLJ ToDo: update with value from play mechanics
    //m_impactData.vFaceNormal.z = static_cast<float>(cos(Utility::ToRadians(launchAngle)));; // WLJ ToDo: update with value from play mechanics   

    m_impactData.vFaceNormal = DirectX::SimpleMath::Vector3::Transform(m_impactData.vFaceNormal, 
        DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(Utility::ToRadians(m_impactData.impactMissOffSet))));

    m_impactData.vFaceNormal = DirectX::SimpleMath::Vector3::Transform(m_impactData.vFaceNormal, swingPathRotationMatrix);

    m_impactData.vFaceNormal.Normalize();

    m_impactData.vHeadNormal = m_impactData.vHead.Dot(m_impactData.vFaceNormal) * m_impactData.vFaceNormal;
    m_impactData.vHeadParallel = m_impactData.vHead - m_impactData.vHeadNormal;



    return m_impactData;
}

float GolfSwing::ComputeAlphaDotDot(void) const
{
    float A = (m_armMassMoI + m_club.massMoI + m_club.mass * m_armLength * m_armLength + 2 * m_armLength * m_club.firstMoment * cos(m_beta));
    float B = -(m_club.massMoI + m_armLength * m_club.firstMoment * cos(m_beta));
    float F = m_Qalpha - (m_beta_dot * m_beta_dot - 2 * m_alpha_dot * m_beta_dot) * m_armLength * m_club.firstMoment * sin(m_beta) + m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta)) + (m_armFirstMoment + m_club.mass * m_armLength)
        * (m_gravity * sin(m_theta) - m_shoulderHorizAccel * cos(m_theta));
    float D = m_club.massMoI;
    float G = m_Qbeta - m_alpha_dot * m_alpha_dot * m_armLength * m_club.firstMoment * sin(m_beta) - m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta));
    return (F - (B * G / D)) / (A - (B * B / D));
}

float GolfSwing::ComputeBetaDotDot(void) const
{
    float C = -(m_club.massMoI + m_armLength * m_club.firstMoment * cos(m_beta));
    float D = m_club.massMoI;
    float G = m_Qbeta - m_alpha_dot * m_alpha_dot * m_armLength * m_club.firstMoment * sin(m_beta) - m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta));
    return (G - C * m_alpha_dotdot) / D;
}

void GolfSwing::CycleClub(const bool aIsCycleClubUp)
{
    int newClubIndex = m_clubIndex;
    if (aIsCycleClubUp == true)
    {
        if (m_clubIndex < m_pBag->GetClubCount() - 1 && m_clubIndex >= 0)
        {
            newClubIndex++;
        }
        else
        {
            newClubIndex = 0;
        }
    }
    else
    {
        if (m_clubIndex > 0 && m_clubIndex < m_pBag->GetClubCount())
        {
            newClubIndex--;
        }
        else
        {
            newClubIndex = m_pBag->GetClubCount() - 1;
        }
    }
    this->InputClub(newClubIndex);
}

// ToDo: Add build function for this to remove calculations from the Get()
DirectX::SimpleMath::Vector3 GolfSwing::GetShoulderPos()
{
    if (GetSwingImpactStep() == 0)
    { 
        // add error checking
        return DirectX::SimpleMath::Vector3(0.0,GetArmLength() + GetClubLength(), 0.0);
    }
    else
    {
        const float armAngle = -m_alphaBetaThetaVec[GetSwingImpactStep()].z;
        const float clubAngle = -m_alphaBetaThetaVec[GetSwingImpactStep()].y + armAngle;
        
        DirectX::SimpleMath::Vector3 club(0.0, GetClubLength(), 0.0);
        club = DirectX::SimpleMath::Vector3::Transform(club, DirectX::SimpleMath::Matrix::CreateRotationZ(clubAngle));
        DirectX::SimpleMath::Vector3 arm(0.0, GetArmLength(), 0.0);
        arm = DirectX::SimpleMath::Vector3::Transform(arm, DirectX::SimpleMath::Matrix::CreateRotationZ(armAngle));

        DirectX::SimpleMath::Vector3 shoulderPos = club + arm;

        float halfClubHeadWidth = (GetClubLength() * 0.11f) * .5f; // 0.11 is the ratio of club length to club head width as measured, use half that so ball impacts center of club head
        shoulderPos.z -= halfClubHeadWidth;  //use += for lefties once that functionality is implemented
        return shoulderPos;
    }
}

void GolfSwing::InputClub(int aInput)
{
    if (aInput <= m_pBag->GetClubCount() || aInput > 0)
    {
        m_clubIndex = aInput;
        m_club = m_pBag->GetClub(aInput);
        UpdateGolfSwingValues();
    }
    else
    {
        // add input error handling 
    }
}

void GolfSwing::ResetAlphaBeta()
{
    m_alphaBetaThetaVec.clear();
    m_alphaCord.clear();
    m_betaCord.clear();
    m_thetaCord.clear();

    m_club.length = m_club.lengthBase * m_clubLengthModifier;
    // Input Variables
    m_alpha = 0.0; // Angle swept by arm rod from initial backswing position in radians
    m_alpha_dot = 0.0;
    m_alpha_dotdot = 0.0;
    m_beta = Utility::ToRadians(120.0); // Wrist cock angle in radians
    m_beta_dot = 0.0;
    m_beta_dotdot = 0.0;
    m_Qalpha = 100; // Torque applied at the shoulder to the arm rod in N m
    m_Qbeta = -10; // Torque applied at the wrist joint to the club rod in N m

    // dependant variables 
    m_armFirstMoment = (m_armMass * m_armLength * m_armBalancePoint); // First moment of the arm rod about the shoulder axis kg m
    m_club.firstMoment = (m_club.mass * m_club.length * m_club.balancePoint); // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_shoulderHorizAccel = 0.1f * VerifySwingGravityDirection(m_gravity); // Horizontal acceleration of the shoulder in  m/s^2
    m_gamma = Utility::ToRadians(135.0);
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians  
}

void GolfSwing::SetArmBalancePoint(float aBalancePoint)
{
    m_armBalancePoint = aBalancePoint;
}

void GolfSwing::SetArmLength(float aLength)
{
    m_armLength = aLength;
}

void GolfSwing::SetArmMass(float aArmMass)
{
    m_armMass = aArmMass;
}

void GolfSwing::SetArmMassMoI(float aArmMassMoi)
{
    m_armMassMoI = aArmMassMoi;
}

void GolfSwing::SetBackSwingPercentage(float aPercentage)
{
    m_backSwingPercentage = aPercentage;
    m_impactData.power = aPercentage;
}

void GolfSwing::SetBallPlacementAngle(float aAngle)
{
    m_ballPlacementAngle = aAngle;
}

void GolfSwing::SetBeta(float aBeta)
{
    m_beta = aBeta;
}

void GolfSwing::SetClubAngle(float aAngle)
{
    m_club.angle = aAngle;
}

void GolfSwing::SetClubCoR(float aCoR)
{
    m_club.coefficiantOfRestitution = aCoR;
}

void GolfSwing::SetClubLength(float aLength)
{
    m_club.length = aLength;
}

void GolfSwing::SetClubLengthModifier(float aLengthModifier)
{
    m_clubLengthModifier = aLengthModifier;
    m_club.length = m_club.lengthBase * m_clubLengthModifier;
}

void GolfSwing::SetClubMass(float aMass)
{
    m_club.mass = aMass;
}

void GolfSwing::SetDefaultSwingValues(float aGravity)
{
    Utility::ZeroImpactData(m_impactData);

    m_gravity = VerifySwingGravityDirection(aGravity);

    // Input Variables
    m_alpha = 0.0; // Angle swept by arm rod from initial backswing position in radians
    m_alpha_dot = 0.0;
    m_alpha_dotdot = 0.0;
    m_backSwingPercentage = 100.0;
    m_impactData.power = 100.0;
    m_ballPlacementAngle = Utility::ToRadians(0.0);
    m_beta = Utility::ToRadians(120.0); // Wrist cock angle in radians
    m_beta_dot = 0.0;
    m_beta_dotdot = 0.0;
    m_impactData.angleX = 25.0;
    m_impactData.cor = 0.78;
    m_club.length = m_club.lengthBase * m_clubLengthModifier;
    m_Qalpha = 100; // Torque applied at the shoulder to the arm rod in N m
    m_Qbeta = -10; // Torque applied at the wrist joint to the club rod in N m

    // dependant variables 
    m_armFirstMoment = (m_armMass * m_armLength * m_armBalancePoint); // First moment of the arm rod about the shoulder axis kg m
    m_club.firstMoment = (m_club.mass * m_club.length * m_club.balancePoint); // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_shoulderHorizAccel = 0.1f * VerifySwingGravityDirection(m_gravity); // Horizontal acceleration of the shoulder in  m/s^2
    m_gamma = Utility::ToRadians(135.0);
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians  
}

void GolfSwing::SetImpactDirectionDegrees(const float aDirectionDegrees)
{
    m_impactData.directionDegrees = aDirectionDegrees;
}

void GolfSwing::SetShoulderAccel(float aShouldAcell)
{
    m_shoulderHorizAccel = aShouldAcell * VerifySwingGravityDirection(m_gravity);
}

void GolfSwing::SetQalpha(float aQalpha)
{
    m_Qalpha = aQalpha;
}

void GolfSwing::SetQbeta(float aQbeta)
{
    m_Qbeta = aQbeta;
}

void GolfSwing::UpdateGolfer()
{
    m_club.firstMoment = m_club.mass * m_club.length * m_club.balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_armFirstMoment = m_armMass * m_armLength * m_armBalancePoint; // First moment of the arm rod about the shoulder axis kg m
}

void GolfSwing::UpdateGolfSwingValues()
{
    m_club.firstMoment = m_club.mass * m_club.length * m_club.balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_armFirstMoment = m_armMass * m_armLength * m_armBalancePoint; // First moment of the arm rod about the shoulder axis kg m
    float swingFactor = m_impactData.power * 0.01f;
    m_beta = m_beta * swingFactor;
    m_gamma = m_gamma * swingFactor;
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians
}

void GolfSwing::UpdateGravityDependants(const float aGravity)
{
    m_gravity = VerifySwingGravityDirection(aGravity);

    m_shoulderHorizAccel = 0.1f * m_gravity;
}

void GolfSwing::UpdateImpactData(Utility::ImpactData aImpactData)
{
    m_launchImpact = aImpactData.impactMissOffSet;
    m_impactData.impactMissOffSet = aImpactData.impactMissOffSet;
    m_backSwingPercentage = aImpactData.power;
    m_impactData.power = aImpactData.power;
    m_impactData.directionDegrees = aImpactData.directionDegrees;

    UpdateGolfSwingValues();
}

void GolfSwing::UpdateImpactDataAxis(const float aAxisAngle)
{
    m_impactData.ballAxisTilt += aAxisAngle;
}

void GolfSwing::UpdateImpactDataPlane(const float aPlaneAngle)
{
    m_impactData.swingPlaneAngle += aPlaneAngle;
}

// Verify gravity direction is pointing in the right positive/negative direction for the swing equations
float GolfSwing::VerifySwingGravityDirection(float aGravity) const
{
    if (aGravity < 0.0) 
    {
        aGravity = aGravity;
    }
    else
    {
        aGravity = -aGravity;
    }
    return aGravity;
}

void GolfSwing::ZeroDataForUI()
{
    m_backSwingPercentage = 0.0;
    m_launchAngle = 0.0;
    m_launchVelocity = 0.0;
}