#pragma once
#include <vector>
#include "Environment.h"
#include "Utility.h"
#include "Vector4d.h"
#include "GolfSwing.h"

struct BallMotion
{
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 velocity;
};

struct SpinProjectile
{
    double          airDensity;
    double          area;
    double          dragCoefficient;
    double          flightTime;
    double          gravity;
    double          launchHeight;
    double          landingHeight;
    double          mass;
    int             numEqns;  // number of equations to solve
    double          omega;  //  angular velocity, m/s
    BallMotion      q;      //  ball position and velocities
    DirectX::SimpleMath::Vector3 rotationAxis;
    double          radius; //  sphere radius, m
    DirectX::SimpleMath::Vector3 windSpeed;
};

// Class to handle the balls
class GolfBall
{
public:
    double CalculateImpactTime(double aTime1, double aTime2, double aHeight1, double aHeight2);
    void FireProjectile(Utility::ImpactData aImpact);

    DirectX::SimpleMath::Vector3 GetBallPosInEnviron(DirectX::SimpleMath::Vector3 aPos) const;

    int GetBounceCount() const { return m_bounceCount; };
    double GetDebugValue01() const { return m_debugValue01; };
    double GetDebugValue02() const { return m_debugValue02; };
    double GetDebugValue03() const { return m_debugValue03; };
    double GetDebugValue04() const { return m_debugValue04; };
    float GetDistanceToHole() const;
    DirectX::SimpleMath::Vector3 GetLandingCordinates() const { return m_landingCordinates; };
    DirectX::SimpleMath::Vector3 GetLandingCordinates2() const { return m_landingImpactCordinates; };
    double GetMaxHeight() const { return m_maxHeight; };   
    double GetImpactAngle() const;
    double GetImpactDirection() const;
    DirectX::SimpleMath::Plane GetImpactPlane() const;
    float GetImpactVelocity() const;
    double GetInitialSpinRate() const { return m_initialSpinRate; };
    double GetLandingSpinRate() const { return m_landingSpinRate; };
    double GetLandingHeight() const;
    double GetShotDistance() const;
    double GetShotFlightDistance() const;
    std::vector<float>& GetShotTimeSteps() { return m_shotPathTimeStep; };
  
    std::vector<DirectX::SimpleMath::Vector3>& OutputShotPath() { return m_shotPath; };
    void PrepProjectileLaunch(Utility::ImpactData aImpactData);
    void ResetBallData();
    void SetDefaultBallValues(Environment* pEnviron);
    void SetEnvironment(Environment* pEnviron);
    void SetLaunchPosition(const DirectX::SimpleMath::Vector3 aPos);
    void ZeroDataForUI();

private:
    bool DoesBallRollInHole(const DirectX::SimpleMath::Vector3 aEnterRadiusPos, const double aEnterRadiusTime, const DirectX::SimpleMath::Vector3 aExitRadiusPos, const double aExitRadiusTime);
    DirectX::SimpleMath::Vector3 GetPostCollisionVelocity(const DirectX::SimpleMath::Vector3 aVec1, const DirectX::SimpleMath::Vector3 aVec2, const DirectX::SimpleMath::Vector3 aVec3, const float aHeightDrop) const;
    void LandProjectile();
    void LaunchProjectile();
    void SetInitialSpinRate(const double aSpinRate) { m_initialSpinRate = aSpinRate; };
    void SetLandingSpinRate(const double aSprinRate) { m_landingSpinRate = aSprinRate; };
    void SetLandingCordinates(DirectX::SimpleMath::Vector3 aCord);
    void SetMaxHeight(const double aMaxHeight) { m_maxHeight = aMaxHeight; };
    void SetSpinAxis(DirectX::SimpleMath::Vector3 aAxis);
    void RollBall();
    void ProjectileRightHandSide(struct SpinProjectile* projectile, BallMotion* q, BallMotion* deltaQ, double aTimeDelta, double aQScale, BallMotion* dq);
    void ProjectileRungeKutta4(struct SpinProjectile* projectile, double aTimeDelta);
    void ProjectileRungeKutta4wPointers(struct SpinProjectile* projectile, double aTimeDelta);
    void PushFlightData();
    void UpdateSpinRate(double aTimeDelta);

    SpinProjectile                              m_ball;
    int                                         m_bounceCount = 0;
    const double                                m_faceRoll = 0.7142857142857143; // <== 5/7, represents the ball moving up the club face to impart spin
    double                                      m_initialSpinRate;
    bool                                        m_isBallInHole = false;
    double                                      m_landingSpinRate;
    DirectX::SimpleMath::Vector3                m_landingCordinates;
    DirectX::SimpleMath::Vector3                m_landingImpactCordinates;
    double                                      m_maxHeight;
    std::vector<DirectX::SimpleMath::Vector3>   m_shotPath;
    std::vector<float>                          m_shotPathTimeStep;
    DirectX::SimpleMath::Vector3                m_shotOrigin;
    const double                                m_spinRateDecay = 0.04; // Rate at which the spinrate slows over time, using value from Trackman launch monitors of 4% per second
    const float                                 m_timeStep = 0.01f;

    double                                      m_debugValue01 = 0.0;
    double                                      m_debugValue02 = 0.0;
    double                                      m_debugValue03 = 0.0;
    double                                      m_debugValue04 = 0.0;

    Environment const *                         pBallEnvironment;
};


