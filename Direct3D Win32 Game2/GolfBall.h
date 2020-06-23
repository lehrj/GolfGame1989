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
    double airDensity;
    double area;
    double dragCoefficient;
    double flightTime;
    double gravity;
    double launchHeight;
    double landingHeight;
    double mass;
    int numEqns;  // number of equations to solve
    double omega;  //  angular velocity, m/s
    //double q[6];
    BallMotion q;
    /*
    q[0] = vx, velocity
    q[1] = x position
    q[2] = vy, velocity
    q[3] = y position
    q[4] = vz, velocity
    q[5] = z position
    */
    DirectX::SimpleMath::Vector3 rotationAxis;
    //double rx;     //  spin axis vector component
    //double ry;     //  spin axis vector component
    //double rz;     //  spin axis vector component
    double radius; //  sphere radius, m
    DirectX::SimpleMath::Vector3 windSpeed;
    //double windVx;
    //double windVy;
    //double windVz;
};

/*
struct SpinProjectile
{
    double airDensity;
    double area;
    double dragCoefficient;
    double flightTime;
    double gravity;
    double launchHeight;
    double landingHeight;
    double mass;
    int numEqns;  // number of equations to solve
    double omega;  //  angular velocity, m/s
    double q[6];
    
    //q[0] = vx, velocity
    //q[1] = x position
    //q[2] = vy, velocity
    //q[3] = y position
    //q[4] = vz, velocity
    //q[5] = z position
   
    double rx;     //  spin axis vector component
    double ry;     //  spin axis vector component
    double rz;     //  spin axis vector component
    double radius; //  sphere radius, m
    double windVx;
    double windVy;
    double windVz;
};
*/

class GolfBall
{
public:
    double CalculateImpactTime(double aTime1, double aTime2, double aHeight1, double aHeight2);
    DirectX::SimpleMath::Vector4 CalculateImpactVector(double aVelocity, double aFaceAngle, double aFaceRotation);
    void FireProjectile(Vector4d aSwingInput, Environment* pEnviron);
    void FireProjectile2(Utility::ImpactData aImpact, Environment* pEnviron);

    int GetColorIndex() { return m_drawColorIndex; };
    std::vector<int> GetColorVector() { return m_drawColorVector; };

    const int GetXvecSize() { return m_shotPath.size(); };
    const int GetYvecSize() { return m_shotPath.size(); };
    const int GetZvecSize() { return m_shotPath.size(); };

    const Vector4d GetLandingCordinates() { return m_landingCordinates; };
    const double GetMaxHeight() { return m_maxHeight; };
    const double GetShotDistance();
    //const DirectX::SimpleMath::Vector3 GetImpactAngle(DirectX::SimpleMath::Vector3 aPrevImpactPos, DirectX::SimpleMath::Vector3 aPostImpactPos);
    //const DirectX::SimpleMath::Vector3 GetImpactAngle();
    const float GetImpactAngle();
    const DirectX::SimpleMath::Plane GetImpactPlane();
    const float GetImpactVelocity();
    const double GetInitialSpinRate() { return m_initialSpinRate; };
    const double GetLandingSpinRate() { return m_landingSpinRate; };
    const double GetLandingHeight();
    //void LandProjectile(Environment* pEnviron);
    void LandProjectile();
    void LaunchProjectile();
    void LaunchProjectile2();
    void LaunchProjectilePostImpact();
    
    std::vector<DirectX::SimpleMath::Vector3> OutputShotPath() { return m_shotPath; };

    void PrepProjectileLaunch(Vector4d aSwingInput);
    void PrepProjectileLaunch2(Utility::ImpactData aImpactData);
    void PrepProjectileLaunch3(Utility::ImpactData aImpactData);

    void PrintFlightData();
    void PrintLandingData(Vector4d aLandingData, double aMaxY);
    
    //void ProjectileRightHandSide(struct SpinProjectile* projectile, double* q, double* deltaQ, double ds, double qScale, double* dq);
    void ProjectileRightHandSide(struct SpinProjectile* projectile, BallMotion* q, BallMotion* deltaQ, double ds, double qScale, BallMotion* dq);
    void ProjectileRungeKutta4(struct SpinProjectile* projectile, double aDs);
    void ProjectileRungeKutta4wPointers(struct SpinProjectile* projectile, double aDs);
    void PushFlightData();
    void ResetBallData();
    void RollBall();
    void RollRightHandSide(struct SpinProjectile* projectile,
        double* q, double* deltaQ, double ds,
        double qScale, double* dq);
    void RollRungeKutta4(struct SpinProjectile* projectile, double aDs);
    void SetDefaultBallValues(Environment* pEnviron);
    void SetInitialSpinRate(const double aSpinRate) { m_initialSpinRate = aSpinRate; };
    void SetLandingSpinRate(const double aSprinRate) { m_landingSpinRate = aSprinRate; };
    void SetLandingCordinates(const double aX, const double aY, const double aZ);
    void SetMaxHeight(const double aMaxHeight) { m_maxHeight = aMaxHeight; };
    void SetSpinAxis(DirectX::SimpleMath::Vector4 aAxis);
    void UpdateSpinRate(double aTimeDelta);

private:
    SpinProjectile m_ball;
    const double m_faceRoll = 0.7142857142857143; // <== 5/7, represents the ball moving up the club face to impart spin
    const double m_spinRateDecay = 0.04; // Rate at which the spinrate slows over time, using value from Trackman launch monitors of 4% per second
    float m_timeStep;
    int m_drawColorIndex = 0;
    //DirectX::SimpleMath::Vector4 m_drawColorVector;
    std::vector<int> m_drawColorVector;
    // test variables for adding planned graphical display of golf shot arc
    //std::vector<double> m_xVals;
    //std::vector<double> m_yVals;
    //std::vector<double> m_zVals;
    std::vector<DirectX::SimpleMath::Vector3> m_shotPath;

    double m_initialSpinRate;
    double m_landingSpinRate;
    Vector4d m_landingCordinates;
    double m_maxHeight;
    Vector4d m_shotOrigin;
};