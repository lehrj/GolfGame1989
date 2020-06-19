#pragma once
#include "Utility.h"

class GolfPlay
{
public:
    GolfPlay();
    ~GolfPlay();
    void BuildDebugData();
    void CalculateImpact();
    std::vector<std::string> GetDebugData();
    Utility::ImpactData GetImpactData() { return m_impactData; };
    float GetImpact() { return m_swingImpactProcessed; };
    float GetBackswingSet() { return m_impactData.power; };
    bool GetIsBackswingSet() { return m_isOnDownSwing; };
    float GetMeterPower() { return m_meterBar; };
    float GetSwingPower() { return m_swingPower; };
    float GetMeterLength() { return abs(m_swingPowerMax) + abs(m_swingOverImpact); };
    float GetMeterImpactPoint() { return GetMeterLength() - m_swingPowerMax; };
    void ResetPlayData();
    void ResetSwingUpdateReady();
    void Swing();
    void StartSwing();
    void SetImpact();
    void SetPower();
    
    bool UpdateSwing();
    void UpdateSwingState();

    

private:
    Utility::ImpactData m_impactData;

    bool m_isLefty = false;
    bool m_isSwingPowerSet;
    bool m_isSwingStart; 
    bool m_isOnDownSwing; 
    bool m_isSwingUpdateReady; 

    float m_chunkRate;
    float m_hookRate;
    float m_meterBar;
    float m_swingImpact;
    float m_swingImpactProcessed;
    float m_swingIncrement = m_defaultSwingIncrementSpeed;
    float m_swingPower;
    float m_skullRate;
    float m_sliceRate;

    const float m_defaultSwingIncrementSpeed = 0.9;
    const float m_swingPerfectImpactPoint = 0.0;
    const float m_swingOverImpact = -40.0;
    const float m_swingPowerBunnyMax = 20.0;
    const float m_swingPowerTurtleMax = -20.0;
    const float m_bunnyTurtleMax = 20.0;
    const float m_swingPowerMax = 100.0;

    std::vector<std::string> m_debugData;
};

