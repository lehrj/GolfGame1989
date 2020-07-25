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

        DirectX::SimpleMath::Vector3 swingAngles(m_alpha, m_beta, m_theta);
        m_alphaBetaThetaVec.push_back(swingAngles);

        phi = m_theta + m_beta;
        if (Utility::ToDegrees(phi) < m_ballPlacementAngle)
        {
            if (isVcFound == false)
            {
                //PrintSwingMechanics(Vc, time);
                m_swingImpactStep = i;
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

    // start work for switch to vector usage
    m_impactData.vHead.x = velocityCapture;
    m_impactData.vFaceNormal.x = cos(Utility::ToRadians(launchAngle));
    m_impactData.vFaceNormal.y = sin(Utility::ToRadians(launchAngle));
    m_impactData.vFaceNormal.z = 0.0; // WLJ ToDo: update with value from play mechanics

    m_impactData.vFaceNormal = DirectX::SimpleMath::Vector3::Transform(m_impactData.vFaceNormal, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(m_impactData.impactMissOffSet)));
    //m_impactData.vFaceNormal = DirectX::SimpleMath::Vector3::Transform(m_impactData.vFaceNormal, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(10.5)));

    m_impactData.vFaceNormal.Normalize();

    m_impactData.vHeadNormal = m_impactData.vHead.Dot(m_impactData.vFaceNormal) * m_impactData.vFaceNormal;
    m_impactData.vHeadParallel = m_impactData.vHead - m_impactData.vHeadNormal;
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

void GolfSwing::InputClub(int aInput)
{
    m_club = m_pBag->GetClub(aInput);
    UpdateGolfSwingValues();
    //CalculateLaunchVector();
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
    m_club.length = m_club.lengthBase * m_clubLengthModifier;
    // Input Variables
    m_alpha = 0.0; // Angle swept by arm rod from initial backswing position in radians
    m_alpha_dot = 0.0;
    m_alpha_dotdot = 0.0;
    //m_armBalancePoint = 0.5;
    //m_armLength = 0.62;
    //m_armMass = 7.3;
    //m_armMassMoI = 1.15; // Mass moment of inertia of the rod representing the arm in kg m^2
    //m_backSwingPercentage = 100.0;
    //m_ballPlacementAngle = 5.0;
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

    m_club.length = 1.1; // length of club in m
    //m_club.length = m_club.lengthBase * m_clubLengthModifier;
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
    //double swingFactor = m_backSwingPercentage * 0.01;
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
    //CalculateLaunchVector2();
}
