#include "pch.h"
#include <iostream>
#include <malloc.h>
#include <math.h>
#include <memory>
#include "GolfBall.h"
#include <vector>


void GolfBall::AddDebugDrawLines(DirectX::SimpleMath::Vector3 aOriginPos, DirectX::SimpleMath::Vector3 aLine, DirectX::XMVECTORF32 aColor)
{
    DirectX::VertexPositionColor originVertex(aOriginPos, aColor);
    DirectX::VertexPositionColor lineEndVertex(aLine + aOriginPos, aColor);
    std::pair<DirectX::VertexPositionColor, DirectX::VertexPositionColor> vertexPair(originVertex, lineEndVertex);
    m_debugDrawLines.push_back(vertexPair);
}

double GolfBall::CalculateImpactTime(double aTime1, double aTime2, float aHeight1, float aHeight2)
{
    float m = (aHeight2 - aHeight1) / (static_cast<float>(aTime2 - aTime1));
    float b = aHeight1 - (m * static_cast<float>(aTime1));
    float impactTime = -b / m;
    float dt = static_cast<float>(aTime2) - impactTime;

    return dt;
}

bool GolfBall::DoesBallRollInHole(const DirectX::SimpleMath::Vector3 aEnterRadiusPos, const double aEnterRadiusTime, const DirectX::SimpleMath::Vector3 aExitRadiusPos, const double aExitRadiusTime)
{
    // s = (1/2) a t^2
    // s = Vit (1/2) a t^2 // with initial velocity

    double traversalTime = aExitRadiusTime - aEnterRadiusTime;

    float verticalDrop = .5f * m_ball.gravity * static_cast<float>((traversalTime * traversalTime));
    bool isInHole = false;
    if (abs(verticalDrop) >= m_ball.radius)
    {
        isInHole = true;
    }
    else
    {
        isInHole = false;
    }

    if (isInHole == false)
    {
        DirectX::SimpleMath::Vector3 updatedVelocity = GetPostCollisionVelocity(aEnterRadiusPos, aExitRadiusPos, pBallEnvironment->GetHolePosition(), verticalDrop);
        m_ball.q.velocity = updatedVelocity;
    }
    else
    {
        // ToDo: add positioning ball in hole location here
    }

    return isInHole;
}

void GolfBall::FireProjectile(Utility::ImpactData aImpactData)
{   
    PrepProjectileLaunch(aImpactData);
    LaunchProjectile();
}

DirectX::SimpleMath::Vector3 GolfBall::GetBallPosInEnviron(DirectX::SimpleMath::Vector3 aPos) const
{
    float scaleFactor = pBallEnvironment->GetScale();
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(scaleFactor, scaleFactor, scaleFactor);
    DirectX::SimpleMath::Vector3 scaledPos = DirectX::SimpleMath::Vector3::Transform(aPos, scaleMatrix);
    scaledPos += m_shotOrigin;

    return scaledPos;
}

float GolfBall::GetBallFlightAltitude(DirectX::SimpleMath::Vector3 aPos)
{  
    float scale = pBallEnvironment->GetScale();   
    aPos *= scale;
    aPos += m_shotOrigin;

    float terrainHeight = pBallEnvironment->GetTerrainHeightAtPos(aPos);
    float scaledBallHeight = (m_ball.q.position.y * scale) + m_shotOrigin.y ;
    float aboveGroundLevelHeight = scaledBallHeight - terrainHeight;

    return aboveGroundLevelHeight;
}

float GolfBall::GetDistanceToHole() const
{
    float environScale = pBallEnvironment->GetScale();
    DirectX::SimpleMath::Vector3 startPos = m_shotOrigin;
    DirectX::SimpleMath::Vector3 scaledBallPos = m_ball.q.position;
    DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(environScale, environScale, environScale);
    scaledBallPos = DirectX::SimpleMath::Vector3::Transform(scaledBallPos, scaleMatrix);
    scaledBallPos += startPos;

    return (pBallEnvironment->GetHolePosition() - scaledBallPos).Length();
}

float GolfBall::GetImpactAngleDeg() const
{
    DirectX::SimpleMath::Plane impactPlane = GetImpactPlane();
    DirectX::SimpleMath::Vector3 impactVector(m_ball.q.velocity.x, m_ball.q.velocity.y, m_ball.q.velocity.z);

    impactVector.Normalize();
    DirectX::SimpleMath::Vector3 planeNormal = impactPlane.Normal();
    float angle = acos(impactVector.Dot(planeNormal));

    angle = Utility::ToDegrees(angle);
    angle = 180.0f - angle;

    return angle;
}

float GolfBall::GetImpactDirection() const
{
    DirectX::SimpleMath::Vector3 ballVec(m_ball.q.velocity.x, 0.0, m_ball.q.velocity.z);
    DirectX::SimpleMath::Vector3 zeroDirection(1.0, 0.0, 0.0);

    float direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(ballVec), DirectX::XMVector3Normalize(zeroDirection)));

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

float GolfBall::GetLandingHeight() const
{
    if (m_ballPath.size() < 2)
    {
        return m_shotOrigin.y;
    }
    float height;
    int i = (int)m_ballPath.size() - 1;
    height = m_ballPath[i].position.y;
    return height;
}

float GolfBall::GetShotDistance() const
{
    DirectX::SimpleMath::Vector3 origin = GetBallPosInEnviron(m_shotOrigin);
    DirectX::SimpleMath::Vector3 landingPos = GetBallPosInEnviron(GetLandingCordinates());
    float distance = sqrt(((landingPos.x - origin.x) * (landingPos.x - origin.x)) + ((landingPos.y - origin.y)
        * (landingPos.y - origin.y) + ((landingPos.z - origin.z) * (landingPos.z - origin.z))));

    return distance;
}

float GolfBall::GetShotFlightDistance() const
{
    DirectX::SimpleMath::Vector3 origin = m_shotOrigin;
    DirectX::SimpleMath::Vector3 landingPos = GetLandingCordinates2();
    float distance = sqrt(((landingPos.x - origin.x) * (landingPos.x - origin.x)) + ((landingPos.y - origin.y)
        * (landingPos.y - origin.y) + ((landingPos.z - origin.z) * (landingPos.z - origin.z))));

    return distance;
}

// Prototype hole rim collisions to redirect ball path if it interacts but doesn't go in the hole
DirectX::SimpleMath::Vector3 GolfBall::GetPostCollisionVelocity(const DirectX::SimpleMath::Vector3 aVec1, const DirectX::SimpleMath::Vector3 aVec2, const DirectX::SimpleMath::Vector3 aVec3, const float aHeightDrop)
{
    DirectX::SimpleMath::Vector3 preTravelNorm = m_ball.q.velocity;
    preTravelNorm.Normalize();
    DirectX::SimpleMath::Vector3 holeNormal = pBallEnvironment->GetHolePosition() - GetBallPosInEnviron(m_ball.q.position);
    holeNormal.Normalize();

    DirectX::SimpleMath::Vector3 ballDropPos = m_ball.q.position;
    ballDropPos.y += aHeightDrop / pBallEnvironment->GetScale();
    ballDropPos = GetBallPosInEnviron(ballDropPos);

    DirectX::SimpleMath::Vector3 dropVec = pBallEnvironment->GetHolePosition() - ballDropPos;
 
    dropVec.y *= -1;

    DirectX::SimpleMath::Plane impactPlane = GetImpactPlane();
    DirectX::SimpleMath::Vector3 impactNormal = impactPlane.Normal();
    
    DirectX::SimpleMath::Vector3 holePerpendicular = holeNormal;
    holePerpendicular.Cross(impactNormal, holePerpendicular);

    DirectX::SimpleMath::Vector3 collisionAxis = holePerpendicular;
    collisionAxis.Cross(dropVec, collisionAxis);

    DirectX::SimpleMath::Vector3 updatedVelocityNorm = preTravelNorm;

    AddDebugDrawLines(GetBallPosInEnviron(m_ball.q.position), collisionAxis, DirectX::Colors::Red);

    updatedVelocityNorm.Transform(updatedVelocityNorm, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(collisionAxis, -Utility::GetPi()), updatedVelocityNorm);
    updatedVelocityNorm *= -1;
    updatedVelocityNorm.y = 0.0;
    AddDebugDrawLines(GetBallPosInEnviron(m_ball.q.position), updatedVelocityNorm, DirectX::Colors::Yellow);

    updatedVelocityNorm *= m_ball.q.velocity.Length();

    return updatedVelocityNorm;
}

// WIP : Tweeking equations and measurement units and other voodoo hotness to get something that looks legit 
//     : Work on pause until 3D terrain work is finished
void GolfBall::LandProjectile()
{
    float direction = GetImpactDirection();
    float impactAngle = GetImpactAngleDeg();

    impactAngle = Utility::ToRadians(impactAngle);
    float impactSpinRate = m_ball.omega; // conversion from rad per s to rpm
    
    impactSpinRate = impactSpinRate * -1;
    //impactSpinRate = impactSpinRate * 9.5493;
    //impactSpinRate = impactSpinRate / m_ball.radius;

    DirectX::SimpleMath::Vector3 preVelocity = m_ball.q.velocity;
    // WTF?
    DirectX::SimpleMath::Vector3 velocity = m_ball.q.velocity;
    m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(direction)));
    velocity = DirectX::SimpleMath::Vector3::Transform(velocity, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(direction)));

    float vix = velocity.x;
    float viy = velocity.y;

    DirectX::SimpleMath::Vector3 vi(velocity.x, velocity.y, velocity.z);
    
    float impactSpeed = vi.Length();

    vi.Normalize();

    //float phi = atan(abs(m_ball.q.velocity.x / m_ball.q.velocity.y));

    DirectX::SimpleMath::Vector3 phiVec(velocity.x, velocity.y, 0.0);
    DirectX::SimpleMath::Vector3 zeroDirection(1.0, 0.0, 0.0);

    float phi = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(phiVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(phiVec, zeroDirection)) < 0.0f)
    {
        phi = -phi;
    }

    //phi = Utility::ToDegrees(phi);

    //?c = 15.4?(vi / (impact speed))(? / (impact angle))
    //float newThetaC = 15.4f * (impactSpeed) * (phi);
    //float newThetaC = Utility::ToRadians(15.4) * (impactSpeed) * (phi);
    //float test = (vi / impactSpeed2) * (phi / impactAngle);
    float thetaC = Utility::ToRadians(15.4);
    //thetaC = 15.4;
    //thetaC = newThetaC;
    //float thetaC = 15.4;
    //thetaC = 15.4f * (impactSpeed) * (phi);
    //thetaC = Utility::ToRadians(thetaC);
    //thetaC = thetaC * (impactSpeed) * (phi);

    //thetaC = Utility::ToRadians(15.4) * impactSpeed * Utility::ToRadians(impactAngle);

    float vixPrime = vix * cos(thetaC) - abs(viy) * sin(thetaC);
    float absViyPrime = vix * sin(thetaC) + abs(viy) * cos(thetaC);

    absViyPrime = abs(absViyPrime);
    float e;

   // if (absViyPrime <= 20.0)
    if (impactSpeed <= 20.0)
    {
        e = 0.510f - 0.0375f * absViyPrime + 0.000903f * absViyPrime * absViyPrime;
    }
    else
    {
        e = 0.120f;
    }

    float muMin = (2.0f * (vix + m_ball.radius * impactSpinRate)) / (7.0f * (1.0f + e) * viy);
    float mu = 0.43; // (greek u symbol) from Danish equation from green impact, WLJ will need to tweek as its based off type of terrain
    //float muC = (2 * (vixPrime + (m_ball.radius * m_ball.omega))) / (7 * (1.0 + e) * absViyPrime);
    float muC = (2 * (vixPrime + (m_ball.radius * impactSpinRate))) / (7 * (1.0f + e) * absViyPrime);

    float vrxPrime, vryPrime, omegaR;

    // If mu is less than the critical value muC the ball will slide throughout the impact
    // If mu is great than muC the ball will roll out of the collision
    if (mu < muC)
    {
        vrxPrime = vixPrime - mu * absViyPrime * (1.0f + e);
        vryPrime = e * absViyPrime;
        //omegaR = m_ball.omega - ((5.0 * mu) / (2.0 * m_ball.radius)) * absViyPrime * (1.0 + e);
        omegaR = impactSpinRate - ((5.0f * mu) / (2.0f * m_ball.radius)) * absViyPrime * (1.0f + e);
    }
    else
    {
        //vrxPrime = (5.0 / 7.0) * vixPrime - (2.0 / 7.0) * m_ball.radius * m_ball.omega;
        vrxPrime = (5.0f / 7.0f) * vixPrime - (2.0f / 7.0f) * m_ball.radius * impactSpinRate;
        vryPrime = e * absViyPrime;
        omegaR = -vrxPrime / m_ball.radius;
    }

    float vrx = vrxPrime * cos(thetaC) - vryPrime * sin(thetaC);
    float vry = vrxPrime * sin(thetaC) + vryPrime * cos(thetaC); 
    // WLJ ToDo: sort out unit conversion errors with RPM vs Rad per S or what ever units are getting used due to multiple sources for equations and attempts to counter problem
    //float vry = vrxPrime * cos(thetaC) + vryPrime * cos(thetaC);

    // WLJ dirty calculation for z velocity update until I convert 2d equations into 3d;
    DirectX::SimpleMath::Vector3 directionOfTravel = velocity;
    directionOfTravel.y = 0.0f;

    //float ratioX = vrx / m_ball.q.velocity.x;
    //float ratioY = vry / m_ball.q.velocity.y;
    //float ratioZ = ratioX + ratioY;
    //float preZ = m_ball.q.velocity.z;
    //float vrz = m_ball.q.velocity.z * ratioZ;

    float backwardsBounceCheck = (impactSpeed / (2.f * m_ball.radius)) * (5.f * sin(impactAngle - thetaC) - 7.f * e * tan(thetaC) * cos(impactAngle - thetaC));
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

    //m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(-direction)));

    //  ::: Vnew = b * (-2 * (V dot N) * N + V)
    const float b = .3f;
    DirectX::SimpleMath::Vector3 terrainNorm = pBallEnvironment->GetTerrainNormal(GetBallPosInEnviron(m_ball.q.position));
    m_ball.q.velocity = b * (-2 * (preVelocity.Dot(terrainNorm)) * terrainNorm + preVelocity);;
   
    //m_ball.omega = omegaR * m_ball.radius;
    omegaR = omegaR * -1;
    m_ball.omega = omegaR;
    //m_ball.omega = omegaR * .10472; // conversion from rpm to rad per second
}

void GolfBall::LaunchProjectile()
{
    PushFlightData();

    // Fly ball on an upward trajectory until it stops climbing
    BallMotion flightData;
    double dt = m_timeStep;
    float maxHeight = m_ball.launchHeight;
    float bounceHeight = m_ball.launchHeight;
    SetInitialSpinRate(m_ball.omega);

    int count = 0;    
    bool isBallFlyOrBounce = true;
    while (isBallFlyOrBounce == true)
    {
        bool doesCollisionBeforeApex = false;
        bool isBallAscending = true;
        while (isBallAscending == true)
        {
            ProjectileRungeKutta4(&m_ball, dt);
            UpdateSpinRate(dt);
            flightData = this->m_ball.q;
            PushFlightData();

            if (m_ball.q.velocity.y <= 0.0)
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

            if (GetBallFlightAltitude(m_ball.q.position) < 0.0)
            {
                //isBallAscending = false;
                //doesCollisionBeforeApex = true;
                //SetLandingSpinRate(m_ball.omega);
                //LandProjectile();
            }
        }

        if (doesCollisionBeforeApex == false)
        {
            double previousTime = m_ball.q.time;
            float currentAlt = GetBallFlightAltitude(m_ball.q.position);
            float prevFlightAlt = currentAlt;
            //  Calculate ball decent path until it reaches landing area height
            while (currentAlt > 0.0)
            {
                prevFlightAlt = currentAlt;
                previousTime = m_ball.q.time;
                ProjectileRungeKutta4(&m_ball, dt);
                UpdateSpinRate(dt);
                flightData = this->m_ball.q;
                PushFlightData();
                currentAlt = GetBallFlightAltitude(m_ball.q.position);
            }


            if (m_ballPath.size() > 1)
            {
                double rollBackTime = CalculateImpactTime(previousTime, m_ball.q.time, prevFlightAlt, currentAlt);
                ProjectileRungeKutta4(&m_ball, -rollBackTime);
                m_ballPath[m_ballPath.size() - 1] = m_ball.q;
            }
        }

        SetLandingSpinRate(m_ball.omega);
        LandProjectile();

        ++m_bounceCount;
        ++count;
        if (m_ball.q.velocity.y < 3.9 || count > 19 || bounceHeight < .3) // WLJ bounce height threshold is just a guess at this point
        {
            isBallFlyOrBounce = false;
        }
        bounceHeight = m_ball.landingHeight;
        SetBallToTerrain(m_ball.q.position - m_shotOrigin);
        //isBallFlyOrBounce = false;
    }

    SetMaxHeight(maxHeight);

    //m_ball.q.velocity.y = 0.0;
    //SetBallToTerrain(m_ball.q.position - m_shotOrigin);
    //PushFlightData();

    RollBall();
    SetLandingCordinates(m_ball.q.position);
}

// WIP : Work on pause until 3D terrain work finished to work with landing function
void GolfBall::PrepProjectileLaunch(Utility::ImpactData aImpactData)
{ 
    //  Reset ball rotional axis and spin rate prior 
    //  prepping new launch data
    m_ball.rotationAxis.x = 0.0;
    m_ball.rotationAxis.y = 0.0;
    m_ball.rotationAxis.z = 1.0;
    m_ball.omega = 0.0;

    //  Convert the loft angle from degrees to radians and
    //  assign values to some convenience variables.
    float loft = Utility::ToRadians(aImpactData.angleY);
    //float cosL = cos(loft);
    float sinL = sin(loft);

    //  Calculate the pre-collision velocities normal
    //  and parallel to the line of action.
    float velocity = aImpactData.velocity;
    //float vcp = cosL * velocity;
    float vcn = -sinL * velocity;

    //  Compute the post-collision velocity of the ball
    //  along the line of action.
    float ballMass = m_ball.mass;
    float clubMass = aImpactData.mass;
    float e = aImpactData.cor; //  coefficient of restitution of club face striking the ball
    //float vbp = (1.0 + e) * clubMass * vcp / (clubMass + ballMass);

    //  Compute the post-collision velocity of the ball
    //  perpendicular to the line of action.
    //float vbn = (1.0 - m_faceRoll) * clubMass * vcn / (clubMass + ballMass);

    //  Compute the initial spin rate assuming ball is
    //  rolling without sliding.
    float radius = m_ball.radius;
    float omega = m_faceRoll * vcn / radius;

    //  Rotate post-collision ball velocities back into 
    //  standard Cartesian frame of reference. Because the
    //  line-of-action was in the xy plane, the z-velocity
    //  is zero.
    //float vx0 = cosL * vbp - sinL * vbn;
    //float vy0 = sinL * vbp + cosL * vbn;
    //float vz0 = 0.0;

    DirectX::SimpleMath::Vector3 vBall = (static_cast<float>((((1.0 + e) * clubMass) / (clubMass + ballMass))) * aImpactData.vHeadNormal)
        + (static_cast<float>(((2 * clubMass) / (7 * (clubMass + ballMass)))) * aImpactData.vHeadParallel);

    DirectX::SimpleMath::Vector3 unitVHead = aImpactData.vHead;
    unitVHead.Normalize();
    DirectX::SimpleMath::Vector3 unitFaceNormal = aImpactData.vFaceNormal;
    unitFaceNormal.Normalize();

    DirectX::SimpleMath::Vector3 crossVheadvFace = unitVHead.Cross(unitFaceNormal);

    float absVhP = sqrt((aImpactData.vHeadParallel.x * aImpactData.vHeadParallel.x)
        + (aImpactData.vHeadParallel.y * aImpactData.vHeadParallel.y) 
        + (aImpactData.vHeadParallel.z * aImpactData.vHeadParallel.z));

    DirectX::SimpleMath::Vector3 omegaBall = static_cast<float>(((5.0 * absVhP) / (7.0 * m_ball.radius))) * crossVheadvFace;
    
    /*
    float absOmegaBall = sqrt((omegaBall.x * omegaBall.x)
        + (omegaBall.y * omegaBall.y)
        + (omegaBall.z * omegaBall.z));
    float absvBall = sqrt((vBall.x * vBall.x)
        + (vBall.y * vBall.y)
        + (vBall.z * vBall.z));
    */

    DirectX::SimpleMath::Vector3 absOmegaBall = DirectX::XMVectorAbs(omegaBall);
    DirectX::SimpleMath::Vector3 absvBall = DirectX::XMVectorAbs(vBall);

    //float cL = -0.05 + sqrt(0.0025 + 0.36 * ((m_ball.radius * absOmegaBall) / absvBall));
    //DirectX::SimpleMath::Vector3 testCL2 = DirectX::XMVectorSqrt((0.0025 + 0.36) * ((m_ball.radius * absOmegaBall) / absvBall));

    //DirectX::SimpleMath::Vector3 fMangus = (static_cast<float>(.5f * m_ball.airDensity * m_ball.area * cL * absvBall * absvBall))
    //    * (unitFaceNormal.Cross(unitVHead));

    //DirectX::SimpleMath::Vector3 fMangus = ((.5f * m_ball.airDensity * m_ball.area * cL * absvBall * absvBall)) * (unitFaceNormal.Cross(unitVHead));

    //DirectX::SimpleMath::Vector3 normfManus = fMangus;
    //DirectX::SimpleMath::Vector3 normOmegaBall = omegaBall;

    //normfManus.Normalize();
    //normOmegaBall.Normalize();

    m_ball.omega = omega;
   
    m_ball.q.velocity.x = vBall.x;
    m_ball.q.velocity.y = vBall.y;
    m_ball.q.velocity.z = vBall.z;

    // Turn velocity to shot aim direction
    m_ball.q.velocity = DirectX::SimpleMath::Vector3::Transform(m_ball.q.velocity, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aImpactData.directionDegrees))); 
    
    // Tilt ball spin axis to curve shot, ToDo: Update from gameplay inputs
    m_ball.rotationAxis = DirectX::SimpleMath::Vector3::Transform(m_ball.rotationAxis, DirectX::SimpleMath::Matrix::CreateRotationX(aImpactData.ballAxisTilt));

    // Turn ball spin axis to shot aim direction
    m_ball.rotationAxis = DirectX::SimpleMath::Vector3::Transform(m_ball.rotationAxis, DirectX::SimpleMath::Matrix::CreateRotationY(Utility::ToRadians(aImpactData.directionDegrees)));
}

void GolfBall::PushFlightData()
{    
    m_ballPath.push_back(m_ball.q);
}

//  This method loads the right-hand sides for the projectile ODEs
void GolfBall::ProjectileRightHandSide(struct SpinProjectile* pBall, BallMotion* q, BallMotion* deltaQ, double aTimeDelta, float aQScale, BallMotion* dq)
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
    float vx = newQ.velocity.x;
    float vy = newQ.velocity.y;
    float vz = newQ.velocity.z;

    //  Compute the apparent velocities bz subtracting
    //  the wind velocity components from the projectile
    //  velocity components.
    float vax = vx - pBall->windSpeed.x;
    float vay = vy - pBall->windSpeed.y;
    float vaz = vz - pBall->windSpeed.z;

    //  Compute the apparent velocity magnitude. The 1.0e-8 term
    //  ensures there won't be a divide bz yero later on
    //  if all of the velocity components are zero.
    float va = sqrt(vax * vax + vay * vay + vaz * vaz) + 1.0e-8f;

    //  Compute the total drag force.
    float Fd = 0.5f * pBall->airDensity * pBall->area * pBall->dragCoefficient * va * va;
    float Fdx = -Fd * vax / va;
    float Fdy = -Fd * vay / va;
    float Fdz = -Fd * vaz / va;

    // ToDo: WLJ look into coverting drag forces to vector, this does not apply wind to flight
    DirectX::SimpleMath::Vector3 absVball = DirectX::XMVectorAbs(newQ.velocity);
    DirectX::SimpleMath::Vector3 vBallNorm = newQ.velocity;
    vBallNorm.Normalize();   
    DirectX::SimpleMath::Vector3 Fd3D = -(0.5f * pBall->airDensity * pBall->area * m_ball.dragCoefficient * (absVball * absVball)) * vBallNorm;
    //float Fdx1 = Fd3D.x;
    //float Fdy1 = Fd3D.y;
    //float Fdz1 = Fd3D.z;

    //float Fdx2 = Fd3D.x * vax / va;
    //float Fdy2 = Fd3D.y * vay / va;
    //float Fdz2 = Fd3D.z * vaz / va;

    //Fdx = Fdx2;
    //Fdy = Fdy2;
    //Fdz = Fdz2;

    //  Compute the velocity magnitude
    float v = sqrt(vx * vx + vy * vy + vz * vz) + 1.0e-8f;

    //  Evaluate the Magnus force terms.
    float Cl = -0.05f + sqrt(0.0025f + 0.36f * fabs(pBall->radius * pBall->omega / v));  // this equation gives a more accurate representation to fit experimental data than Cl = (radius * omega)/v
    float Fm = 0.5f * pBall->airDensity * pBall->area * Cl * v * v;
    float Fmx = (vz * pBall->rotationAxis.y - pBall->rotationAxis.z * vy) * Fm / v;
    float Fmy = (vx * pBall->rotationAxis.z - pBall->rotationAxis.x * vz) * Fm / v;
    float Fmz = -(vx * pBall->rotationAxis.y - pBall->rotationAxis.x * vy) * Fm / v;

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
    pBall->q.time = pBall->q.time + aTimeDelta;

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
    const int numEqns = pBall->numEqns;
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
    pBall->q.time = pBall->q.time + aTimeDelta;
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
    m_landingCordinates = m_shotOrigin;
    m_bounceCount = 0;
    m_ballPath.clear();
    m_ball.q.time = 0.0;
    m_ball.omega = 0.0;
    m_ball.q.position.x = 0.0;
    m_ball.q.position.y = 0.0;
    m_ball.q.position.z = 0.0;
    m_ball.q.velocity.x = 0.0;
    m_ball.q.velocity.y = 0.0;
    m_ball.q.velocity.z = 0.0;
    m_debugValue01 = 0.0;
    m_debugValue02 = 0.0;
    m_debugValue03 = 0.0;
    m_debugValue04 = 0.0;
    m_debugDrawLines.clear();
}

void GolfBall::ResetIsInHole()
{
    m_isBallInHole = false;
}

void GolfBall::RollBall()
{
    float pg = 0.131f; //0.131 is from A. Raymond Penner "The Run of a Golf Ball" doc
    float g = m_ball.gravity;
    float a = -(5.0f / 7.0f) * pg * g; // a = 0.916999996	float

    float decelFactor = a;
    decelFactor = .69999;
    //decelFactor = 0.000001;
    decelFactor = 0.6;
    //float stopTolerance = 0.05;
    float stopTolerance = 0.5;
    int overflowTolerance = 6650;

    //DirectX::SimpleMath::Vector3 directionVec = m_ball.q.velocity;
    //directionVec.y = 0.0;
    //directionVec.Normalize();

    bool isBallInHoleRadius = false;
    bool isBallInHole = false;
    DirectX::SimpleMath::Vector3 posOnEnteringHoleRadius;
    double timeOnEnteringHoleRadius;

    DirectX::SimpleMath::Vector3 startVelocity = m_ball.q.velocity;
    m_ball.q.velocity.y = 0.0f;
    int i = 0;
    while (m_ball.q.velocity.Length() > stopTolerance && i < overflowTolerance)
    {      
        if (isBallInHoleRadius == false)
        {
            if (GetDistanceToHole() < pBallEnvironment->GetHoleRadius())
            {
                isBallInHoleRadius = true;
                posOnEnteringHoleRadius = m_ball.q.position;
                timeOnEnteringHoleRadius = m_ball.q.time;
            }
        }
        if (isBallInHoleRadius == true)
        {
            if (GetDistanceToHole() >= pBallEnvironment->GetHoleRadius())
            {
                isBallInHoleRadius = false;
                isBallInHole = DoesBallRollInHole(posOnEnteringHoleRadius, timeOnEnteringHoleRadius, m_ball.q.position, m_ball.q.time);
                if (isBallInHole == false)
                {
                    posOnEnteringHoleRadius = m_ball.q.position;
                    timeOnEnteringHoleRadius = m_ball.q.time;
                }
            }
            else if (m_ball.q.velocity.Length() < 0.1)
            {
                isBallInHole = true;
            }
        }

        if (isBallInHole == false)
        {
            float velocity = m_ball.q.velocity.Length();
            velocity -= decelFactor * velocity * m_timeStep;

            DirectX::SimpleMath::Vector3 tragectoryNormilized = m_ball.q.velocity;
            tragectoryNormilized.Normalize();


            //////////////////////////////////////
            /// Testing  terrain velocity effects
            //////////////////////////////////////
            /*
            DirectX::SimpleMath::Vector3 gravity(0.0, 9.8, 0.0);
            DirectX::SimpleMath::Vector3 terrainNorm = pBallEnvironment->GetTerrainNormal(GetBallPosInEnviron(m_ball.q.position));

            //inline Vector3D Project(const Vector3D & a, const Vector3D & b)
               // return (b * (Dot(a, b) / Dot(b, b)));
                //Vector3f slopeAcceleration = gravity.subtract(gravity.project(slopeNormal));
                //velocity = velocity.add(slopeAcceleration.mult(secondsSinceLastFrame));
            DirectX::SimpleMath::Vector3 projection = (terrainNorm * (gravity.Dot(terrainNorm) / terrainNorm.Dot(terrainNorm)));
            DirectX::SimpleMath::Vector3 slopeAcceleration = gravity - projection;
            //DirectX::SimpleMath::Vector3 xVelocity = m_ball.q.velocity + (slopeAcceleration * m_timeStep);
            DirectX::SimpleMath::Vector3 xVelocity = m_ball.q.velocity + (slopeAcceleration);
            //xVelocity *= .2;


            float frictionFactor = -0.05;
            //float frictionFactor = -1.0;
            DirectX::SimpleMath::Vector3 friction = frictionFactor * m_ball.q.velocity;
            if (m_ball.q.velocity.Length() < 1.0)
            {
                //friction = DirectX::SimpleMath::Vector3::Zero;
            }

            DirectX::SimpleMath::Vector3 accelerationTerrain = terrainNorm * 0.1f;
            //DirectX::SimpleMath::Vector3 accelerationTerrain = terrainNorm;
            DirectX::SimpleMath::Vector3 acceleration = accelerationTerrain + friction;
            //DirectX::SimpleMath::Vector3 speed = m_ball.q.velocity + m_timeStep * acceleration;
            //DirectX::SimpleMath::Vector3 speed = m_ball.q.velocity + m_timeStep * acceleration * decelFactor;
            DirectX::SimpleMath::Vector3 speed = m_ball.q.velocity + m_timeStep * acceleration * .5;
            DirectX::SimpleMath::Vector3 speed2 = m_ball.q.velocity * acceleration;
            DirectX::SimpleMath::Vector3 speed3 = m_ball.q.velocity + m_timeStep * acceleration * .9;

            //speed *= decelFactor;

            m_ball.q.velocity = speed;
            */
            //////////////////////////////////////

            DirectX::SimpleMath::Vector3 preVelocity = m_ball.q.velocity;
            DirectX::SimpleMath::Vector3 gravity(0.0, 9.8, 0.0);
            DirectX::SimpleMath::Vector3 terrainNorm = pBallEnvironment->GetTerrainNormal(GetBallPosInEnviron(m_ball.q.position));
            DirectX::SimpleMath::Vector3 terrainNormTest = terrainNorm;
            terrainNormTest.Normalize();

            DirectX::SimpleMath::Vector3 testV1 = gravity + terrainNorm;
            DirectX::SimpleMath::Vector3 testV2 = gravity * terrainNorm;
            DirectX::SimpleMath::Vector3 testV3 = terrainNorm;
            testV3.y = 0.0f;
            testV3 *= .01;

            DirectX::SimpleMath::Vector3 testV4 = tragectoryNormilized * static_cast<float>(velocity) + testV3 * decelFactor;
            DirectX::SimpleMath::Vector3 testV5 = tragectoryNormilized * static_cast<float>(velocity) + testV3;

            DirectX::SimpleMath::Vector3 terrainAcceleration = terrainNorm;
            terrainAcceleration *= (m_ball.gravity * m_timeStep);
            //terrainAcceleration.y = 0.0f;
            //terrainAcceleration *= decelFactor;
            //m_ball.q.velocity = tragectoryNormilized * static_cast<float>(velocity) + testV3 * decelFactor;

            m_ball.q.velocity = tragectoryNormilized * static_cast<float>(velocity) - terrainAcceleration * decelFactor;
            //m_ball.q.velocity = tragectoryNormilized * static_cast<float>(velocity) * decelFactor;
            m_ball.q.position += m_ball.q.velocity * m_timeStep;

            // Temp while soriting out terrain following ball path
            //m_ball.q.position.y = GetBallFlightAltitude(m_ball.q.position);
            SetBallToTerrain(m_ball.q.position);

            m_ball.q.velocity.y = 0.0;

            m_ball.q.time = m_ball.q.time + m_timeStep;
        }
        else //stop the ball motion if its in the hole
        {
            m_isBallInHole = true;
            m_ball.q.velocity = DirectX::SimpleMath::Vector3::Zero;
        }

        PushFlightData();
        ++i;
    }

    if (i > overflowTolerance - 2)
    {
        m_ball.q.position.y += 10.0f;
        PushFlightData();
    }
}

void GolfBall::SetBallToTerrain(DirectX::XMFLOAT3& aPos)
{
    DirectX::XMFLOAT3 testPos = aPos;
    DirectX::XMFLOAT3 origin = m_shotOrigin;
    float originScaled = origin.y / pBallEnvironment->GetScale();

    DirectX::SimpleMath::Vector3 environPos = GetBallPosInEnviron(m_ball.q.position);
    DirectX::SimpleMath::Vector3 delta = environPos - m_shotOrigin;
    float terrainHeight = pBallEnvironment->GetTerrainHeightAtPos(environPos);
    //float terrainHeight = pBallEnvironment->GetTerrainHeightAtPos(delta);

    terrainHeight = terrainHeight / pBallEnvironment->GetScale();

    //aPos.y = terrainHeight;
    aPos.y = terrainHeight - originScaled + m_ball.radius;
}

void GolfBall::SetDefaultBallValues(Environment* pEnviron)
{
    m_ball.airDensity = pEnviron->GetAirDensity();
    m_ball.area = 0.001432;
    m_ball.dragCoefficient = 0.22;
    m_ball.q.time = 0.0;
    m_ball.gravity = pEnviron->GetGravity();
    m_ball.launchHeight = pEnviron->GetLauchHeight();
    m_ball.landingHeight = pEnviron->GetLandingHeight();
    m_ball.radius = 0.02135;
    m_ball.mass = 0.0459;
    m_ball.numEqns = 6;
    m_ball.omega = 0.0;
    m_ball.q.velocity.x = 0.0;
    m_ball.q.velocity.y = 0.0;
    m_ball.q.velocity.z = 0.0;
    m_ball.rotationAxis.x = 0.0;
    m_ball.rotationAxis.y = 0.0;
    m_ball.rotationAxis.z = 1.0; // ball will only be spinning about the z axis, this will need to be adjusted if/when imperfect impact mechanics added for hooks and slices
    m_ball.windSpeed.x = static_cast<float>(pEnviron->GetWindX());
    m_ball.windSpeed.y = static_cast<float>(pEnviron->GetWindY());
    m_ball.windSpeed.z = static_cast<float>(pEnviron->GetWindZ());

    m_isBallInHole = false;
}

void GolfBall::SetEnvironment(Environment* pEnviron)
{
    pBallEnvironment = pEnviron;
}

void GolfBall::SetLandingCordinates(DirectX::SimpleMath::Vector3 aCord)
{
    m_landingCordinates = aCord;
}

void GolfBall::SetLaunchPosition(const DirectX::SimpleMath::Vector3 aPos)
{
    m_shotOrigin = aPos;
}

void GolfBall::SetSpinAxis(DirectX::SimpleMath::Vector3 aAxis)
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
    // Slow speed to approch zero for eithe positive or negative spin rates
    if (m_ball.omega < 0.0)
    {
        m_ball.omega *= 1.0f - (static_cast<float>(aTimeDelta) * m_spinRateDecay);
    }
    else
    {
        m_ball.omega *= 1.0f + (static_cast<float>(aTimeDelta) * m_spinRateDecay);
    }
}

void GolfBall::ZeroDataForUI()
{
    m_initialSpinRate = 0.0;
    m_landingSpinRate = 0.0;
    m_shotOrigin = DirectX::SimpleMath::Vector3::Zero;
    m_landingCordinates = DirectX::SimpleMath::Vector3::Zero;;
    m_maxHeight = 0.0;
}

