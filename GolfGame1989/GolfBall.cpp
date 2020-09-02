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

void GolfBall::FireProjectile(Utility::ImpactData aImpactData)
{   
    PrepProjectileLaunch(aImpactData);
    LaunchProjectile();
}

// WIP: Tweeking equations and measurement units and other voodoo hotness to get something that looks legit 
void GolfBall::LandProjectile()
{
    double direction = GetImpactDirection();
    double impactAngle = GetImpactAngle();

    impactAngle = Utility::ToRadians(impactAngle);
    double impactSpinRate = m_ball.omega; // conversion from rad per s to rpm
    
    impactSpinRate = impactSpinRate * -1;
    //impactSpinRate = impactSpinRate * 9.5493;
    //impactSpinRate = impactSpinRate / m_ball.radius;
    double vix = m_ball.q.velocity.x;
    double viy = m_ball.q.velocity.y;

    DirectX::SimpleMath::Vector3 vi(m_ball.q.velocity.x, m_ball.q.velocity.y, m_ball.q.velocity.z);
    
    double impactSpeed = vi.Length();

    vi.Normalize();

    //double phi = atan(abs(m_ball.q.velocity.x / m_ball.q.velocity.y));

    DirectX::SimpleMath::Vector3 phiVec(m_ball.q.velocity.x, m_ball.q.velocity.y, 0.0);
    DirectX::SimpleMath::Vector3 zeroDirection(1.0, 0.0, 0.0);

    double phi = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(phiVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(phiVec, zeroDirection)) < 0.0f)
    {
        phi = -phi;
    }

    //phi = Utility::ToDegrees(phi);

    //?c = 15.4?(vi / (impact speed))(? / (impact angle))
    //double newThetaC = 15.4f * (impactSpeed) * (phi);
    //double newThetaC = Utility::ToRadians(15.4) * (impactSpeed) * (phi);
    //double test = (vi / impactSpeed2) * (phi / impactAngle);
    double thetaC = Utility::ToRadians(15.4);
    //thetaC = 15.4;
    //thetaC = newThetaC;
    //double thetaC = 15.4;
    //thetaC = 15.4f * (impactSpeed) * (phi);
    //thetaC = Utility::ToRadians(thetaC);
    //thetaC = thetaC * (impactSpeed) * (phi);

    //thetaC = Utility::ToRadians(15.4) * impactSpeed * Utility::ToRadians(impactAngle);

    double vixPrime = vix * cos(thetaC) - abs(viy) * sin(thetaC);
    double absViyPrime = vix * sin(thetaC) + abs(viy) * cos(thetaC);

    absViyPrime = abs(absViyPrime);
    double e;

   // if (absViyPrime <= 20.0)
    if(impactSpeed <= 20.0)
    {
        e = 0.510 - 0.0375 * absViyPrime + 0.000903 * absViyPrime * absViyPrime;
    }
    else
    {
        e = 0.120;
    }

    double muMin = (2.0 * (vix + m_ball.radius * impactSpinRate)) / (7.0 * (1.0 + e) * viy);
    double mu = 0.43; // (greek u symbol) from Danish equation from green impact, WLJ will need to tweek as its based off type of terrain
    //double muC = (2 * (vixPrime + (m_ball.radius * m_ball.omega))) / (7 * (1.0 + e) * absViyPrime);
    double muC = (2 * (vixPrime + (m_ball.radius * impactSpinRate))) / (7 * (1.0 + e) * absViyPrime);

    double vrxPrime, vryPrime, omegaR;

    // If mu is less than the critical value muC the ball will slide throughout the impact
    // If mu is great than muC the ball will roll out of the collision
    if (mu < muC)
    {
        vrxPrime = vixPrime - mu * absViyPrime * (1.0 + e);
        vryPrime = e * absViyPrime;
        //omegaR = m_ball.omega - ((5.0 * mu) / (2.0 * m_ball.radius)) * absViyPrime * (1.0 + e);
        omegaR = impactSpinRate - ((5.0 * mu) / (2.0 * m_ball.radius)) * absViyPrime * (1.0 + e);
    }
    else
    {
        //vrxPrime = (5.0 / 7.0) * vixPrime - (2.0 / 7.0) * m_ball.radius * m_ball.omega;
        vrxPrime = (5.0 / 7.0) * vixPrime - (2.0 / 7.0) * m_ball.radius * impactSpinRate;
        vryPrime = e * absViyPrime;
        omegaR = -vrxPrime / m_ball.radius;
    }

    double vrx = vrxPrime * cos(thetaC) - vryPrime * sin(thetaC);
    double vry = vrxPrime * sin(thetaC) + vryPrime * cos(thetaC); 
    // WLJ ToDo: sort out unit conversion errors with RPM vs Rad per S or what ever units are getting used due to multiple sources for equations and attempts to counter problem
    //double vry = vrxPrime * cos(thetaC) + vryPrime * cos(thetaC);

    // WLJ dirty calculation for z velocity update until I convert 2d equations into 3d;
    DirectX::SimpleMath::Vector3 directionOfTravel = m_ball.q.velocity;
    directionOfTravel.y = 0.0f;

    //double ratioX = vrx / m_ball.q.velocity.x;
    //double ratioY = vry / m_ball.q.velocity.y;
    //double ratioZ = ratioX + ratioY;
    //double preZ = m_ball.q.velocity.z;
    //double vrz = m_ball.q.velocity.z * ratioZ;

    double backwardsBounceCheck = (impactSpeed / (2. * m_ball.radius)) * (5. * sin(impactAngle - thetaC) - 7. * e * tan(thetaC) * cos(impactAngle - thetaC));
    backwardsBounceCheck = backwardsBounceCheck * -1;
    
    if (impactSpinRate > backwardsBounceCheck)
    {
        //direction -= Utility::ToRadians(180.);
    }
    
    if (muMin > muC)
    {
        //direction += Utility::ToRadians(180.);
    }
    
    m_ball.q.velocity.x = static_cast<float>(vrx);
    m_ball.q.velocity.y = static_cast<float>(vry);
    //m_ball.q.velocity.z = vrz;
    m_ball.q.velocity.z = 0.0; // doing it dirty until calculations can be sorted in 3d

    m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(-direction)));
    //m_ball.omega = omegaR;
    //m_ball.omega = omegaR * m_ball.radius;
    omegaR = omegaR * -1;
    m_ball.omega = omegaR;
    //m_ball.omega = omegaR;
    //m_ball.omega = omegaR * .10472; // conversion from rpm to rad per second
}

void GolfBall::RollBall()
{
    double pg = 0.131; //0.131 is from A. Raymond Penner "The Run of a Golf Ball" doc
    double g = m_ball.gravity;
    double a = -(5.0 / 7.0) * pg * g; // a = 0.916999996	float

    double decelFactor = a;
    double stopTolerance = 0.1;
    int overflowTolerance = 650;

    DirectX::SimpleMath::Vector3 directionVec = m_ball.q.velocity;
    directionVec.y = 0.0;
    directionVec.Normalize();

    int i = 0;
    while (m_ball.q.velocity.x > stopTolerance && i < overflowTolerance)
    {
        double velocity = m_ball.q.velocity.Length();

        velocity -= decelFactor * velocity * m_timeStep;
        m_ball.q.velocity = directionVec * static_cast<float>(velocity);
        m_ball.q.position += m_ball.q.velocity * m_timeStep;
        m_ball.flightTime = m_ball.flightTime + m_timeStep;
        PushFlightData();
        ++i;
    }
}

void GolfBall::LaunchProjectile()
{
    PushFlightData();

    // Fly ball on an upward trajectory until it stops climbing
    BallMotion flightData;
    double dt = m_timeStep;
    double maxHeight = m_ball.launchHeight;
    double bounceHeight = m_ball.launchHeight;
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
                if (m_ball.q.position.y > maxHeight)
                {
                    maxHeight = m_ball.q.position.y;
                }
                if (m_ball.q.position.y > bounceHeight)
                {
                    bounceHeight = m_ball.q.position.y;
                }
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
            PushFlightData();
            time = m_ball.flightTime;
        }

        if (m_shotPath.size() > 1)
        {
            double rollBackTime = CalculateImpactTime(previousTime, time, previousY, m_ball.q.position.y);
            ProjectileRungeKutta4(&m_ball, -rollBackTime);
            m_shotPath[m_shotPath.size() - 1] = m_ball.q.position;
        }

        if (count == 0)
        {
            m_landingImpactCordinates = m_ball.q.position;
        }

        SetLandingSpinRate(m_ball.omega);
        
        ++m_bounceCount;
        
        double angle1 = GetImpactAngle();

        LandProjectile();

        double angle2 = GetImpactAngle();
        double angle2post = angle2 - 90.0;

        ++count;
        if (m_ball.q.velocity.y < 3.9 || count > 19 || bounceHeight < .3) // WLJ bounce height threshold is just a guess at this point
        {
            isBallFlyOrBounce = false;
        } 
        bounceHeight = m_ball.landingHeight;
    }

    SetMaxHeight(maxHeight);

    this->m_ball.q.velocity.y = 0.0;
    m_ball.q.position.y = 0.0;
    RollBall();
    SetLandingCordinates(m_ball.q.position);
}

void GolfBall::PrepProjectileLaunch(Utility::ImpactData aImpactData)
{ 
    //  Convert the loft angle from degrees to radians and
    //  assign values to some convenience variables.
    double loft = Utility::ToRadians(aImpactData.angleY);
    //double cosL = cos(loft);
    double sinL = sin(loft);

    //  Calculate the pre-collision velocities normal
    //  and parallel to the line of action.
    double velocity = aImpactData.velocity;
    //double vcp = cosL * velocity;
    double vcn = -sinL * velocity;

    //  Compute the post-collision velocity of the ball
    //  along the line of action.
    double ballMass = m_ball.mass;
    double clubMass = aImpactData.mass;
    double e = aImpactData.cor; //  coefficient of restitution of club face striking the ball
    //double vbp = (1.0 + e) * clubMass * vcp / (clubMass + ballMass);

    //  Compute the post-collision velocity of the ball
    //  perpendicular to the line of action.
    //double vbn = (1.0 - m_faceRoll) * clubMass * vcn / (clubMass + ballMass);

    //  Compute the initial spin rate assuming ball is
    //  rolling without sliding.
    double radius = m_ball.radius;
    double omega = m_faceRoll * vcn / radius;

    //  Rotate post-collision ball velocities back into 
    //  standard Cartesian frame of reference. Because the
    //  line-of-action was in the xy plane, the z-velocity
    //  is zero.
    //double vx0 = cosL * vbp - sinL * vbn;
    //double vy0 = sinL * vbp + cosL * vbn;
    //double vz0 = 0.0;

    //DirectX::SimpleMath::Vector4 vBall = (static_cast<float>((((1.0 + e) * clubMass) / (clubMass + ballMass))) * aImpactData.vHeadNormal)
    //    + (static_cast<float>(((2 * clubMass) / (7 * (clubMass + ballMass)))) * aImpactData.vHeadParallel);
    DirectX::SimpleMath::Vector3 vBall = (static_cast<float>((((1.0 + e) * clubMass) / (clubMass + ballMass))) * aImpactData.vHeadNormal)
        + (static_cast<float>(((2 * clubMass) / (7 * (clubMass + ballMass)))) * aImpactData.vHeadParallel);

    DirectX::SimpleMath::Vector3 unitVHead = aImpactData.vHead;
    unitVHead.Normalize();
    DirectX::SimpleMath::Vector3 unitFaceNormal = aImpactData.vFaceNormal;
    unitFaceNormal.Normalize();

    //DirectX::SimpleMath::Vector4 crossVheadvFace = unitVHead.Cross(unitFaceNormal);
    DirectX::SimpleMath::Vector3 crossVheadvFace = unitVHead.Cross(unitFaceNormal);

    double absVhP = sqrt((aImpactData.vHeadParallel.x * aImpactData.vHeadParallel.x)
        + (aImpactData.vHeadParallel.y * aImpactData.vHeadParallel.y) 
        + (aImpactData.vHeadParallel.z * aImpactData.vHeadParallel.z));

    //DirectX::SimpleMath::Vector4 omegaBall = static_cast<float>(((5.0 * absVhP) / (7.0 * m_ball.radius))) * crossVheadvFace;
    DirectX::SimpleMath::Vector3 omegaBall = static_cast<float>(((5.0 * absVhP) / (7.0 * m_ball.radius))) * crossVheadvFace;

    double absOmegaBall = sqrt((omegaBall.x * omegaBall.x)
        + (omegaBall.y * omegaBall.y)
        + (omegaBall.z * omegaBall.z));
    double absvBall = sqrt((vBall.x * vBall.x)
        + (vBall.y * vBall.y)
        + (vBall.z * vBall.z));

    double cL = -0.05 + sqrt(0.0025 + 0.36 * ((m_ball.radius * absOmegaBall) / absvBall));
    DirectX::SimpleMath::Vector3 fMangus = (static_cast<float>(.5f * m_ball.airDensity * m_ball.area * cL * absvBall * absvBall)) 
        * (unitFaceNormal.Cross(unitVHead));

    DirectX::SimpleMath::Vector3 normfManus = fMangus;
    DirectX::SimpleMath::Vector3 normOmegaBall = omegaBall;

    normfManus.Normalize();
    normOmegaBall.Normalize();

    m_ball.omega = omega;
   
    m_ball.q.velocity.x = vBall.x;
    m_ball.q.velocity.y = vBall.y;
    m_ball.q.velocity.z = vBall.z;

    float shotDirection = aImpactData.directionDegrees;

    // Turn velocity and axis of rotation to aimed direction
    m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aImpactData.directionDegrees)));
    m_ball.rotationAxis = DirectX::SimpleMath::Vector3::Transform(m_ball.rotationAxis, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aImpactData.directionDegrees)));
}

void GolfBall::PushFlightData()
{
    // Prevent push of data below ground level
    if (m_ball.q.position.y >= m_ball.landingHeight)
    {
        m_shotPath.push_back(m_ball.q.position);
        float timeStep = static_cast<float>(m_ball.flightTime);
        m_shotPathTimeStep.push_back(timeStep);
        //m_shotPathTimeStep.push_back(m_ball.flightTime); //Error C4244
    }
}

//  This method loads the right-hand sides for the projectile ODEs
void GolfBall::ProjectileRightHandSide(struct SpinProjectile* pBall, BallMotion* q, BallMotion* deltaQ, double aTimeDelta, double aQScale, BallMotion* dq)
{
    //  Compute the intermediate values of the 
    //  dependent variables.
    BallMotion newQ;
    newQ.position.x = q->position.x + static_cast<float>(aQScale) * deltaQ->position.x;
    newQ.position.y = q->position.y + static_cast<float>(aQScale) * deltaQ->position.y;
    newQ.position.z = q->position.z + static_cast<float>(aQScale) * deltaQ->position.z;
    newQ.velocity.x = q->velocity.x + static_cast<float>(aQScale) * deltaQ->velocity.x;
    newQ.velocity.y = q->velocity.y + static_cast<float>(aQScale) * deltaQ->velocity.y;
    newQ.velocity.z = q->velocity.z + static_cast<float>(aQScale) * deltaQ->velocity.z;

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

    // ToDo: WLJ look into coverting drag forces to vector, this does not apply wind to flight
    DirectX::SimpleMath::Vector3 absVball = DirectX::XMVectorAbs(newQ.velocity);
    DirectX::SimpleMath::Vector3 vBallNorm = newQ.velocity;
    vBallNorm.Normalize();   
    DirectX::SimpleMath::Vector3 Fd3D = -(0.5 * pBall->airDensity * pBall->area * m_ball.dragCoefficient * (absVball * absVball)) * vBallNorm;
    double Fdx1 = Fd3D.x;
    double Fdy1 = Fd3D.y;
    double Fdz1 = Fd3D.z;

    double Fdx2 = Fd3D.x * vax / va;
    double Fdy2 = Fd3D.y * vay / va;
    double Fdz2 = Fd3D.z * vaz / va;

    //Fdx = Fdx2;
    //Fdy = Fdy2;
    //Fdz = Fdz2;

    //  Compute the velocity magnitude
    double v = sqrt(vx * vx + vy * vy + vz * vz) + 1.0e-8;

    //  Evaluate the Magnus force terms.
    double Cl = -0.05 + sqrt(0.0025 + 0.36 * fabs(pBall->radius * pBall->omega / v));  // this equation gives a more accurate representation to fit experimental data than Cl = (radius * omega)/v
    double Fm = 0.5 * pBall->airDensity * pBall->area * Cl * v * v;
    double Fmx = (vz * pBall->rotationAxis.y - pBall->rotationAxis.z * vy) * Fm / v;
    double Fmy = (vx * pBall->rotationAxis.z - pBall->rotationAxis.x * vz) * Fm / v;
    double Fmz = -(vx * pBall->rotationAxis.y - pBall->rotationAxis.x * vy) * Fm / v;

    //  Compute right-hand side values.
    dq->velocity.x = static_cast<float>(aTimeDelta * (Fdx + Fmx) / pBall->mass);
    dq->position.x = static_cast<float>(aTimeDelta * vx);
    dq->velocity.y = static_cast<float>(aTimeDelta * (pBall->gravity + (Fdy + Fmy) / pBall->mass));
    dq->position.y = static_cast<float>(aTimeDelta * vy);
    dq->velocity.z = static_cast<float>(aTimeDelta * (Fdz + Fmz) / pBall->mass);
    dq->position.z = static_cast<float>(aTimeDelta * vz);
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

    pQ.position.x = static_cast<float>(pQ.position.x + (pQ1.position.x + 2.0 * pQ2.position.x + 2.0 * pQ3.position.x + pQ4.position.x) / numEqns);
    pBall->q.position.x = pQ.position.x;
    pQ.position.y = static_cast<float>(pQ.position.y + (pQ1.position.y + 2.0 * pQ2.position.y + 2.0 * pQ3.position.y + pQ4.position.y) / numEqns);
    pBall->q.position.y = pQ.position.y;
    pQ.position.z = static_cast<float>(pQ.position.z + (pQ1.position.z + 2.0 * pQ2.position.z + 2.0 * pQ3.position.z + pQ4.position.z) / numEqns);
    pBall->q.position.z = pQ.position.z;
    pQ.velocity.x = static_cast<float>(pQ.velocity.x + (pQ1.velocity.x + 2.0 * pQ2.velocity.x + 2.0 * pQ3.velocity.x + pQ4.velocity.x) / numEqns);
    pBall->q.velocity.x = pQ.velocity.x;
    pQ.velocity.y = static_cast<float>(pQ.velocity.y + (pQ1.velocity.y + 2.0 * pQ2.velocity.y + 2.0 * pQ3.velocity.y + pQ4.velocity.y) / numEqns);
    pBall->q.velocity.y = pQ.velocity.y;
    pQ.velocity.z = static_cast<float>(pQ.velocity.z + (pQ1.velocity.z + 2.0 * pQ2.velocity.z + 2.0 * pQ3.velocity.z + pQ4.velocity.z) / numEqns);
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
    pQ->position.x = static_cast<float>(pQ->position.x + (pQ1->position.x + 2.0 * pQ2->position.x + 2.0 * pQ3->position.x + pQ4->position.x) / numEqns);
    pBall->q.position.x = pQ->position.x;
    pQ->position.y = static_cast<float>(pQ->position.y + (pQ1->position.y + 2.0 * pQ2->position.y + 2.0 * pQ3->position.y + pQ4->position.y) / numEqns);
    pBall->q.position.y = pQ->position.y;
    pQ->position.z = static_cast<float>(pQ->position.z + (pQ1->position.z + 2.0 * pQ2->position.z + 2.0 * pQ3->position.z + pQ4->position.z) / numEqns);
    pBall->q.position.z = pQ->position.z;
    pQ->velocity.x = static_cast<float>(pQ->velocity.x + (pQ1->velocity.x + 2.0 * pQ2->velocity.x + 2.0 * pQ3->velocity.x + pQ4->velocity.x) / numEqns);
    pBall->q.velocity.x = pQ->velocity.x;
    pQ->velocity.y = static_cast<float>(pQ->velocity.y + (pQ1->velocity.y + 2.0 * pQ2->velocity.y + 2.0 * pQ3->velocity.y + pQ4->velocity.y) / numEqns);
    pBall->q.velocity.y = pQ->velocity.y;
    pQ->velocity.z = static_cast<float>(pQ->velocity.z + (pQ1->velocity.z + 2.0 * pQ2->velocity.z + 2.0 * pQ3->velocity.z + pQ4->velocity.z) / numEqns);
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
}

void GolfBall::ResetBallData()
{
    m_shotPath.clear();
    m_shotPathTimeStep.clear();
    m_landingCordinates = m_shotOrigin;
    m_bounceCount = 0;
    m_shotPath.clear();
    m_shotPathTimeStep.clear();
    m_ball.flightTime = 0.0;
    m_ball.omega = 0.0;
    m_ball.q.position.x = 0.0;
    m_ball.q.position.y = 0.0;
    m_ball.q.position.z = 0.0;
    m_ball.q.velocity.x = 0.0;
    m_ball.q.velocity.y = 0.0;
    m_ball.q.velocity.z = 0.0;
}

void GolfBall::SetDefaultBallValues(Environment* pEnviron)
{
    m_ball.airDensity = pEnviron->GetAirDensity();
    m_ball.area = 0.001432;
    m_ball.dragCoefficient = 0.22;
    m_ball.flightTime = 0.0;      
    m_ball.gravity = pEnviron->GetGravity();
    m_ball.launchHeight = pEnviron->GetLauchHeight();
    m_ball.landingHeight = pEnviron->GetLandingHeight();
    m_ball.radius = 0.02135;
    m_ball.mass = 0.0459;
    m_ball.numEqns = 6;
    m_ball.omega = 0.0;
    m_ball.q.position = pEnviron->GetTeePosition();
    m_ball.q.velocity.x = 0.0;
    m_ball.q.velocity.y = 0.0;
    m_ball.q.velocity.z = 0.0;
    m_ball.rotationAxis.x = 0.0;
    m_ball.rotationAxis.y = 0.0;
    m_ball.rotationAxis.z = 1.0; // ball will only be spinning about the z axis, this will need to be adjusted if/when imperfect impact mechanics added for hooks and slices
    m_ball.windSpeed.x = static_cast<float>(pEnviron->GetWindX());
    m_ball.windSpeed.y = static_cast<float>(pEnviron->GetWindY());
    m_ball.windSpeed.z = static_cast<float>(pEnviron->GetWindZ());
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

double GolfBall::GetImpactAngle() const
{
    DirectX::SimpleMath::Plane impactPlane = GetImpactPlane();
    DirectX::SimpleMath::Vector3 impactVector(m_ball.q.velocity.x, m_ball.q.velocity.y, m_ball.q.velocity.z);

    impactVector.Normalize();
    DirectX::SimpleMath::Vector3 planeNormal = impactPlane.Normal();
    double angle = acos(impactVector.Dot(planeNormal));

    angle = Utility::ToDegrees(angle);
    angle = 180.0 - angle;

    return angle;
}

double GolfBall::GetImpactDirection() const
{
    DirectX::SimpleMath::Vector3 ballVec(m_ball.q.velocity.x, 0.0, m_ball.q.velocity.z);
    DirectX::SimpleMath::Vector3 zeroDirection(1.0, 0.0, 0.0);

    double direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(ballVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(ballVec, zeroDirection)) < 0.0f)
    {
        direction = -direction;
    }

    return direction;
}

DirectX::SimpleMath::Plane GolfBall::GetImpactPlane() const
{
    // create a default horizontal plane at 0,0,0. fill in later for non-horizontal plane gameplay
    DirectX::SimpleMath::Vector3 a = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 b = DirectX::SimpleMath::Vector3::Zero;;
    a.y = m_landingCordinates.y;
    b.y = a.y + 1.0f;
    DirectX::SimpleMath::Plane impactPlane = DirectX::SimpleMath::Plane(a, b);
    return impactPlane;
}

float GolfBall::GetImpactVelocity() const
{
    DirectX::SimpleMath::Vector3 impactPoint;
    impactPoint.x = m_ball.q.velocity.x;
    impactPoint.y = m_ball.q.velocity.y;
    impactPoint.z = m_ball.q.velocity.z;

    float velocity = impactPoint.Length();

    return velocity;
}

double GolfBall::GetShotDistance() const
{
    DirectX::SimpleMath::Vector3 origin = m_shotOrigin;
    DirectX::SimpleMath::Vector3 landingPos = GetLandingCordinates();
    double distance = sqrt(((landingPos.x - origin.x) * (landingPos.x - origin.x)) + ((landingPos.y - origin.y)
        * (landingPos.y - origin.y) + ((landingPos.z - origin.z) * (landingPos.z - origin.z))));

    return distance;
}

double GolfBall::GetShotFlightDistance() const
{
    DirectX::SimpleMath::Vector3 origin = m_shotOrigin;
    DirectX::SimpleMath::Vector3 landingPos = GetLandingCordinates2();
    double distance = sqrt(((landingPos.x - origin.x) * (landingPos.x - origin.x)) + ((landingPos.y - origin.y)
        * (landingPos.y - origin.y) + ((landingPos.z - origin.z) * (landingPos.z - origin.z))));

    return distance;
}

double GolfBall::GetLandingHeight() const
{
    if (m_shotPath.size() < 2)
    {
        return m_shotOrigin.y;
    }
    double height;
    int i = (int)m_shotPath.size() - 1;
    height = m_shotPath[i].y;
    return height;
}

void GolfBall::ZeroDataForUI()
{
    m_initialSpinRate = 0.0;
    m_landingSpinRate = 0.0;
    m_shotOrigin.Zero;
    m_landingCordinates.Zero;
    m_maxHeight = 0.0;
}