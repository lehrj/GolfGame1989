#pragma once
#include "Environment.h"
#include "GolfBag.h"
#include "Utility.h"
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
    const float GetArmLength() const { return m_armLength; };
    
    float GetBackSwingPercentage() const { return m_impactData.power; };
    float GetBallPlacementAngle() const { return m_ballPlacementAngle; };
    float GetClubAngle() const { return m_club.angle; };
    float GetClubLength() const { return m_club.length; };
    float GetClubMass() const { return m_club.mass; };
    std::string GetClubName() const { return m_club.clubName; };
    DirectX::SimpleMath::Vector3 GetFaceNormal() const { return m_impactData.vFaceNormal; };
    float GetLaunchAngle() const { return m_launchAngle; };
    
    float GetLaunchVelocity() const { return m_impactData.velocity; };
    std::vector<DirectX::SimpleMath::Vector3> GetRawAlphaBetaTheta() const { return m_alphaBetaThetaVec; };
    DirectX::SimpleMath::Vector3 GetShoulderPos();
    int GetSwingImpactStep() const { return m_swingImpactStep; };
    int GetSwingStepIncrementCount() const { return m_swingStepIncrementCount; };

    void ResetAlphaBeta();

    void SetArmBalancePoint(float aBalancePoint);
    void SetArmLength(float aLength);
    void SetArmMass(float aArmMass);
    void SetArmMassMoI(float aArmMassMoi);
    void SetBackSwingPercentage(float aPercentage);
    void SetBallPlacementAngle(float aAngle);

    void SetClubAngle(float aAngle);
    void SetClubCoR(float aCoR);
    void SetClubLength(float aLength);
    void SetClubLengthModifier(float aLengthModifier);
    void SetClubMass(float aMass);
    void SetDefaultSwingValues(float aGravity);
    void SetImpactDirectionDegrees(const float aDirectionDegrees);
    void UpdateGolfer();
    void UpdateGolfSwingValues();
    void UpdateGravityDependants(const float aGravity);
    void UpdateImpactData(Utility::ImpactData aImpactData);
    void UpdateImpactDataAxis(const float aAxisAngle);
    void UpdateImpactDataPlane(const float aPlaneAngle);   
    void ZeroDataForUI();

private:
    float ComputeAlphaDotDot(void) const;
    float ComputeBetaDotDot(void) const;
    void SetBeta(float aBeta);
    void SetQalpha(float aQalpha);
    void SetQbeta(float aQbeta);
    void SetShoulderAccel(float aShoulderAccel);
    float VerifySwingGravityDirection(float aGravity) const;

    GolfBag*                                    m_pBag;
    GolfClub                                    m_club;

    Utility::ImpactData                         m_impactData;
    int                                         m_clubIndex = 2;

    std::vector<DirectX::SimpleMath::Vector3>   m_alphaBetaThetaVec;

    std::vector<DirectX::SimpleMath::Vector3>   m_alphaCord;
    std::vector<DirectX::SimpleMath::Vector3>   m_betaCord;
    std::vector<DirectX::SimpleMath::Vector3>   m_thetaCord;

    float                                      m_launchVelocity;
    float                                      m_launchAngle;
    float                                      m_launchImpact;
    
    const double                                m_timeDelta = 0.0025; // WLJ removed magic number from CalculateLaunchVector(), ToDo: Search through code to elimnate and replace others and factor with m_swingStepIncrementCount below
    const int                                   m_swingStepIncrementCount = 200;
    int                                         m_swingImpactStep = 0;
    float                                      m_alpha; // Angle swept by arm rod from initial backswing position in radians
    float                                      m_alpha_dot;
    float                                      m_alpha_dotdot;
    float                                      m_armBalancePoint;
    float                                      m_armFirstMoment; // First moment of the arm rod about the shoulder axis kg m
    float                                      m_armLength; // Length arm rod in m
    float                                      m_armMass;
    float                                      m_armMassMoI; // Mass moment of inertia of the rod representing the arm in kg m^2
    float                                      m_backSwingPercentage;
    float                                      m_ballPlacementAngle;
    float                                      m_beta; // Wrist cock angle in radians 120.0 in radians
    float                                      m_beta_dot;
    float                                      m_beta_dotdot;
    float                                      m_clubLengthModifier = 1.0;
    float                                      m_gamma; // 135.0 degrees in radians
    float                                      m_gravity; // Gravity's acceleration m/s^2
    float                                      m_Qalpha; // Torque applied at the shoulder to the arm rod in N m
    float                                      m_Qbeta; // Torque applied at the wrist joint to the club rod in N m   
    float                                      m_shoulderHorizAccel; // Horizontal acceleration of the shoulder in  m/s^2
    float                                      m_theta;// = m_gamma - m_alpha, Angle between arm rod and vertical axis in radians
};