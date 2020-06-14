#include "pch.h"
#include <iostream>
#include <malloc.h>
#include <math.h>
#include <memory>
#include "GolfBall.h"
#include "Vector4d.h"
#include <vector>

double GolfBall::CalculateImpactTime(double aTime1, double aTime2, double aHeight1, double aHeight2)
{
    double m = (aHeight2 - aHeight1) / (aTime2 - aTime1);
    double b = aHeight1 - (m * aTime1);
    double impactTime = -b / m;
    double dt = aTime2 - impactTime;
    return dt;
}

void GolfBall::FireProjectile(Vector4d aSwingInput, Environment* pEnviron)
{
    //PrepProjectileLaunch(aSwingInput);
    PrepProjectileLaunch2(aSwingInput, 0.0);
    LaunchProjectile();
    //LandProjectile(pEnviron);
}

void GolfBall::FireProjectile2(Vector4d aSwingInput, Environment* pEnviron, float aImpact)
{
    //PrepProjectileLaunch(aSwingInput);
    PrepProjectileLaunch2(aSwingInput, aImpact);
    LaunchProjectile();
    //LandProjectile(pEnviron);
}
void GolfBall::LandProjectile()
{
    printf("Warning: Landing bounce/roll functionality not fully implemented (WIP)\n");

    DirectX::SimpleMath::Vector3 impactAngle = GetImpactAngle();
    Vector4d impactVector{ m_ball.q[0], m_ball.q[2],m_ball.q[4], 0.0 }; // vx, vy, vz

    double impactSpeed = impactVector.Magnitude3();
    printf("Impact Speed = %lf (m/s) \n", impactSpeed);
    impactVector.NormalizeVector(impactVector);
}

void GolfBall::LaunchProjectile()
{
    double shotOrigin = 0.0;
    m_xVals.push_back(shotOrigin);
    m_yVals.push_back(shotOrigin);
    m_zVals.push_back(shotOrigin);

    // Fly ball on an upward trajectory until it stops climbing
    Vector4d flightData;
    double dt = m_timeStep;
    double maxHeight = m_ball.launchHeight;
    double time = 0.0;
    SetInitialSpinRate(m_ball.omega);
    double x = m_ball.q[1];
    double y = m_ball.q[3];

    bool isBallAscending = true;
    while (isBallAscending == true)
    {
        ProjectileRungeKutta4(&m_ball, dt);

        flightData.SetAll(m_ball.q[1], m_ball.q[3], m_ball.q[2], m_ball.flightTime);

        //PrintFlightData();
        PushFlightData();

        if (m_ball.q[2] < 0.0)
        {
            maxHeight = m_ball.q[3];
            isBallAscending = false;
        }
    }
    // Check to verify landing area height can be reached. If it cannot the shot is treated as if it is out of play so x = 0.0;
    /*
    if (maxHeight + m_ball.launchHeight < m_ball.landingHeight)
    {
        printf("Ball has landed out of play, ball does not reach height of landing area!\n");
        flightData.SetX(0.0);
        x = 0.0;
    }
    */
    //else
    //{

    double previousY = flightData.GetY();
    double previousTime = flightData.GetW();
    //  Calculate ball decent path until it reaches landing area height
    while (m_ball.q[3] + m_ball.launchHeight >= m_ball.landingHeight)
    {
        previousY = flightData.GetY();
        previousTime = flightData.GetW();
        ProjectileRungeKutta4(&m_ball, dt);
        flightData.SetAll(m_ball.q[1], m_ball.q[3], m_ball.q[2], m_ball.flightTime);
        //PrintFlightData();
        PushFlightData();
        time = m_ball.flightTime;
        y = m_ball.q[3];
    }

    //double rollBackTime = CalculateImpactTime(previousTime, time, previousY, y);
    //ProjectileRungeKutta4(&m_ball, -rollBackTime);
    flightData.SetAll(m_ball.q[1], m_ball.q[3], m_ball.q[2], m_ball.flightTime);

    // WLJ BugTask: look into systemic bugs from erro passing dz value instead of z value here
    //SetLandingCordinates(flightData.GetX(), flightData.GetY(), flightData.GetZ());
        /*
    q[0] = vx, velocity
    q[1] = x position
    q[2] = vy, velocity
    q[3] = y position
    q[4] = vz, velocity
    q[5] = z position
    */
    SetLandingCordinates(m_ball.q[1], m_ball.q[3], m_ball.q[5]);
    SetLandingSpinRate(m_ball.omega);
    SetMaxHeight(maxHeight);
    LandProjectile();
    //PrintLandingData(flightData, maxHeight);
}


DirectX::SimpleMath::Vector4 GolfBall::CalculateImpactVector(double aVelocity, double aFaceAngle, double aFaceRotation)
{
    DirectX::SimpleMath::Vector4 impactNormal = DirectX::SimpleMath::Vector4::Zero;
    impactNormal.x = cos(aFaceAngle);
    impactNormal.y = sin(aFaceAngle);
    impactNormal.w = 1.0;
    impactNormal.Normalize();
    //impactNormal.w = 1.0;
    aFaceRotation = Utility::ToRadians(-15.0);

    impactNormal = DirectX::SimpleMath::Vector4::Transform(impactNormal, DirectX::SimpleMath::Matrix::CreateRotationY(aFaceRotation));
    return impactNormal;
}

void GolfBall::PrepProjectileLaunch(Vector4d aSwingInput)
{
    DirectX::SimpleMath::Vector4 impactVector = CalculateImpactVector(aSwingInput.GetX(), Utility::ToRadians(aSwingInput.GetY()), 0.0);

    //  Convert the loft angle from degrees to radians and
    //  assign values to some convenience variables.
    double loft = Utility::ToRadians(aSwingInput.GetY());
    double cosL = cos(loft);
    double sinL = sin(loft);

    //  Calculate the pre-collision velocities normal
    //  and parallel to the line of action.
    double velocity = aSwingInput.GetX();
    double vcp = cosL * velocity;
    double vcn = -sinL * velocity;

    //  Compute the post-collision velocity of the ball
    //  along the line of action.
    double ballMass = m_ball.mass;
    double clubMass = aSwingInput.GetZ();
    double e = aSwingInput.GetW(); //  coefficient of restitution of club face striking the ball
    double vbp = (1.0 + e) * clubMass * vcp / (clubMass + ballMass);

    //  Compute the post-collision velocity of the ball
    //  perpendicular to the line of action.
    double vbn = (1.0 - m_faceRoll) * clubMass * vcn / (clubMass + ballMass);

    //  Compute the initial spin rate assuming ball is
    //  rolling without sliding.
    double radius = m_ball.radius;
    double omega = m_faceRoll * vcn / radius;

    std::cout << "omega = " << omega << std::endl;
    std::cout << "vcn = " << vcn << std::endl;
    std::cout << "radius = " << radius << std::endl;
    std::cout << "m_ball.radius = " << m_ball.radius << std::endl;

    //  Rotate post-collision ball velocities back into 
    //  standard Cartesian frame of reference. Because the
    //  line-of-action was in the xy plane, the z-velocity
    //  is zero.
    double vx0 = cosL * vbp - sinL * vbn;
    double vy0 = sinL * vbp + cosL * vbn;
    double vz0 = 0.0;

    printf("vx0=%lf  vy0=%lf  vz0=%lf  omega=%lf\n", vx0, vy0, vz0, omega);

    //  Load the initial ball velocities into the 
    //  SpinProjectile struct.
    m_ball.omega = omega;
    m_ball.q[0] = vx0;   //  vx 
    m_ball.q[2] = vy0;   //  vy 
    m_ball.q[4] = vz0;   //  vz 
}

/*
DirectX::SimpleMath::Vector4 GolfBall::CalculateImpactVector(double aVelocity, double aFaceAngle, double aFaceRotation)
{
    DirectX::SimpleMath::Vector4 impactNormal = DirectX::SimpleMath::Vector4::Zero;
    impactNormal.x = cos(aFaceAngle);
    impactNormal.y = sin(aFaceAngle);
    impactNormal.w = 1.0;
    impactNormal.Normalize();
    aFaceRotation = Utility::ToRadians(-15.0);
    impactNormal = DirectX::SimpleMath::Vector4::Transform(impactNormal, DirectX::SimpleMath::Matrix::CreateRotationY(aFaceRotation));
    return impactNormal;
}
*/
void GolfBall::PrepProjectileLaunch2(Vector4d aSwingInput, float aImpact)
{
    //Vector4d aSwingInput = { m_launchVelocity, m_launchAngle, m_club.mass, m_club.coefficiantOfRestitution };
    DirectX::SimpleMath::Vector4 vHead = DirectX::SimpleMath::Vector4::Zero;
    vHead.x = aSwingInput.GetFirst();
    
    //vHead.w = aSwingInput.GetFirst();

    DirectX::SimpleMath::Vector4 vFaceNormal = DirectX::SimpleMath::Vector4::Zero;
    vFaceNormal.x = cos(Utility::ToRadians(aSwingInput.GetY()));
    vFaceNormal.y = sin(Utility::ToRadians(aSwingInput.GetY()));
    vFaceNormal.z = sin(Utility::ToRadians(aImpact));
    vFaceNormal.Normalize();
    
    DirectX::SimpleMath::Vector4 vHeadNormal = (vHead.Dot(vFaceNormal)) * vFaceNormal;
    
    DirectX::SimpleMath::Vector4 vHeadParallel = vHead - vHeadNormal;
    

    double ballMass = m_ball.mass;
    double clubMass = aSwingInput.GetZ();
    double e = aSwingInput.GetW(); //  coefficient of restitution of club face striking the ball

    double div1 = ((1.0 + e) * clubMass) / (clubMass + ballMass);
    double div2 = (2 * clubMass) / (7 * (clubMass + ballMass));

    DirectX::SimpleMath::Vector4 vBall = (div1 * vHeadNormal) + (div2 * vHeadParallel);
    DirectX::SimpleMath::Vector4 vBall1 = (((1.0 + e) * clubMass) / (clubMass + ballMass) * vHeadNormal);
    DirectX::SimpleMath::Vector4 vBall2 = ((2 * clubMass) / (7 * (clubMass + ballMass)) * vHeadParallel);
    DirectX::SimpleMath::Vector4 vBall3 = vBall1 + vBall2;

    DirectX::SimpleMath::Vector3 unitVHead = vHead;
    unitVHead.Normalize();
    DirectX::SimpleMath::Vector3 unitFaceNormal = vFaceNormal;
    unitFaceNormal.Normalize();


    DirectX::SimpleMath::Vector3 a = unitVHead;
    DirectX::SimpleMath::Vector3 b = unitFaceNormal;

    DirectX::SimpleMath::Vector3 c;
    float x = a.y * b.z - a.z * b.y;
    float y = a.z * b.x - a.x * b.z;
    float z = a.x * b.y - a.y * b.x;
    //c = (a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    //c = (x, y, z);
    c.x = x;
    c.y = y;
    c.z = z;
    DirectX::SimpleMath::Vector4 crossVheadvFace = c;

    DirectX::SimpleMath::Vector4 absvHeadParallel = vHeadParallel;
    absvHeadParallel.x = abs(absvHeadParallel.x);
    absvHeadParallel.y = abs(absvHeadParallel.y);
    absvHeadParallel.z = abs(absvHeadParallel.z);
    absvHeadParallel.w = abs(absvHeadParallel.w);

    float absVhP = sqrt((vHeadParallel.x * vHeadParallel.x) + (vHeadParallel.y * vHeadParallel.y) + (vHeadParallel.z * vHeadParallel.z));

    //crossVheadvFace = unitVHead;
    //crossVheadvFace.Cross(unitFaceNormal);

    DirectX::SimpleMath::Vector4 omegaBall = DirectX::SimpleMath::Vector4::Zero;
    //omegaBall = ((5 * absvHeadParallel) / (7 * m_ball.radius)) * crossVheadvFace;
    omegaBall = ((5.0 * absVhP) / (7.0 * m_ball.radius)) * crossVheadvFace;
    
    //DirectX::SimpleMath::Vector4 crossVheadvFace;// = DirectX::SimpleMath::Vector4::Cross(unitVHead, unitFaceNormal);
    //DirectX::SimpleMath::Vector4 crossVheadvFace = unitVHead;
    //crossVheadvFace.Cross(unitVHead, unitFaceNormal);
    //crossVheadvFace.Cross(unitVHead, unitFaceNormal);

    //omegaBall = ((5 * abs(vHeadParallel)) / (7 * m_ball.radius) * (unitVHead.Cross(unitFaceNormal)));

    /*
    //Vector4d aSwingInput = { m_launchVelocity, m_launchAngle, m_club.mass, m_club.coefficiantOfRestitution };
    DirectX::SimpleMath::Vector3 vHead = DirectX::SimpleMath::Vector4::Zero;
    vHead.x = aSwingInput.GetFirst();
    //vHead.w = aSwingInput.GetFirst();

    DirectX::SimpleMath::Vector3 vFaceNormal = DirectX::SimpleMath::Vector4::Zero;
    vFaceNormal.x = cos(Utility::ToRadians(aSwingInput.GetY()));
    vFaceNormal.y = sin(Utility::ToRadians(aSwingInput.GetY()));
    vFaceNormal.Normalize();
    DirectX::SimpleMath::Vector3 vHeadNormalized = (vHead * vFaceNormal) * vFaceNormal;
    DirectX::SimpleMath::Vector3 vHeadParallel = vHead - vHeadNormalized;

    double ballMass = m_ball.mass;
    double clubMass = aSwingInput.GetZ();
    double e = aSwingInput.GetW(); //  coefficient of restitution of club face striking the ball

    double div1 = ((1.0 + e) * clubMass) / (clubMass + ballMass);
    double div2 = (2 * clubMass) / (7 * (clubMass + ballMass));

    DirectX::SimpleMath::Vector3 vBall = (div1 * vHeadNormalized) + (div2 * vHeadParallel);
    */

    DirectX::SimpleMath::Vector4 impactVector = CalculateImpactVector(aSwingInput.GetX(), Utility::ToRadians(aSwingInput.GetY()), 0.0);
    
    //  Convert the loft angle from degrees to radians and
    //  assign values to some convenience variables.
    double loft = Utility::ToRadians(aSwingInput.GetY());
    double cosL = cos(loft);
    double sinL = sin(loft);

    //  Calculate the pre-collision velocities normal
    //  and parallel to the line of action.
    double velocity = aSwingInput.GetX();
    double vcp = cosL * velocity;
    double vcn = -sinL * velocity;

    //  Compute the post-collision velocity of the ball
    //  along the line of action.

    double vbp = (1.0 + e) * clubMass * vcp / (clubMass + ballMass);

    //  Compute the post-collision velocity of the ball
    //  perpendicular to the line of action.
    double vbn = (1.0 - m_faceRoll) * clubMass * vcn / (clubMass + ballMass);

    //  Compute the initial spin rate assuming ball is
    //  rolling without sliding.
    double radius = m_ball.radius;
    double omega = m_faceRoll * vcn / radius;

    std::cout << "omega = " << omega << std::endl;
    std::cout << "vcn = " << vcn << std::endl;
    std::cout << "radius = " << radius << std::endl;
    std::cout << "m_ball.radius = " << m_ball.radius << std::endl;

    //  Rotate post-collision ball velocities back into 
    //  standard Cartesian frame of reference. Because the
    //  line-of-action was in the xy plane, the z-velocity
    //  is zero.
    double vx0 = cosL * vbp - sinL * vbn;
    double vy0 = sinL * vbp + cosL * vbn;
    double vz0 = 0.0;

    printf("vx0=%lf  vy0=%lf  vz0=%lf  omega=%lf\n", vx0, vy0, vz0, omega);


    //  Load the initial ball velocities into the 
    //  SpinProjectile struct.
    /*
    m_ball.omega = omega;  
    m_ball.q[0] = vx0;   //  vx 
    m_ball.q[2] = vy0;   //  vy 
    m_ball.q[4] = vz0;   //  vz 
    */
    SetSpinAxis(omegaBall);
    //m_ball.omega = omegaBall.x;
    double omegaTest = Utility::ToDegrees(omegaBall.z);

    m_ball.omega = omega;
    m_ball.q[0] = vBall.x;   //  vx 
    m_ball.q[2] = vBall.y;   //  vy 
    m_ball.q[4] = vBall.z;   //  vz 
}

void GolfBall::PushFlightData()
{
    if (m_ball.q[3] > m_ball.landingHeight)
    {
        m_xVals.push_back(m_ball.q[1]);
        m_yVals.push_back(m_ball.q[3]);
        m_zVals.push_back(m_ball.q[5]);
    }
}

void GolfBall::PrintFlightData()
{
    printf("Time = %.1f sec, X = %f m, Y = %f m, Z = %f m, delta X = %f m/s, delta Y = %f m/s\n",
        m_ball.flightTime, m_ball.q[1], m_ball.q[3], m_ball.q[5], m_ball.q[0], m_ball.q[2]);

    m_xVals.push_back(m_ball.q[1]);
    m_yVals.push_back(m_ball.q[3]);
    m_zVals.push_back(m_ball.q[5]);
}

void GolfBall::PrintLandingData(Vector4d aLandingData, double aMaxY)
{
    double distanceXinYards = aLandingData.GetX() * 1.0936; // 1.0936 is the number of yards in a meter
    printf("================================== Final Flight Results ====================================\n");
    printf(" Flight Time                                       : %g (seconds) \n", aLandingData.GetW());
    printf(" Carry Distance                                    : %g (meters) (%g yards) \n", aLandingData.GetX(), distanceXinYards);
    printf(" Landing Height                                    : %g (meters) \n", aLandingData.GetY());
    printf(" Max Height                                        : %g (meters) \n", aMaxY);
    printf(" Landing Position z                                : %g (meters) \n", m_ball.q[5]);
    printf(" Landing Velocity x                                : %g (m/s) \n", m_ball.q[0]);
    printf(" Landing Velocity y                                : %g (m/s) \n", aLandingData.GetZ());
    printf(" Landing Velocity z                                : %g (m/s) \n", m_ball.q[4]);
    printf("============================================================================================\n");
}

//  This method loads the right-hand sides for the projectile ODEs
void GolfBall::ProjectileRightHandSide(struct SpinProjectile* pBall,
    double* q, double* deltaQ, double ds,
    double qScale, double* dq)
{
    //  Compute the intermediate values of the 
    //  dependent variables.
    double newQ[6]; // intermediate dependent variable values.
    for (int i = 0; i < 6; ++i)
    {
        newQ[i] = q[i] + qScale * deltaQ[i];
    }

    //  Declare some convenience variables representing
    //  the intermediate values of velocity.
    double vx = newQ[0];
    double vy = newQ[2];
    double vz = newQ[4];

    //  Compute the apparent velocities bz subtracting
    //  the wind velocity components from the projectile
    //  velocity components.
    double vax = vx - pBall->windVx;
    double vay = vy - pBall->windVy;
    double vaz = vz - pBall->windVz;

    //  Compute the apparent velocity magnitude. The 1.0e-8 term
    //  ensures there won't be a divide bz yero later on
    //  if all of the velocity components are zero.
    double va = sqrt(vax * vax + vay * vay + vaz * vaz) + 1.0e-8;

    //  Compute the total drag force.
    double Fd = 0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va;
    double Fdx = -Fd * vax / va;
    double Fdy = -Fd * vay / va;
    double Fdz = -Fd * vaz / va;

    //  Compute the velocity magnitude
    double v = sqrt(vx * vx + vy * vy + vz * vz) + 1.0e-8;

    //  Evaluate the Magnus force terms.
    double Cl = -0.05 + sqrt(0.0025 + 0.36 * fabs(pBall->radius * pBall->omega / v));  // this equation gives a more accurate representation to fit experimental data than Cl = (radius * omega)/v
    double Fm = 0.5 * pBall->airDensity * pBall->area * Cl * v * v;
    double Fmx = (vz * pBall->ry - pBall->rz * vy) * Fm / v;
    double Fmy = (vx * pBall->rz - pBall->rx * vz) * Fm / v;
    double Fmz = -(vx * pBall->ry - pBall->rx * vy) * Fm / v;

    //  Compute right-hand side values.
    dq[0] = ds * (Fdx + Fmx) / pBall->mass;
    dq[1] = ds * vx;
    dq[2] = ds * (pBall->gravity + (Fdy + Fmy) / pBall->mass);
    dq[3] = ds * vy;
    dq[4] = ds * (Fdz + Fmz) / pBall->mass;
    dq[5] = ds * vz;
}

void GolfBall::ProjectileRungeKutta4(struct SpinProjectile* pBall, double aTimeDelta)
{
    int numEqns = pBall->numEqns;
    //  Allocate memory for the arrays.    
    std::vector<double> vecQ(numEqns);
    std::vector<double> vecDq1(numEqns);
    std::vector<double> vecDq2(numEqns);
    std::vector<double> vecDq3(numEqns);
    std::vector<double> vecDq4(numEqns);
    double* pQ = vecQ.data();
    double* pQ1 = vecDq1.data();
    double* pQ2 = vecDq2.data();
    double* pQ3 = vecDq3.data();
    double* pQ4 = vecDq4.data();

    //  Retrieve the current values of the dependent
    //  and independent variables.
    for (int i = 0; i < numEqns; ++i)
    {
        pQ[i] = pBall->q[i];
    }

    // Compute the four Runge-Kutta steps, The return 
    // value of projectileRightHandSide method is an array
    // of delta-q values for each of the four steps.   
    ProjectileRightHandSide(pBall, pQ, pQ, aTimeDelta, 0.0, pQ1);
    ProjectileRightHandSide(pBall, pQ, pQ1, aTimeDelta, 0.5, pQ2);
    ProjectileRightHandSide(pBall, pQ, pQ2, aTimeDelta, 0.5, pQ3);
    ProjectileRightHandSide(pBall, pQ, pQ3, aTimeDelta, 1.0, pQ4);

    //  Update the dependent and independent variable values
    //  at the new dependent variable location and store the
    //  values in the ODE object arrays.
    pBall->flightTime = pBall->flightTime + aTimeDelta;
    for (int i = 0; i < numEqns; ++i)
    {
        pQ[i] = pQ[i] + (pQ1[i] + 2.0 * pQ2[i] + 2.0 * pQ3[i] + pQ4[i]) / numEqns;
        pBall->q[i] = pQ[i];
    }

    //  Free up memory   
    pQ = nullptr;
    pQ1 = nullptr;
    pQ2 = nullptr;
    pQ3 = nullptr;
    pQ4 = nullptr;
    delete pQ;
    delete pQ1;
    delete pQ2;
    delete pQ3;
    delete pQ4;

    UpdateSpinRate(aTimeDelta);
}

void GolfBall::ResetBallData()
{
    m_xVals.clear();
    m_yVals.clear();
    m_zVals.clear();
    m_timeStep = 0.1f;
    m_ball.flightTime = 0.0;
    m_ball.omega = 0.0;
    m_ball.q[0] = 0.0;   //  vx = 0.0
    m_ball.q[1] = 0.0;   //  x  = 0.0
    m_ball.q[4] = 0.0;   //  vz = 0.0
    m_ball.q[5] = 0.0;   //  z  = 0.0
    m_ball.q[2] = 0.0;   //  vy = 0.0
    m_ball.q[3] = 0.0;   //  y  = 0.0
}

void GolfBall::SetDefaultBallValues(Environment* pEnviron)
{
    m_timeStep = 0.1f;
    m_ball.airDensity = pEnviron->GetAirDensity();
    m_ball.area = 0.001432;
    m_ball.dragCoefficient = 0.22;
    m_ball.flightTime = 0.0;      //  time = 0.0
    m_ball.gravity = pEnviron->GetGravity();
    m_ball.launchHeight = pEnviron->GetLauchHeight();
    m_ball.landingHeight = pEnviron->GetLandingHeight();
    m_ball.radius = 0.02135;
    m_ball.mass = 0.0459;
    m_ball.numEqns = 6;
    m_ball.omega = 0.0;
    m_ball.q[0] = 0.0;   //  vx = 0.0
    m_ball.q[1] = 0.0;   //  x  = 0.0
    m_ball.q[4] = 0.0;   //  vz = 0.0
    m_ball.q[5] = 0.0;   //  z  = 0.0
    m_ball.q[2] = 0.0;   //  vy = 0.0
    m_ball.q[3] = 0.0;   //  y  = 0.0
    m_ball.rx = 0.0;
    m_ball.ry = 0.0;
    m_ball.rz = 1.0; // ball will only be spinning about the z axis, this will need to be adjusted if/when imperfect impact mechanics added for hooks and slices
    m_ball.windVx = pEnviron->GetWindX();
    m_ball.windVy = pEnviron->GetWindY();
    m_ball.windVz = pEnviron->GetWindZ();
}

void GolfBall::SetLandingCordinates(const double aX, const double aY, const double aZ)
{
    m_landingCordinates.SetW(0.0);
    m_landingCordinates.SetX(aX);
    m_landingCordinates.SetY(aY);
    m_landingCordinates.SetZ(aZ);
}

void GolfBall::SetSpinAxis(DirectX::SimpleMath::Vector4 aAxis)
{
    float spinTotal = abs(aAxis.x) + abs(aAxis.y) + abs(aAxis.z);
    const float tolerance = 0.000001;
    if (spinTotal > tolerance)
    {
        m_ball.rx = aAxis.x / spinTotal;
        m_ball.ry = aAxis.y / spinTotal;
        m_ball.rz = aAxis.z / spinTotal;
    }
    else
    {
        m_ball.rx = 0.0;
        m_ball.ry = 0.0;
        m_ball.rz = 0.0;
    }
}

void GolfBall::UpdateSpinRate(double aTimeDelta)
{
    m_ball.omega *= 1.0 - (aTimeDelta * m_spinRateDecay);
}

void GolfBall::OutputPosition()
{
    /*
    printf("X Vals ================ \n");
    for (int i = 0; i < m_xVals.size(); ++i)
    {
        std::cout << m_xVals[i] << std::endl;
    }
    printf("\n\n\n");
    printf("Y Vals ================ \n");
    for (int i = 0; i < m_yVals.size(); ++i)
    {
        std::cout << m_yVals[i] << std::endl;
    }
    */
}

std::vector<double> GolfBall::OutputXvals()
{
    return m_xVals;
}

std::vector<double> GolfBall::OutputYvals()
{
    return m_yVals;
}

std::vector<double> GolfBall::OutputZvals()
{
    return m_zVals;
}

const DirectX::SimpleMath::Vector3 GolfBall::GetImpactAngle()
{
    DirectX::SimpleMath::Plane impactPlane = GetImpactPlane();
    
    DirectX::SimpleMath::Vector3 impactPoint;
    impactPoint.x = m_landingCordinates.GetX();
    impactPoint.y = m_landingCordinates.GetY();
    impactPoint.z = m_landingCordinates.GetZ();
    DirectX::SimpleMath::Vector3 impactMinus1;
    impactMinus1.x = m_xVals[m_xVals.size() - 1];
    impactMinus1.y = m_yVals[m_yVals.size() - 1];
    impactMinus1.z = m_zVals[m_zVals.size() - 1];
    DirectX::SimpleMath::Vector3 impactMinus2;
    impactMinus2.x = m_xVals[m_xVals.size() - 2];
    impactMinus2.y = m_yVals[m_yVals.size() - 2];
    impactMinus2.z = m_zVals[m_zVals.size() - 2];
    DirectX::SimpleMath::Vector3 impactAngle;
    impactAngle.Zero;
    impactAngle = impactMinus1 - impactPoint;
    DirectX::SimpleMath::Vector3 impact2 = impactAngle;
    impact2.Normalize();

    DirectX::SimpleMath::Vector3 horizontalVec;
    horizontalVec.Zero;
    horizontalVec.x = impactAngle.x;
    horizontalVec.z = impactAngle.z;

    float aDotB = impactAngle.Dot(horizontalVec);
    float c = (impactAngle.x * impactAngle.x) + (impactAngle.y * impactAngle.y) + (impactAngle.z * impactAngle.z);
    float d = (horizontalVec.x * horizontalVec.x) + (horizontalVec.y * horizontalVec.y) + (horizontalVec.z * horizontalVec.z);
    float e = aDotB / (sqrt(c) * sqrt(d));
    float f = acos(e);
    float g = Utility::ToDegrees(f);

    int test = 8;
    return impactAngle;
}

const DirectX::SimpleMath::Plane GolfBall::GetImpactPlane()
{
    DirectX::SimpleMath::Vector3 a, b;
    a.Zero;
    b.Zero;
    //a.y = m_landingCordinates.GetY();
    a.y = -1.08899951;
    b.y = a.y;
    a.x = 1.0;
    b.z = 1.0;

    DirectX::SimpleMath::Vector3 c;
    c.Zero;
    //c.x = -1.0;

    b.Cross(a, c);

    DirectX::SimpleMath::Plane impactPlane = DirectX::SimpleMath::Plane(c, a);

    return impactPlane;
}

double GolfBall::GetIndexX(const int aIndex)
{
    if (aIndex > m_xVals.size())
    {
        std::cerr << "out of range error in GolfBall::GetIndex_ \n";
    }
    else
    {
        return m_xVals[aIndex];
    }
}

double GolfBall::GetIndexY(const int aIndex)
{
    if (aIndex > m_yVals.size())
    {
        std::cerr << "out of range error in GolfBall::GetIndex_ \n";
    }
    else
    {
        return m_zVals[aIndex];
    }
}

double GolfBall::GetIndexZ(const int aIndex)
{
    if (aIndex > m_zVals.size())
    {
        std::cerr << "out of range error in GolfBall::GetIndex_ \n";
    }
    else
    {
        return m_zVals[aIndex];
    }
}

const double GolfBall::GetShotDistance()
{
    Vector4d origin = m_shotOrigin;
    Vector4d landingPos = GetLandingCordinates();
    double distance = sqrt(((landingPos.GetX() - origin.GetX()) * (landingPos.GetX() - origin.GetX())) + ((landingPos.GetY() - origin.GetY())
        * (landingPos.GetY() - origin.GetY()) + ((landingPos.GetZ() - origin.GetZ()) * (landingPos.GetZ() - origin.GetZ()))));
    return distance;
}