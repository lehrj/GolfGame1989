#pragma once
class GolfPlay
{
public:

private:
    bool isLeft = false;
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
};

