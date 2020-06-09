#pragma once
#include <vector>
#include "Environment.h"
#include "Utility.h"
#include "Vector4d.h"

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
    /*
    q[0] = vx, velocity
    q[1] = x position
    q[2] = vy, velocity
    q[3] = y position
    q[4] = vz, velocity
    q[5] = z position
    */
    double rx;     //  spin axis vector component
    double ry;     //  spin axis vector component
    double rz;     //  spin axis vector component
    double radius; //  sphere radius, m
    double windVx;
    double windVy;
    double windVz;
};

class GolfBall
{
public:
    void OutputPosition();
    std::vector<double> OutputXvals();
    std::vector<double> OutputYvals();
    std::vector<double> OutputZvals();
    const int GetXvecSize() { return m_xVals.size(); };
    const int GetYvecSize() { return m_yVals.size(); };
    const int GetZvecSize() { return m_zVals.size(); };
    double GetIndexX(const int aIndex);
    double GetIndexY(const int aIndex);
    double GetIndexZ(const int aIndex);
    const Vector4d GetLandingCordinates() { return m_landingCordinates; };
    const double GetMaxHeight() { return m_maxHeight; };
    const double GetShotDistance();
    const double GetInitialSpinRate() { return m_initialSpinRate; };
    const double GetLandingSpinRate() { return m_landingSpinRate; };
    double CalculateImpactTime(double aTime1, double aTime2, double aHeight1, double aHeight2);
    void FireProjectile(Vector4d aSwingInput, Environment* pEnviron);
    void LandProjectile(Environment* pEnviron);
    void LaunchProjectile();
    void PrepProjectileLaunch(Vector4d aSwingInput);
    
    void PrintFlightData();
    void PrintLandingData(Vector4d aLandingData, double aMaxY);
    
    void ProjectileRightHandSide(struct SpinProjectile* projectile,
        double* q, double* deltaQ, double ds,
        double qScale, double* dq);
    void ProjectileRungeKutta4(struct SpinProjectile* projectile, double aDs);
    void PushFlightData();
    void ResetBallData();
    void SetDefaultBallValues(Environment* pEnviron);
    void SetInitialSpinRate(const double aSpinRate) { m_initialSpinRate = aSpinRate; };
    void SetLandingSpinRate(const double aSprinRate) { m_landingSpinRate = aSprinRate; };
    void SetLandingCordinates(const double aX, const double aY, const double aZ);
    void SetMaxHeight(const double aMaxHeight) { m_maxHeight = aMaxHeight; };
    void UpdateSpinRate(double aTimeDelta);

private:
    SpinProjectile m_ball;
    const double m_faceRoll = 0.7142857142857143; // <== 5/7, represents the ball moving up the club face to impart spin
    const double m_spinRateDecay = 0.04; // Rate at which the spinrate slows over time, using value from Trackman launch monitors of 4% per second
    float m_timeStep;

    // test variables for adding planned graphical display of golf shot arc
    std::vector<double> m_xVals;
    std::vector<double> m_yVals;
    std::vector<double> m_zVals;
    
    double m_initialSpinRate;
    double m_landingSpinRate;
    Vector4d m_landingCordinates;
    double m_maxHeight;
    Vector4d m_shotOrigin;
};