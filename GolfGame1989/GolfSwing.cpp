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

    double Vc = 0.0;
    double time = 0.0;
    double dt = m_timeDelta; // Time delta between frames in seconds
    double a, at; // stores previous time steps results for alpha and its first derivative
    double b, bt; // stores previous time steps results for beta and its first derivative
    double phi; // stores value of theta + beta
    double Vc2; // square of club head velocity 
    double ak1, ak2, ak3, ak4; // stores intermediate results of Runge-Kutta integration scheme
    double bk1, bk2, bk3, bk4; // stores intermediate results of Runge-Kutta integration scheme
    double velocityCapture = 0;
    double launchAngle = 0.0;
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
        m_beta = b + m_beta_dot * dt;
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
    m_impactData.vHead.x = static_cast<float>(velocityCapture);
    m_impactData.vFaceNormal.x = static_cast<float>(cos(Utility::ToRadians(launchAngle)));
    m_impactData.vFaceNormal.y = static_cast<float>(sin(Utility::ToRadians(launchAngle)));
    m_impactData.vFaceNormal.z = 0.0; // WLJ ToDo: update with value from play mechanics

    m_impactData.vFaceNormal = DirectX::SimpleMath::Vector3::Transform(m_impactData.vFaceNormal, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(Utility::ToRadians(m_impactData.impactMissOffSet))));

    m_impactData.vFaceNormal.Normalize();

    m_impactData.vHeadNormal = m_impactData.vHead.Dot(m_impactData.vFaceNormal) * m_impactData.vFaceNormal;
    m_impactData.vHeadParallel = m_impactData.vHead - m_impactData.vHeadNormal;

    return m_impactData;
}

double GolfSwing::ComputeAlphaDotDot(void) const
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

double GolfSwing::ComputeBetaDotDot(void) const
{
    double C = -(m_club.massMoI + m_armLength * m_club.firstMoment * cos(m_beta));
    double D = m_club.massMoI;
    double G = m_Qbeta - m_alpha_dot * m_alpha_dot * m_armLength * m_club.firstMoment * sin(m_beta) - m_club.firstMoment
        * (m_gravity * sin(m_theta + m_beta) - m_shoulderHorizAccel * cos(m_theta + m_beta));
    return (G - C * m_alpha_dotdot) / D;
}

void GolfSwing::InputClub(int aInput)
{
    m_club = m_pBag->GetClub(aInput);
    UpdateGolfSwingValues();
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
    m_shoulderHorizAccel = 0.1 * m_gravity; // Horizontal acceleration of the shoulder in  m/s^2
    m_gamma = Utility::ToRadians(135.0);
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians  
}

void GolfSwing::SetArmBalancePoint(double aBalancePoint)
{
    m_armBalancePoint = aBalancePoint;
}

void GolfSwing::SetArmLength(double aLength)
{
    m_armLength = aLength;
}

void GolfSwing::SetArmMass(double aArmMass)
{
    m_armMass = aArmMass;
}

void GolfSwing::SetArmMassMoI(double aArmMassMoi)
{
    m_armMassMoI = aArmMassMoi;
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

void GolfSwing::SetClubLengthModifier(double aLengthModifier)
{
    m_clubLengthModifier = aLengthModifier;
    m_club.length = m_club.lengthBase * m_clubLengthModifier;
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

    m_club.length = m_club.lengthBase * m_clubLengthModifier;
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

void GolfSwing::UpdateGolfer()
{
    m_club.firstMoment = m_club.mass * m_club.length * m_club.balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_armFirstMoment = m_armMass * m_armLength * m_armBalancePoint; // First moment of the arm rod about the shoulder axis kg m
}

void GolfSwing::UpdateGolfSwingValues()
{
    m_club.firstMoment = m_club.mass * m_club.length * m_club.balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    m_armFirstMoment = m_armMass * m_armLength * m_armBalancePoint; // First moment of the arm rod about the shoulder axis kg m
    double swingFactor = m_impactData.power * 0.01;
    m_beta = m_beta * swingFactor;
    m_gamma = m_gamma * swingFactor;
    m_theta = m_gamma - m_alpha;  // Angle between arm rod and vertical axis in radians
}

void GolfSwing::UpdateGravityDependants(const double aGravity)
{
    m_gravity = aGravity;
    m_shoulderHorizAccel = 0.1 * m_gravity;
}

void GolfSwing::UpdateImpactData(Utility::ImpactData aImpactData)
{
    m_launchImpact = aImpactData.impactMissOffSet;
    m_impactData.impactMissOffSet = aImpactData.impactMissOffSet;
    m_backSwingPercentage = aImpactData.power;
    m_impactData.power = aImpactData.power;
    UpdateGolfSwingValues();
}

void GolfSwing::ZeroDataForUI()
{
    m_backSwingPercentage = 0.0;
    m_launchAngle = 0.0;
    m_launchVelocity = 0.0;
}