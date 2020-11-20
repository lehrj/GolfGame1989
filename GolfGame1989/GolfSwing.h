#pragma once
#include "Environment.h"
#include "GolfBag.h"
#include "Utility.h"
#include "Vector4d.h"
#include "GolfCharacter.h"

// Class to handle club functionality
class GolfSwing
{
public:
    GolfSwing();
    ~GolfSwing();

    Utility::ImpactData CalculateLaunchVector();

    void CycleClub(const bool aIsCycleClubUp);

    void InputClub(int aInput);
    const double GetArmLength() const { return m_armLength; };
    
    double GetBackSwingPercentage() const { return m_impactData.power; };
    double GetBallPlacementAngle() const { return m_ballPlacementAngle; };
    double GetClubAngle() const { return m_club.angle; };
    double GetClubLength() const { return m_club.length; };
    double GetClubMass() const { return m_club.mass; };
    std::string GetClubName() const { return m_club.clubName; };
    DirectX::SimpleMath::Vector3 GetFaceNormal() const { return m_impactData.vFaceNormal; };
    double GetLaunchAngle() const { return m_launchAngle; };
    
    double GetLaunchVelocity() const { return m_impactData.velocity; };
    std::vector<DirectX::SimpleMath::Vector3> GetRawAlphaBetaTheta() const { return m_alphaBetaThetaVec; };
    DirectX::SimpleMath::Vector3 GetShoulderPos();
    int GetSwingImpactStep() const { return m_swingImpactStep; };
    int GetSwingStepIncrementCount() const { return m_swingStepIncrementCount; };

    void ResetAlphaBeta();

    void SetArmBalancePoint(double aBalancePoint);
    void SetArmLength(double aLength);
    void SetArmMass(double aArmMass);
    void SetArmMassMoI(double aArmMassMoi);
    void SetBackSwingPercentage(double aPercentage);
    void SetBallPlacementAngle(double aAngle);

    void SetClubAngle(double aAngle);
    void SetClubCoR(double aCoR);
    void SetClubLength(double aLength);
    void SetClubLengthModifier(double aLengthModifier);
    void SetClubMass(double aMass);
    void SetDefaultSwingValues(double aGravity);
    void SetImpactDirectionDegrees(const double aDirectionDegrees);
    void UpdateGolfer();
    void UpdateGolfSwingValues();
    void UpdateGravityDependants(const double aGravity);
    void UpdateImpactData(Utility::ImpactData aImpactData);
    void UpdateImpactDataAxis(const double aAxisAngle);
    void UpdateImpactDataPlane(const double aPlaneAngle);   
    void ZeroDataForUI();

private:
    double ComputeAlphaDotDot(void) const;
    double ComputeBetaDotDot(void) const;
    void SetBeta(double aBeta);
    void SetQalpha(double aQalpha);
    void SetQbeta(double aQbeta);
    void SetShoulderAccel(double aShoulderAccel);
    double VerifySwingGravityDirection(double aGravity) const;

    GolfBag*                                    m_pBag;
    GolfClub                                    m_club;

    Utility::ImpactData                         m_impactData;
    int                                         m_clubIndex = 2;

    std::vector<DirectX::SimpleMath::Vector3>   m_alphaBetaThetaVec;

    std::vector<DirectX::SimpleMath::Vector3>   m_alphaCord;
    std::vector<DirectX::SimpleMath::Vector3>   m_betaCord;
    std::vector<DirectX::SimpleMath::Vector3>   m_thetaCord;

    double                                      m_launchVelocity;
    double                                      m_launchAngle;
    double                                      m_launchImpact;
    
    const double                                m_timeDelta = 0.0025; // WLJ removed magic number from CalculateLaunchVector(), ToDo: Search through code to elimnate and replace others and factor with m_swingStepIncrementCount below
    const int                                   m_swingStepIncrementCount = 200;
    int                                         m_swingImpactStep = 0;
    double                                      m_alpha; // Angle swept by arm rod from initial backswing position in radians
    double                                      m_alpha_dot;
    double                                      m_alpha_dotdot;
    double                                      m_armBalancePoint;
    double                                      m_armFirstMoment; // First moment of the arm rod about the shoulder axis kg m
    double                                      m_armLength; // Length arm rod in m
    double                                      m_armMass;
    double                                      m_armMassMoI; // Mass moment of inertia of the rod representing the arm in kg m^2
    double                                      m_backSwingPercentage;
    double                                      m_ballPlacementAngle;
    double                                      m_beta; // Wrist cock angle in radians 120.0 in radians
    double                                      m_beta_dot;
    double                                      m_beta_dotdot;
    double                                      m_clubLengthModifier = 1.0;
    double                                      m_gamma; // 135.0 degrees in radians
    double                                      m_gravity; // Gravity's acceleration m/s^2
    double                                      m_Qalpha; // Torque applied at the shoulder to the arm rod in N m
    double                                      m_Qbeta; // Torque applied at the wrist joint to the club rod in N m   
    double                                      m_shoulderHorizAccel; // Horizontal acceleration of the shoulder in  m/s^2
    double                                      m_theta;// = m_gamma - m_alpha, Angle between arm rod and vertical axis in radians
};