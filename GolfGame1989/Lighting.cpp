#include "pch.h"
#include "Lighting.h"


Lighting::Lighting()
{

}

Lighting::LightingState Lighting::GetLightingState()
{
    return m_currentLightingState;
}

void Lighting::SetLighting(LightingState aLightState)
{
    m_currentLightingState = aLightState;
}

void Lighting::UpdateLighting(std::shared_ptr<DirectX::NormalMapEffect> aEffect, const double aTimer)
{
    // placeholder 
    aEffect->EnableDefaultLighting();

}