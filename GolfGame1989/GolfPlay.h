#pragma once
#include "Utility.h"

// Gameplay Interface class 
class GolfPlay
{
public:
    GolfPlay();

    void DebugShot();
    void CalculateScoreString(const int aPar);
    float GetBackswingSet() const { return m_impactData.power; };
    std::vector<std::string> GetDebugData();
    Utility::ImpactData GetImpactData() const { return m_impactData; };
    float GetImpact() const { return m_swingImpactProcessed; };
    bool GetIsBackswingSet() const { return m_isOnDownSwing; };
    bool GetIsGameplayButtonReady() const { return m_isGameplayButtonReady; };
    float GetMeterPower() const { return m_meterBar; };
    float GetSwingPower() const { return m_swingPower; };
    float GetMeterLength() const { return abs(m_swingPowerMax) + abs(m_swingOverImpact); };
    float GetMeterImpactPoint() const { return GetMeterLength() - m_swingPowerMax; };   
    float GetShotDirection() const { return m_impactData.directionDegrees; };
    int GetSwingCount() const { return m_swingCount; };
    std::string GetUIScoreString() const { return m_scoreString; };
    void IncrementSwingCount();
    bool IsSwingStateAtImpact() const;
    void ResetGamePlayButton() { m_isGameplayButtonReady = true; };
    void ResetPlayData();
    void ResetSwingCount();
    void ResetSwingUpdateReady();   
    void SetAim(DirectX::SimpleMath::Vector3 aShotPos, DirectX::SimpleMath::Vector3 aTargetPos);
    void SetGameplayButtonReadyFalse() { m_isGameplayButtonReady = false; };
    void SetImpact();
    void SetPower();
    void StartSwing();
    void Swing();
    void TurnShotAim(float aTurn, float aTurnRate);
    bool UpdateSwing();
    void UpdateSwingState();

private:
    void BuildDebugData();

    Utility::ImpactData             m_impactData;
    bool                            m_isGameplayButtonReady = false;
    bool                            m_isLefty = false;
    bool                            m_isSwingPowerSet;
    bool                            m_isSwingStart; 
    bool                            m_isOnDownSwing; 
    bool                            m_isSwingUpdateReady; 
    float                           m_chunkRate;
    float                           m_hookRate;
    float                           m_meterBar;
    float                           m_skullRate;
    float                           m_sliceRate;
    float                           m_swingImpact;
    float                           m_swingImpactProcessed;
    float                           m_swingIncrement = m_defaultSwingIncrementSpeed;
    float                           m_swingPower;
    const float                     m_bunnyTurtleMax = 20.0;
    const float                     m_defaultSwingIncrementSpeed = 0.9;
    const float                     m_swingPerfectImpactPoint = 0.0;
    const float                     m_swingOverImpact = -40.0;
    const float                     m_swingPowerBunnyMax = 20.0;
    const float                     m_swingPowerTurtleMax = -20.0;
    const float                     m_swingPowerMax = 100.0;
    const float                    m_turnRate = 10.0;

    std::vector<std::string>        m_debugData;

    int                             m_swingCount;
    std::string                     m_scoreString;
};

