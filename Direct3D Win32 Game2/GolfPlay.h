#pragma once
class GolfPlay
{
public:
    GolfPlay();
    ~GolfPlay();
    void BuildDebugData();
    std::vector<std::string> GetDebugData();
    void ResetPlayData();

private:
    bool isLefty = false;
    float m_chunkRate;
    float m_hookRate;
    float m_swingImpact;
    float m_swingPower;
    float m_skullRate;
    float m_sliceRate;

    const float m_swingImpactPoint = 0.0;
    const float m_swingOverImpact = -40.0;
    const float m_swingPowerBunnyMax = 20.0;
    const float m_swingPowerTurtleMax = -20.0;
    const float m_swingPowerMax = 100.0;

    std::vector<std::string> m_debugData;
};

