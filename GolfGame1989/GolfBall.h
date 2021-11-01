#pragma once
#include <vector>
#include "Environment.h"
#include "Utility.h"
#include "GolfSwing.h"

struct BallMotion
{
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 velocity;
    double time{};
};

struct SpinProjectile
{
    float          airDensity;
    float          area;
    float          dragCoefficient;
    //float          flightTime;
    float          gravity;
    float          launchHeight;
    float          landingHeight;
    float          mass;
    int             numEqns;  // number of equations to solve
    float          omega;  //  angular velocity, m/s
    BallMotion      q;      //  ball position and velocities
    DirectX::SimpleMath::Vector3 rotationAxis;
    float          radius; //  sphere radius, m
    DirectX::SimpleMath::Vector3 windSpeed;
};

// Class to handle the balls
class GolfBall
{
public:
    double CalculateImpactTime(double aTime1, double aTime2, float aHeight1, float aHeight2);
    void FireProjectile(Utility::ImpactData aImpact);

    DirectX::SimpleMath::Vector3 GetBallPosInEnviron(DirectX::SimpleMath::Vector3 aPos) const;
    float GetBallRadius() { return m_ball.radius; };

    int GetBounceCount() const { return m_bounceCount; };
    std::vector<std::pair< DirectX::VertexPositionColor, DirectX::VertexPositionColor>> GetDebugLines() const { return m_debugDrawLines; };
    float GetDebugValue01() const { return m_debugValue01; };
    float GetDebugValue02() const { return m_debugValue02; };
    float GetDebugValue03() const { return m_debugValue03; };
    float GetDebugValue04() const { return m_debugValue04; };
    float GetDistanceToHole() const;
    float GetImpactAngleDeg() const;
    float GetImpactDirection() const;
    DirectX::SimpleMath::Plane GetImpactPlane() const;
    float GetInitialSpinRate() const { return m_initialSpinRate; };
    bool GetIsInHole() const { return m_isBallInHole; };
    DirectX::SimpleMath::Vector3 GetLandingCordinates() const { return m_landingCordinates; };
    DirectX::SimpleMath::Vector3 GetLandingCordinates2() const { return m_landingImpactCordinates; };
    float GetLandingSpinRate() const { return m_landingSpinRate; };
    float GetLandingHeight() const;
    float GetMaxHeight() const { return m_maxHeight; };
    float GetShotDistance() const;
    float GetShotFlightDistance() const;
  
    std::vector<BallMotion>& OutputBallPath() { return m_ballPath; };

    void PrepProjectileLaunch(Utility::ImpactData aImpactData);
    void ResetBallData();
    void ResetIsInHole();
    void SetDefaultBallValues(Environment* pEnviron);
    void SetEnvironment(Environment* pEnviron);
    void SetLaunchPosition(const DirectX::SimpleMath::Vector3 aPos);
    void ZeroDataForUI();

private:
    void AddDebugDrawLines(DirectX::SimpleMath::Vector3 aOriginPos, DirectX::SimpleMath::Vector3 aLine, DirectX::XMVECTORF32 aColor);
    //bool DoesBallRollInHole(const DirectX::SimpleMath::Vector3 aEnterRadiusPos, const double aEnterRadiusTime, const DirectX::SimpleMath::Vector3 aExitRadiusPos, const double aExitRadiusTime);
    bool DoesBallRollInHole(const double aEnterRadiusTime, const double aExitRadiusTime);
    DirectX::SimpleMath::Vector3 GetPostCollisionVelocity(const float aHeightDrop);
    float GetBallFlightAltitude(DirectX::SimpleMath::Vector3 aPos);
    void LandProjectile();
    void LaunchProjectile();
    void SetBallToTerrain(DirectX::XMFLOAT3& aPos);
    void SetInitialSpinRate(const float aSpinRate) { m_initialSpinRate = aSpinRate; };
    void SetLandingSpinRate(const float aSprinRate) { m_landingSpinRate = aSprinRate; };
    void SetLandingCordinates(DirectX::SimpleMath::Vector3 aCord);
    void SetMaxHeight(const float aMaxHeight) { m_maxHeight = aMaxHeight; };
    void SetSpinAxis(DirectX::SimpleMath::Vector3 aAxis);
    void RollBall();
    void ProjectileRightHandSide(struct SpinProjectile* projectile, BallMotion* q, BallMotion* deltaQ, double aTimeDelta, float aQScale, BallMotion* dq);
    void ProjectileRungeKutta4(struct SpinProjectile* projectile, double aTimeDelta);
    void ProjectileRungeKutta4wPointers(struct SpinProjectile* projectile, double aTimeDelta);
    void PushFlightData();
    void UpdateSpinRate(double aTimeDelta);

    SpinProjectile                              m_ball;
    int                                         m_bounceCount = 0;
    const float                                m_faceRoll = 0.7142857142857143; // <== 5/7, represents the ball moving up the club face to impart spin
    float                                      m_initialSpinRate;
    bool                                        m_isBallInHole = false;
    float                                      m_landingSpinRate;
    DirectX::SimpleMath::Vector3                m_landingCordinates;
    DirectX::SimpleMath::Vector3                m_landingImpactCordinates;
    float                                      m_maxHeight;

    std::vector<BallMotion>                     m_ballPath;

    DirectX::SimpleMath::Vector3                m_shotOrigin;
    const float                                m_spinRateDecay = 0.04; // Rate at which the spinrate slows over time, using value from Trackman launch monitors of 4% per second
    //const float                                 m_timeStep = 0.01f;
    const float                                 m_timeStep = 0.01f;
    float                                      m_debugValue01 = 0.0;
    float                                      m_debugValue02 = 0.0;
    float                                      m_debugValue03 = 0.0;
    float                                      m_debugValue04 = 0.0;

    Environment const *                         pBallEnvironment;

    std::vector<std::pair< DirectX::VertexPositionColor, DirectX::VertexPositionColor>> m_debugDrawLines;
};


