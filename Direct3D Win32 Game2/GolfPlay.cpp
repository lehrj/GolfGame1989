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

void GolfPlay::ResetPlayData()
{
    m_chunkRate = 0.0;
    m_hookRate = 0.0;
    m_swingImpact = 0.0;
    m_swingPower = 0.0;
    m_skullRate = 0.0;
    m_sliceRate = 0.0;
}


