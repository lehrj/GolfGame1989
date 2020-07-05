#include "pch.h"
#include "GolfPlay.h"
#include <fstream>

GolfPlay::GolfPlay()
{
    ResetPlayData();
}

GolfPlay::~GolfPlay()
{
}

void GolfPlay::BuildDebugData()
{
    m_debugData.clear();
    m_debugData.push_back("m_meterBar = " + std::to_string(m_meterBar));
    m_debugData.push_back("m_swingPower = " + std::to_string(m_swingPower));
    m_debugData.push_back("m_chunkRate = " + std::to_string(m_chunkRate));
    m_debugData.push_back("m_hookRate = " + std::to_string(m_hookRate));
    m_debugData.push_back("m_swingImpact = " + std::to_string(m_swingImpact));
    m_debugData.push_back("m_skullRate = " + std::to_string(m_skullRate));
    m_debugData.push_back("m_sliceRate = " + std::to_string(m_sliceRate));
}

std::vector<std::string> GolfPlay::GetDebugData() 
{ 
    BuildDebugData();
    return m_debugData; 
};

void GolfPlay::UpdateSwingState()
{
    if (m_isSwingStart == false && m_isSwingPowerSet == false && m_isOnDownSwing == false)
    {
        this->StartSwing();
    }
    //else if (m_isSwingStart == true && m_isSwingPowerSet == false && m_isOnDownSwing == false)
    else if (m_isSwingStart == true && m_isSwingPowerSet == false)
    {
        this->SetPower();
    }
    else if (m_isSwingStart == true && m_isSwingPowerSet == true && m_isOnDownSwing == true)
    {
        this->SetImpact();
    }
    else
    {
        printf("error in GolfPlay::GetSwingState() \n");    
    }
}

void GolfPlay::ResetPlayData()
{
    Utility::ZeroImpactData(m_impactData);
    m_isSwingPowerSet = false;
    m_isSwingStart = false;
    m_isOnDownSwing = false;
    m_isSwingUpdateReady = false;
    m_chunkRate = 0.0;
    m_hookRate = 0.0;
    m_meterBar = 0.0;
    m_swingImpact = 0.0;
    m_swingImpactProcessed = 0.0;
    m_swingIncrement = m_defaultSwingIncrementSpeed;
    m_swingPower = 0.0;
    m_skullRate = 0.0;
    m_sliceRate = 0.0;
}

void GolfPlay::ResetSwingUpdateReady()
{
    m_isSwingUpdateReady = false;
}

void GolfPlay::Swing()
{
    if (m_isSwingStart == true)
    {
        if (m_isOnDownSwing == false)
        {
            m_meterBar += m_swingIncrement;
            if (m_meterBar >= m_swingPowerMax)
            {
                m_isOnDownSwing = true;
            }
        }
        else
        {
            m_meterBar -= m_swingIncrement;
            if (m_isSwingPowerSet == false && m_meterBar <= 0.0)
            {
                ResetPlayData();
            }
            if (m_meterBar <= m_swingOverImpact)
            {
                SetImpact();
            }
        }
    }
}

void GolfPlay::SetImpact()
{
    if (m_isSwingStart == true && m_isOnDownSwing == true)
    {
        m_impactData.impactMissOffSet = m_meterBar;
        m_swingImpactProcessed = m_meterBar;
        m_swingImpact = m_meterBar;
        m_isSwingStart = false;
        m_isSwingUpdateReady = true;
    }
}

void GolfPlay::SetPower()
{
    if (m_isSwingStart == true && m_isOnDownSwing == false)
    {
        m_impactData.power = m_meterBar;
        m_swingPower = m_meterBar;
        m_isOnDownSwing = true;
        m_isSwingPowerSet = true;
    }
}

void GolfPlay::StartSwing()
{
    m_isSwingStart = true;
}

bool GolfPlay::UpdateSwing()
{
    return m_isSwingUpdateReady;
}



