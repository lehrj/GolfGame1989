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

void GolfBall::FireProjectile(Utility::ImpactData aImpactData, Environment* pEnviron)
{   
    PrepProjectileLaunch(aImpactData);
    LaunchProjectile();
    LandProjectile();
}

void GolfBall::LandProjectile()
{
    /*
    if (m_drawColorIndex == 0)
    {
        m_drawColorVector.push_back(m_shotPath.size());
    }
    if (m_drawColorIndex == 1)
    {
        m_drawColorVector.push_back(m_shotPath.size());
    }
    if (m_drawColorIndex == 2)
    {
        m_drawColorVector.push_back(m_shotPath.size());
    }
    if (m_drawColorIndex == 3)
    {
        m_drawColorVector.push_back(m_shotPath.size());
    }
    m_drawColorIndex++;
    */
    //DirectX::SimpleMath::Vector3 impactAngle = GetImpactAngle();
    float direction = GetImpactDirection();
    float impactAngle = GetImpactAngle();
    float impactVelocity = GetImpactVelocity();
    //float vix = m_ball.q[0];
    float vix = m_ball.q.velocity.x;
    float viy = m_ball.q.velocity.y;
    
    Vector4d impactVector{ m_ball.q.velocity.x, m_ball.q.velocity.y, m_ball.q.velocity.z, 0.0 }; // vx, vy, vz

    double impactSpeed = impactVector.Magnitude3();
    //printf("Impact Speed = %lf (m/s) \n", impactSpeed);
    impactVector.NormalizeVector(impactVector);

    float phi = atan(abs(m_ball.q.velocity.x / m_ball.q.velocity.y));
    //?c = 15.4?(vi / (impact speed))(? / (impact angle))
    float thetaC = 15.4;
    
    float vixPrime = vix * cos(thetaC) - abs(viy) * sin(thetaC);
    float absViyPrime = vix * sin(thetaC) + abs(viy) * cos(thetaC);

    float e;
    if (absViyPrime <= 20.0)
    {
        e = 0.510 - 0.0375 * absViyPrime + 0.000903 * absViyPrime * absViyPrime;
    }
    else
    {
        e = 0.120;
    }
    float mu = 0.43; // (greek u symbol) from Danish equation from green impact, WLJ will need to tweek as its based off type of terrain
    float muC = (2 * (vixPrime + (m_ball.radius * m_ball.omega))) / (7 * (1.0 + e) * absViyPrime);

    float vrxPrime, vryPrime, omegaR;

    // If mu is less than the critical value muC the ball will slide throughout the impact
    // If mu is great than muC the ball will roll out of the collision
    if (mu < muC) 
    {
        vrxPrime = vixPrime - mu * absViyPrime * (1.0 + e);
        vryPrime = e * absViyPrime;
        omegaR = m_ball.omega - ((5.0 * mu) / (2.0 * m_ball.radius)) * absViyPrime * (1.0 + e);
    }
    else
    {
        vrxPrime = (5.0 / 7.0) * vixPrime - (2.0 / 7.0) * m_ball.radius * m_ball.omega;
        vryPrime = e * absViyPrime;
        omegaR = -vrxPrime / m_ball.radius;
    }

    float vrx = vrxPrime * cos(thetaC) - vryPrime * sin(thetaC);
    //float vry = vrxPrime * sin(thetaC) + vryPrime * cos(thetaC);
    float vry = vryPrime * sin(thetaC) + vryPrime * cos(thetaC);

    m_ball.q.velocity.x = vrx;
    m_ball.q.velocity.y = vry;
    m_ball.q.velocity.z = 0.0;

    m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(-direction));
    m_ball.omega = omegaR;
    
    /*
    float minSpeed = .1;
    if (m_ball.q.velocity.x > minSpeed || m_ball.q.velocity.y > minSpeed)
    {
        LaunchProjectile2();
    }
    */
}

void GolfBall::RollBall()
{
    float pg = 0.15;
    float g = m_ball.gravity;
    float a = -(5.0 / 7.0) * pg * g;

    m_ball.q.velocity.x -= a;
}

void GolfBall::LaunchProjectile()
{
    PushFlightData();
     
    // Fly ball on an upward trajectory until it stops climbing
    BallMotion flightData;
    double dt = m_timeStep;
    double maxHeight = m_ball.launchHeight;
    double time = 0.0;
    SetInitialSpinRate(m_ball.omega);
    
    int count = 0;
    bool isBallFlyOrBounce = true;
    while (isBallFlyOrBounce == true)
    {
        bool isBallAscending = true;
        while (isBallAscending == true)
        {
            ProjectileRungeKutta4(&m_ball, dt);
            UpdateSpinRate(dt);
            flightData = this->m_ball.q;
            PushFlightData();

            if (m_ball.q.velocity.y < 0.0)
            {
                maxHeight = m_ball.q.position.y;
                isBallAscending = false;
            }
        }

        double previousY = flightData.position.y;
        double previousTime = m_ball.flightTime;

        //  Calculate ball decent path until it reaches landing area height
        while (m_ball.q.position.y + m_ball.launchHeight >= m_ball.landingHeight)
        {
            previousY = flightData.position.y;
            previousTime = m_ball.flightTime;
            ProjectileRungeKutta4(&m_ball, dt);
            UpdateSpinRate(dt);
            flightData = this->m_ball.q;
            //PrintFlightData();
            PushFlightData();
            time = m_ball.flightTime;
        }
        double rollBackTime = CalculateImpactTime(previousTime, time, previousY, m_ball.q.position.y);
        ProjectileRungeKutta4(&m_ball, -rollBackTime);
        PushFlightData();
        //flightData.SetAll(m_ball.q.position.x, m_ball.q.position.x, m_ball.q.velocity.y, m_ball.flightTime - rollBackTime);
        //m_shotPath[m_shotPath.size() - 1] = m_ball.q.position;
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
        //SetLandingCordinates(m_ball.q.position);
        SetLandingSpinRate(m_ball.omega);
        SetMaxHeight(maxHeight);
        ++m_bounceCount;
        
        LandProjectile();
        ++count;
        if (m_ball.q.velocity.y < 1.0 || count > 5)
        {
            isBallFlyOrBounce = false;
        } 
    }
    
    m_drawColorVector.push_back(m_shotPath.size());  
    m_drawColorIndex++;

    this->m_ball.q.velocity.y = 0.0;
    m_ball.q.position.y = 0.0;
    bool isBallRolling = true;
    while (isBallRolling == true)
    {

        RollRungeKutta4(&m_ball, dt);
        
        m_ball.q.velocity.y = 0.0;
        m_ball.q.position.y = 0.0;
        m_ball.q.velocity.z = 0.0;
        m_ball.q.position.z = 0.0;
        /*
        float pg = 1.2;
        float g = m_ball.gravity;
        float a = -(5.0 / 7.0) * pg * g;

        a = a * m_timeStep;
        a = 0.6;
        this->m_ball.q.velocity.x = (a / dt) * this->m_ball.q.velocity.x;
        this->m_ball.q.velocity.z = a * this->m_ball.q.velocity.z;
        this->m_ball.q.velocity.y = 0.0;
        m_ball.q.position.y = 0.0;
        //RollRungeKutta4(&m_ball, dt);
        m_ball.q.position += m_ball.q.velocity;
        flightData = m_ball.q;
        */
        //PrintFlightData();
        PushFlightData();
        if (m_ball.q.velocity.x < 0.1)
        {
            isBallRolling = false;
        }
    }
    SetLandingCordinates(m_ball.q.position);
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

void GolfBall::PrepProjectileLaunch(Utility::ImpactData aImpactData)
{
    //DirectX::SimpleMath::Vector4 impactVector = CalculateImpactVector(aSwingInput.GetX(), Utility::ToRadians(aSwingInput.GetY()), 0.0);
    
    //  Convert the loft angle from degrees to radians and
    //  assign values to some convenience variables.
    double loft = Utility::ToRadians(aImpactData.angleY);
    double cosL = cos(loft);
    double sinL = sin(loft);

    //  Calculate the pre-collision velocities normal
    //  and parallel to the line of action.
    double velocity = aImpactData.velocity;
    double vcp = cosL * velocity;
    double vcn = -sinL * velocity;

    //  Compute the post-collision velocity of the ball
    //  along the line of action.
    double ballMass = m_ball.mass;
    double clubMass = aImpactData.mass;
    double e = aImpactData.cor; //  coefficient of restitution of club face striking the ball
    double vbp = (1.0 + e) * clubMass * vcp / (clubMass + ballMass);

    //  Compute the post-collision velocity of the ball
    //  perpendicular to the line of action.
    double vbn = (1.0 - m_faceRoll) * clubMass * vcn / (clubMass + ballMass);

    //  Compute the initial spin rate assuming ball is
    //  rolling without sliding.
    double radius = m_ball.radius;
    double omega = m_faceRoll * vcn / radius;

    //  Rotate post-collision ball velocities back into 
    //  standard Cartesian frame of reference. Because the
    //  line-of-action was in the xy plane, the z-velocity
    //  is zero.
    double vx0 = cosL * vbp - sinL * vbn;
    double vy0 = sinL * vbp + cosL * vbn;
    double vz0 = 0.0;
    /*
    double velocity = aImpactData.velocity;
    double ballMass = m_ball.mass;
    double clubMass = aImpactData.mass;
    double e = aImpactData.cor; //  coefficient of restitution of club face striking the ball
    double radius = m_ball.radius;
    */

    double div1 = ((1.0 + e) * clubMass) / (clubMass + ballMass);
    double div2 = (2 * clubMass) / (7 * (clubMass + ballMass));

    DirectX::SimpleMath::Vector4 vBall = ((((1.0 + e) * clubMass) / (clubMass + ballMass)) * aImpactData.vHeadNormal)
        + (((2 * clubMass) / (7 * (clubMass + ballMass))) * aImpactData.vHeadParallel);
    DirectX::SimpleMath::Vector4 vBall1 = (((1.0 + e) * clubMass) / (clubMass + ballMass) * aImpactData.vHeadNormal);
    DirectX::SimpleMath::Vector4 vBall2 = ((2 * clubMass) / (7 * (clubMass + ballMass)) * aImpactData.vHeadParallel);
    DirectX::SimpleMath::Vector4 vBall3 = vBall1 + vBall2;

    DirectX::SimpleMath::Vector3 unitVHead = aImpactData.vHead;
    unitVHead.Normalize();
    DirectX::SimpleMath::Vector3 unitFaceNormal = aImpactData.vFaceNormal;
    unitFaceNormal.Normalize();

    DirectX::SimpleMath::Vector4 crossVheadvFace = unitVHead.Cross(unitFaceNormal);

    float absVhP = sqrt((aImpactData.vHeadParallel.x * aImpactData.vHeadParallel.x) 
        + (aImpactData.vHeadParallel.y * aImpactData.vHeadParallel.y) 
        + (aImpactData.vHeadParallel.z * aImpactData.vHeadParallel.z));

    DirectX::SimpleMath::Vector4 omegaBall = DirectX::SimpleMath::Vector4::Zero;
    omegaBall = ((5.0 * absVhP) / (7.0 * m_ball.radius)) * crossVheadvFace;

    float absOmegaBall = sqrt((omegaBall.x * omegaBall.x)
        + (omegaBall.y * omegaBall.y)
        + (omegaBall.z * omegaBall.z));
    float absvBall = sqrt((vBall.x * vBall.x)
        + (vBall.y * vBall.y)
        + (vBall.z * vBall.z));

    double cL = -0.05 + sqrt(0.0025 + 0.36 * ((m_ball.radius * absOmegaBall) / absvBall));
    DirectX::SimpleMath::Vector3 fMangus;
    fMangus.Zero;
    fMangus = (.5 * m_ball.airDensity * m_ball.area * cL * absvBall * absvBall) * (unitFaceNormal.Cross(unitVHead));

    DirectX::SimpleMath::Vector3 normfManus = fMangus;
    DirectX::SimpleMath::Vector3 normOmegaBall = omegaBall;

    normfManus.Normalize();
    normOmegaBall.Normalize();

    //SetSpinAxis(omegaBall);

    m_ball.omega = omega;
    //m_ball.omega = omegaBall.z;
   
    m_ball.q.velocity.x = vBall.x;   //  vx 
    m_ball.q.velocity.y = vBall.y;   //  vy 
    m_ball.q.velocity.z = vBall.z;   //  vz 
}

void GolfBall::PushFlightData()
{
    // Prevent push of data below ground level
    if (m_ball.q.position.y >= m_ball.landingHeight)
    {
        m_shotPath.push_back(m_ball.q.position);
        m_shotPathTimeStep.push_back(m_ball.flightTime);
    }
    //m_shotPath.push_back(m_ball.q.position);
}

void GolfBall::PrintFlightData()
{
    printf("Time = %.1f sec, X = %f m, Y = %f m, Z = %f m, delta X = %f m/s, delta Y = %f m/s\n",
        m_ball.flightTime, m_ball.q.position.x, m_ball.q.position.y, m_ball.q.position.z, m_ball.q.velocity.x, m_ball.q.velocity.y);
    //m_shotPath.push_back(m_ball.q.position);
}

//  This method loads the right-hand sides for the projectile ODEs
//void GolfBall::ProjectileRightHandSide(struct SpinProjectile* pBall, double* q, double* deltaQ, double ds, double qScale, double* dq)
void GolfBall::ProjectileRightHandSide(struct SpinProjectile* pBall, BallMotion* q, BallMotion* deltaQ, double ds, double qScale, BallMotion* dq)
{
    //  Compute the intermediate values of the 
    //  dependent variables.
    BallMotion newQ;
    newQ.position.x = q->position.x + qScale * deltaQ->position.x;
    newQ.position.y = q->position.y + qScale * deltaQ->position.y;
    newQ.position.z = q->position.z + qScale * deltaQ->position.z;
    newQ.velocity.x = q->velocity.x + qScale * deltaQ->velocity.x;
    newQ.velocity.y = q->velocity.y + qScale * deltaQ->velocity.y;
    newQ.velocity.z = q->velocity.z + qScale * deltaQ->velocity.z;

    //  Declare some convenience variables representing
    //  the intermediate values of velocity.
    double vx = newQ.velocity.x;
    double vy = newQ.velocity.y;
    double vz = newQ.velocity.z;

    //  Compute the apparent velocities bz subtracting
    //  the wind velocity components from the projectile
    //  velocity components.
    double vax = vx - pBall->windSpeed.x;
    double vay = vy - pBall->windSpeed.y;
    double vaz = vz - pBall->windSpeed.z;

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
    double Fmx = (vz * pBall->rotationAxis.y - pBall->rotationAxis.z * vy) * Fm / v;
    double Fmy = (vx * pBall->rotationAxis.z - pBall->rotationAxis.x * vz) * Fm / v;
    double Fmz = -(vx * pBall->rotationAxis.y - pBall->rotationAxis.x * vy) * Fm / v;

    //  Compute right-hand side values.
    dq->velocity.x = ds * (Fdx + Fmx) / pBall->mass;
    dq->position.x = ds * vx;
    dq->velocity.y = ds * (pBall->gravity + (Fdy + Fmy) / pBall->mass);
    dq->position.y = ds * vy;
    dq->velocity.z = ds * (Fdz + Fmz) / pBall->mass;
    dq->position.z = ds * vz;
}

void GolfBall::ProjectileRungeKutta4(struct SpinProjectile* pBall, double aTimeDelta)
{
    int numEqns = pBall->numEqns;
    BallMotion pQ;
    BallMotion pQ1;
    BallMotion pQ2;
    BallMotion pQ3;
    BallMotion pQ4;

    //  Retrieve the current values of the dependent
    //  and independent variables.
    pQ.position.x = pBall->q.position.x;
    pQ.position.y = pBall->q.position.y;
    pQ.position.z = pBall->q.position.z;
    pQ.velocity.x = pBall->q.velocity.x;
    pQ.velocity.y = pBall->q.velocity.y;
    pQ.velocity.z = pBall->q.velocity.z;

    // Compute the four Runge-Kutta steps, The return 
    // value of projectileRightHandSide method is an array
    // of delta-q values for each of the four steps.   
    ProjectileRightHandSide(pBall, &pQ, &pQ, aTimeDelta, 0.0, &pQ1);
    ProjectileRightHandSide(pBall, &pQ, &pQ1, aTimeDelta, 0.5, &pQ2);
    ProjectileRightHandSide(pBall, &pQ, &pQ2, aTimeDelta, 0.5, &pQ3);
    ProjectileRightHandSide(pBall, &pQ, &pQ3, aTimeDelta, 1.0, &pQ4);

    //  Update the dependent and independent variable values
    //  at the new dependent variable location and store the
    //  values in the ODE object arrays.
    pBall->flightTime = pBall->flightTime + aTimeDelta;

    pQ.position.x = pQ.position.x + (pQ1.position.x + 2.0 * pQ2.position.x + 2.0 * pQ3.position.x + pQ4.position.x) / numEqns;
    pBall->q.position.x = pQ.position.x;
    pQ.position.y = pQ.position.y + (pQ1.position.y + 2.0 * pQ2.position.y + 2.0 * pQ3.position.y + pQ4.position.y) / numEqns;
    pBall->q.position.y = pQ.position.y;
    pQ.position.z = pQ.position.z + (pQ1.position.z + 2.0 * pQ2.position.z + 2.0 * pQ3.position.z + pQ4.position.z) / numEqns;
    pBall->q.position.z = pQ.position.z;
    pQ.velocity.x = pQ.velocity.x + (pQ1.velocity.x + 2.0 * pQ2.velocity.x + 2.0 * pQ3.velocity.x + pQ4.velocity.x) / numEqns;
    pBall->q.velocity.x = pQ.velocity.x;
    pQ.velocity.y = pQ.velocity.y + (pQ1.velocity.y + 2.0 * pQ2.velocity.y + 2.0 * pQ3.velocity.y + pQ4.velocity.y) / numEqns;
    pBall->q.velocity.y = pQ.velocity.y;
    pQ.velocity.z = pQ.velocity.z + (pQ1.velocity.z + 2.0 * pQ2.velocity.z + 2.0 * pQ3.velocity.z + pQ4.velocity.z) / numEqns;
    pBall->q.velocity.z = pQ.velocity.z;
}

void GolfBall::ProjectileRungeKutta4wPointers(struct SpinProjectile* pBall, double aTimeDelta)
{
    int numEqns = pBall->numEqns;
    //  Allocate memory for the arrays.

    BallMotion vecQ;
    BallMotion vecDq1;
    BallMotion vecDq2;
    BallMotion vecDq3;
    BallMotion vecDq4;

    BallMotion* pQ = &vecQ;
    BallMotion* pQ1 = &vecDq1;
    BallMotion* pQ2 = &vecDq2;
    BallMotion* pQ3 = &vecDq3;
    BallMotion* pQ4 = &vecDq4;

    //  Retrieve the current values of the dependent
    //  and independent variables.
    pQ->position.x = pBall->q.position.x;
    pQ->position.y = pBall->q.position.y;
    pQ->position.z = pBall->q.position.z;
    pQ->velocity.x = pBall->q.velocity.x;
    pQ->velocity.y = pBall->q.velocity.y;
    pQ->velocity.z = pBall->q.velocity.z;

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
    pQ->position.x = pQ->position.x + (pQ1->position.x + 2.0 * pQ2->position.x + 2.0 * pQ3->position.x + pQ4->position.x) / numEqns;
    pBall->q.position.x = pQ->position.x;
    pQ->position.y = pQ->position.y + (pQ1->position.y + 2.0 * pQ2->position.y + 2.0 * pQ3->position.y + pQ4->position.y) / numEqns;
    pBall->q.position.y = pQ->position.y;
    pQ->position.z = pQ->position.z + (pQ1->position.z + 2.0 * pQ2->position.z + 2.0 * pQ3->position.z + pQ4->position.z) / numEqns;
    pBall->q.position.z = pQ->position.z;
    pQ->velocity.x = pQ->velocity.x + (pQ1->velocity.x + 2.0 * pQ2->velocity.x + 2.0 * pQ3->velocity.x + pQ4->velocity.x) / numEqns;
    pBall->q.velocity.x = pQ->velocity.x;
    pQ->velocity.y = pQ->velocity.y + (pQ1->velocity.y + 2.0 * pQ2->velocity.y + 2.0 * pQ3->velocity.y + pQ4->velocity.y) / numEqns;
    pBall->q.velocity.y = pQ->velocity.y;
    pQ->velocity.z = pQ->velocity.z + (pQ1->velocity.z + 2.0 * pQ2->velocity.z + 2.0 * pQ3->velocity.z + pQ4->velocity.z) / numEqns;
    pBall->q.velocity.z = pQ->velocity.z;

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
    
    //UpdateSpinRate(aTimeDelta);
}

void GolfBall::ResetBallData()
{
    m_bounceCount = 0;
    m_drawColorIndex = 0;
    m_drawColorVector.clear();
    m_shotPath.clear();
    m_shotPathTimeStep.clear();
    m_timeStep = 0.1f;
    m_ball.flightTime = 0.0;
    m_ball.omega = 0.0;
    m_ball.q.velocity.x = 0.0;   //  vx = 0.0
    m_ball.q.position.x = 0.0;   //  x  = 0.0
    m_ball.q.velocity.z = 0.0;   //  vz = 0.0
    m_ball.q.position.z = 0.0;   //  z  = 0.0
    m_ball.q.velocity.y = 0.0;   //  vy = 0.0
    m_ball.q.position.y = 0.0;   //  y  = 0.0
}

void GolfBall::RollRightHandSide(struct SpinProjectile* pBall, BallMotion* q, BallMotion* deltaQ, double ds, double qScale, BallMotion* dq)
{
    //  Compute the intermediate values of the 
    //  dependent variables.
    BallMotion newQ;
    newQ.position.x = q->position.x + qScale * deltaQ->position.x;
    newQ.position.y = q->position.y + qScale * deltaQ->position.y;
    newQ.position.z = q->position.z + qScale * deltaQ->position.z;
    newQ.velocity.x = q->velocity.x + qScale * deltaQ->velocity.x;
    newQ.velocity.y = q->velocity.y + qScale * deltaQ->velocity.y;
    newQ.velocity.z = q->velocity.z + qScale * deltaQ->velocity.z;

    //  Declare some convenience variables representing
    //  the intermediate values of velocity.
    double vx = newQ.velocity.x;
    double vy = newQ.velocity.y;
    double vz = newQ.velocity.z;

    //  Compute the apparent velocities bz subtracting
    //  the wind velocity components from the projectile
    //  velocity components.
    //double vax = vx;// -pBall->windSpeed.x;
    //double vay = vy;// -pBall->windSpeed.y;
    //double vaz = vz;// -pBall->windSpeed.z;

    //  Compute the apparent velocity magnitude. The 1.0e-8 term
    //  ensures there won't be a divide bz yero later on
    //  if all of the velocity components are zero.
    //double va = sqrt(vax * vax + vay * vay + vaz * vaz) + 1.0e-8;

    // a = - (5/7) PgG = 0.840000093
    double a = 0.840000093;
    //  Compute the total drag force.
    /*
    double Fd = 0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va;
    double FdwithA = 0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va * a;
    double FdwithdivA = (0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va) / a;
    double FdwithplusA = 0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va + a;
    */
    //Fd = FdwithA;
    double Fd = 0.5;
        
    /*
    double Fdx = -Fd * vax / va;
    double Fdy = -Fd * vay / va;
    double Fdz = -Fd * vaz / va;

    double Fdx = (-Fd * vax / va) * a;
    double Fdy = (-Fd * vay / va) * a;
    double Fdz = (-Fd * vaz / va) * a;
    */
    //double Fdx = (-Fd * vax / va);
    //double Fdy = (-Fd * vay / va);
    //double Fdz = (-Fd * vaz / va);
    //Fdx = Fdx * a;
    //Fdy = Fdy * a;
    //Fdz = Fdz * a;
    //  Compute the velocity magnitude
    double v = sqrt(vx * vx + vy * vy + vz * vz) + 1.0e-8;

    //dq->velocity.x = ds * Fdx / pBall->mass;
    //dq->velocity.x = ds * vx * a / pBall->mass;;  
    //dq->velocity.y = ds * Fdy / pBall->mass;  
    //dq->velocity.z = ds * Fdz / pBall->mass;
    dq->position.x = ds * vx;
    dq->position.y = ds * vy;
    dq->position.z = ds * vz;
    //dq->position.z = ds * vz;
    //dq->velocity.x = vx * a;
    //dq->position.x = ds * vx;
    //dq->velocity.z = vx * a;
}

void GolfBall::RollRightHandSideOld(struct SpinProjectile* pBall, BallMotion* q, BallMotion* deltaQ, double ds, double qScale, BallMotion* dq)
{
    //  Compute the intermediate values of the 
    //  dependent variables.
    BallMotion newQ;
    newQ.position.x = q->position.x + qScale * deltaQ->position.x;
    newQ.position.y = q->position.y + qScale * deltaQ->position.y;
    newQ.position.z = q->position.z + qScale * deltaQ->position.z;
    newQ.velocity.x = q->velocity.x + qScale * deltaQ->velocity.x;
    newQ.velocity.y = q->velocity.y + qScale * deltaQ->velocity.y;
    newQ.velocity.z = q->velocity.z + qScale * deltaQ->velocity.z;

    //  Declare some convenience variables representing
    //  the intermediate values of velocity.
    double vx = newQ.velocity.x;
    double vy = newQ.velocity.y;
    double vz = newQ.velocity.z;

    //  Compute the apparent velocities bz subtracting
    //  the wind velocity components from the projectile
    //  velocity components.
    //double vax = vx;// -pBall->windSpeed.x;
    //double vay = vy;// -pBall->windSpeed.y;
    //double vaz = vz;// -pBall->windSpeed.z;

    //  Compute the apparent velocity magnitude. The 1.0e-8 term
    //  ensures there won't be a divide bz yero later on
    //  if all of the velocity components are zero.
    //double va = sqrt(vax * vax + vay * vay + vaz * vaz) + 1.0e-8;
    double va = newQ.velocity.Length();

    // a = - (5/7) PgG = 0.840000093
    double Fd = 0.16;
    //  Compute the total drag force.
    //double Fd = 0.5 * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va;
    
    double Fdx = -Fd * vx / va;
    //double Fdy = -Fd * vy / va;
    //double Fdz = (-Fd * vz) / va;
    /*
    double Fdx = (-Fd * vax / va) * a;
    double Fdy = (-Fd * vay / va) * a;
    double Fdz = (-Fd * vaz / va) * a;
    */
    //double Fdx = (-Fd * vax / va);
    //double Fdy = (-Fd * vay / va);
    //double Fdz = (-Fd * vaz / va);
    //Fdx = Fdx * a;
    //Fdy = Fdy * a;
    //Fdz = Fdz * a;
    //  Compute the velocity magnitude
    //double v = sqrt(vx * vx + vy * vy + vz * vz) + 1.0e-8;

    //dq->velocity.x = ds * Fdx / pBall->mass;
    //dq->velocity.x = ds * vx * a / pBall->mass;;  
    //dq->velocity.y = ds * (Fdy / pBall->mass);
    //dq->position.y = ds * vy;   
    //dq->velocity.z = ds * vz * a / pBall->mass;;
    //double a = 0.840000093;
    
    //dq->velocity.x = vx * a;
    //dq->position.x = ds * vx;
    //dq->velocity.z = vx * a;
    //dq->position.z = ds * vz;

    //double a = 0.840000093;
    dq->velocity.x = (ds * Fdx) / pBall->mass;
    dq->position.x = ds * vx;
    dq->velocity.y = (ds * Fdx) / pBall->mass;
    dq->position.y = ds * vy;
    dq->velocity.z = (ds * Fdx) / pBall->mass;
    dq->position.z = ds * vz;
}

void GolfBall::RollRungeKutta4(struct SpinProjectile* pBall, double aTimeDelta)
{
    int numEqns = pBall->numEqns;
    BallMotion pQ;
    BallMotion pQ1;
    BallMotion pQ2;
    BallMotion pQ3;
    BallMotion pQ4;

    //  Retrieve the current values of the dependent
    //  and independent variables.
    pQ.position.x = pBall->q.position.x;
    pQ.position.y = pBall->q.position.y;
    pQ.position.z = pBall->q.position.z;
    pQ.velocity.x = pBall->q.velocity.x;
    pQ.velocity.y = pBall->q.velocity.y;
    pQ.velocity.z = pBall->q.velocity.z;

    // Compute the four Runge-Kutta steps, The return 
    // value of projectileRightHandSide method is an array
    // of delta-q values for each of the four steps.   
    RollRightHandSideOld(pBall, &pQ, &pQ, aTimeDelta, 0.0, &pQ1);
    RollRightHandSideOld(pBall, &pQ, &pQ1, aTimeDelta, 0.5, &pQ2);
    RollRightHandSideOld(pBall, &pQ, &pQ2, aTimeDelta, 0.5, &pQ3);
    RollRightHandSideOld(pBall, &pQ, &pQ3, aTimeDelta, 1.0, &pQ4);

    //  Update the dependent and independent variable values
    //  at the new dependent variable location and store the
    //  values in the ODE object arrays.
    pBall->flightTime = pBall->flightTime + aTimeDelta;

    pQ.position.x = pQ.position.x + (pQ1.position.x + 2.0 * pQ2.position.x + 2.0 * pQ3.position.x + pQ4.position.x) / numEqns;
    pBall->q.position.x = pQ.position.x;
    pQ.position.y = pQ.position.y + (pQ1.position.y + 2.0 * pQ2.position.y + 2.0 * pQ3.position.y + pQ4.position.y) / numEqns;
    pBall->q.position.y = pQ.position.y;
    pQ.position.z = pQ.position.z + (pQ1.position.z + 2.0 * pQ2.position.z + 2.0 * pQ3.position.z + pQ4.position.z) / numEqns;
    pBall->q.position.z = pQ.position.z;
    pQ.velocity.x = pQ.velocity.x + (pQ1.velocity.x + 2.0 * pQ2.velocity.x + 2.0 * pQ3.velocity.x + pQ4.velocity.x) / numEqns;
    pBall->q.velocity.x = pQ.velocity.x;
    pQ.velocity.y = pQ.velocity.y + (pQ1.velocity.y + 2.0 * pQ2.velocity.y + 2.0 * pQ3.velocity.y + pQ4.velocity.y) / numEqns;
    pBall->q.velocity.y = pQ.velocity.y;
    pQ.velocity.z = pQ.velocity.z + (pQ1.velocity.z + 2.0 * pQ2.velocity.z + 2.0 * pQ3.velocity.z + pQ4.velocity.z) / numEqns;
    pBall->q.velocity.z = pQ.velocity.z;
}

void GolfBall::SetDefaultBallValues(Environment* pEnviron)
{
    m_drawColorVector.clear();
    //m_timeStep = 0.1f;
    m_timeStep = 0.05f;
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
    m_ball.q.velocity.x = 0.0;   //  vx = 0.0
    m_ball.q.position.x = 0.0;   //  x  = 0.0
    m_ball.q.velocity.z = 0.0;   //  vz = 0.0
    m_ball.q.position.z = 0.0;   //  z  = 0.0
    m_ball.q.velocity.y = 0.0;   //  vy = 0.0
    m_ball.q.position.y = 0.0;   //  y  = 0.0
    m_ball.rotationAxis.x = 0.0;
    m_ball.rotationAxis.y = 0.0;
    m_ball.rotationAxis.z = 1.0; // ball will only be spinning about the z axis, this will need to be adjusted if/when imperfect impact mechanics added for hooks and slices
    m_ball.windSpeed.x = pEnviron->GetWindX();
    m_ball.windSpeed.y = pEnviron->GetWindY();
    m_ball.windSpeed.z = pEnviron->GetWindZ();
}

void GolfBall::SetLandingCordinates(DirectX::SimpleMath::Vector3 aCord)
{
    m_landingCordinates = aCord;
}

void GolfBall::SetSpinAxis(DirectX::SimpleMath::Vector4 aAxis)
{
    m_ball.rotationAxis.Normalize();
    float spinTotal = abs(aAxis.x) + abs(aAxis.y) + abs(aAxis.z);
    const float tolerance = 0.000001;
    if (spinTotal > tolerance)
    {
        m_ball.rotationAxis.x = aAxis.x / spinTotal;
        m_ball.rotationAxis.y = aAxis.y / spinTotal;
        m_ball.rotationAxis.z = aAxis.z / spinTotal;
    }
    else
    {
        m_ball.rotationAxis.x = 0.0;
        m_ball.rotationAxis.y = 0.0;
        m_ball.rotationAxis.z = 0.0;
    }
}

void GolfBall::UpdateSpinRate(double aTimeDelta)
{
    m_ball.omega *= 1.0 - (aTimeDelta * m_spinRateDecay);
}

//const DirectX::SimpleMath::Vector3 GolfBall::GetImpactAngle()
const float GolfBall::GetImpactAngle()
{
    DirectX::SimpleMath::Plane impactPlane = GetImpactPlane();
    
    if (m_shotPath.size() < 2)
    {
        std::cerr << "GolfBall::GetImpactAngle() error, m_xVals.size < 2, cannont process impact";
    }
    DirectX::SimpleMath::Vector3 impactPoint;
    impactPoint = m_landingCordinates;
    DirectX::SimpleMath::Vector3 impactMinus1;
    impactMinus1 = m_shotPath[m_shotPath.size() - 1];
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

    return g;
}

const float GolfBall::GetImpactDirection()
{
    float direction = atan(m_ball.q.velocity.z / m_ball.q.velocity.x);
    return direction;
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

const float GolfBall::GetImpactVelocity()
{
    DirectX::SimpleMath::Vector3 impactPoint;
    impactPoint.x = m_ball.q.velocity.x;
    impactPoint.y = m_ball.q.velocity.y;
    impactPoint.z = m_ball.q.velocity.z;

    float velocity = impactPoint.Length();

    return velocity;
}

const double GolfBall::GetShotDistance()
{
    DirectX::SimpleMath::Vector3 origin = m_shotOrigin;
    DirectX::SimpleMath::Vector3 landingPos = GetLandingCordinates();
    double distance = sqrt(((landingPos.x - origin.x) * (landingPos.x - origin.x)) + ((landingPos.y - origin.y)
        * (landingPos.y - origin.y) + ((landingPos.z - origin.z) * (landingPos.z - origin.z))));


    return distance;
}

const double GolfBall::GetLandingHeight()
{
    double height;
    int i = m_shotPath.size() - 1;
    height = m_shotPath[i].y;
    return height;
}