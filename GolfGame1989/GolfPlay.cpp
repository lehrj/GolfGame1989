#include "pch.h"
#include "GolfPlay.h"
#include <fstream>

GolfPlay::GolfPlay()
{
    ResetPlayData();
    ResetSwingCount();
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

void GolfPlay::CalculateScoreString(const int aPar)
{
    const int holeScore = m_swingCount - aPar;
    if (m_swingCount <= 0)
    {
        // add error logging 
    }
    else if (m_swingCount == 1)
    {
        m_scoreString = "Ace!!!";
    }
    else if (holeScore == -3)
    {
        m_scoreString = "Albatross!!";
    }
    else if (holeScore == -2)
    {
        m_scoreString = "Eagle!";
    }
    else if (holeScore == -1)
    {
        m_scoreString = "Birdie!";
    }
    else if (holeScore == 0)
    {
        m_scoreString = "Par";
    }
    else if (holeScore == 1)
    {
        m_scoreString = "Boogie";
    }
    else if (holeScore == 2)
    {
        m_scoreString = "Double Boogie";
    }
    else
    {
        m_scoreString = "Score is : " + std::to_string(holeScore);
    }
}

// set shot power to 100% and impact to perfect for debuging
void GolfPlay::DebugShot()
{
    m_impactData.power = 100.0;
    m_swingPower = 100.0;
    m_isOnDownSwing = true;
    m_isSwingPowerSet = true;

    m_impactData.impactMissOffSet = 0.0;
    m_swingImpactProcessed = 0.0;
    m_swingImpact = 0.0;
    m_isSwingStart = false;
    m_isSwingUpdateReady = true;
}

std::vector<std::string> GolfPlay::GetDebugData()
{ 
    BuildDebugData();
    return m_debugData; 
};

void GolfPlay::IncrementSwingCount()
{
    ++m_swingCount;
}

bool GolfPlay::IsSwingStateAtImpact() const
{
    if (m_isSwingStart == true && m_isSwingPowerSet == true && m_isOnDownSwing == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GolfPlay::UpdateSwingState()
{
    if (m_isSwingStart == false && m_isSwingPowerSet == false && m_isOnDownSwing == false)
    {
        this->StartSwing();
    }
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

void GolfPlay::ResetSwingCount()
{
    m_swingCount = 0;
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
    if (m_isSwingStart == true)
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

void GolfPlay::TurnShotAim(double aTurn, float aTurnRate)
{
    double turnInRadians = Utility::ToRadians(aTurn) * (aTurnRate * 57.2958);
    m_impactData.directionDegrees += Utility::ToDegrees(turnInRadians);
    
    // wrap angle
    if (m_impactData.directionDegrees > 180.0)
    {
        m_impactData.directionDegrees -= 360.0;
    }
    else if (m_impactData.directionDegrees < -180.0)
    {
        m_impactData.directionDegrees += 360.0;
    } 
}

bool GolfPlay::UpdateSwing()
{
    return m_isSwingUpdateReady;
}



