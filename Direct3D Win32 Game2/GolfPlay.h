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
    float GetSwingPower() { return m_swingPower; };
    
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
    float m_swingIncrement = 5.0;
    float m_swingPower;
    float m_skullRate;
    float m_sliceRate;

    const float m_swingPerfectImpactPoint = 0.0;
    const float m_swingOverImpact = -40.0;
    const float m_swingPowerBunnyMax = 20.0;
    const float m_swingPowerTurtleMax = -20.0;
    const float m_bunnyTurtleMax = 20.0;
    const float m_swingPowerMax = 100.0;

    std::vector<std::string> m_debugData;
};

