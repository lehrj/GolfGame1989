#pragma once
#include <vector>
#include "GolfBag.h"
#include "GolfCharacter.h"
#include "GolfPlay.h"
#include "GolfSwing.h"
#include "GolfBall.h"
#include "Environment.h"
#include "Utility.h"
#include "Vector4d.h"

class Golf
{
public:
    Golf();
    ~Golf();
 
    void BuildUIstrings();
    void BuildEnvironSelectStrings();
  
    std::string GetCharacterArmBalancePoint(const int aCharacterIndex) const;
    std::string GetCharacterArmLength(const int aCharacterIndex) const;
    std::string GetCharacterArmMass(const int aCharacterIndex) const;
    std::string GetCharacterClubLengthMod(const int aCharacterIndex) const;
    std::string GetCharacterArmMassMoI(const int aCharacterIndex) const;
    std::string GetCharacterBioLine0(const int aCharacterIndex) const;
    std::string GetCharacterBioLine1(const int aCharacterIndex) const;
    std::string GetCharacterBioLine2(const int aCharacterIndex) const;
    std::string GetCharacterBioLine3(const int aCharacterIndex) const;
    std::string GetCharacterName(const int aCharacterIndex) const;

    // Future updates will process these from the environment once game moves past just a driving range
    //DirectX::SimpleMath::Vector3 GetCameraPreSwingPos() const { return DirectX::SimpleMath::Vector3(-2.9f, 0.5f, 0.0f); };
    //DirectX::SimpleMath::Vector3 GetCameraTargetPreSwingPos() const { return DirectX::SimpleMath::Vector3(-2.0f, 0.3f, 0.0f); };
    DirectX::SimpleMath::Vector3 GetCameraSwingPos() const { return DirectX::SimpleMath::Vector3(-2.0f, 0.02f, .2f); };
    DirectX::SimpleMath::Vector3 GetCameraTargetSwingPos() const { return DirectX::SimpleMath::Vector3(-2.0f, 0.0f, 0.0f); };

    std::vector<std::vector<std::string>> GetEnvironSelectStrings() const { return m_environSelectStrings; };

    std::vector<DirectX::VertexPositionColor> GetFlagVertex() const { return pEnvironment->GetFlagVertex(); };
    std::vector<DirectX::VertexPositionColor> GetHoleVertex() const { return pEnvironment->GetHoleVertex(); };
    int GetImpactStep() const { return pSwing->GetSwingImpactStep(); };
    double GetArmLength() { return pSwing->GetArmLength(); };
    double GetClubLength() { return pSwing->GetClubLength(); };
    float GetDirectionToHoleInRads() const;
    std::vector<DirectX::SimpleMath::Vector3> GetRawSwingAngles() { return pSwing->GetRawAlphaBetaTheta(); };
    std::vector<DirectX::SimpleMath::Vector3>& GetShotPath() { return m_shotPath; };
    std::vector<float>& GetShotPathTimeSteps() { return pBall->GetShotTimeSteps(); }
    DirectX::SimpleMath::Vector3 GetShotStartPos()const { return m_shotStartPos; };
    int GetSwingStepIncCount() const { return pSwing->GetSwingStepIncrementCount(); };
    DirectX::XMVECTORF32 GetTerrainColor() const { return pEnvironment->GetEnvironColor(); };
    DirectX::SimpleMath::Vector3 GetTeePos() const { return pEnvironment->GetTeePosition(); };
    float GetTeeDirection() const { return pEnvironment->GetTeeDirectionDegrees(); };
    std::vector<std::string> GetUIstrings() { return m_uiStrings; };
    double GetWindDirectionRad() const { return pEnvironment->GetWindDirection(); };

    void InputData();
    void LoadEnvironment(const int aIndex);
    void SelectInputClub(int aInput);

    void CycleNextClub(const bool aIsCycleClubUp);

    void SetCharacter(const int aCharacterIndex);    
    void SetEnvironment(const int aEnvironmentIndex);
    void SetShotStartPos(const DirectX::SimpleMath::Vector3 aShotStartPos);
    void UpdateImpact(Utility::ImpactData aImpact);

    void ZeroUIandRenderData();

private:
    void BuildTrajectoryData();
    void CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath);
    void LoadCharacterTraits();

    void ScaleCordinates();
    void SetShotCordMax();
    void TransformCordinates(const int aIndex);

    Environment*                                pEnvironment;    
    GolfBall*                                   pBall;
    GolfCharacter*                              pCharacter;
    GolfPlay*                                   pPlay;
    GolfSwing*                                  pSwing;
    Utility::ImpactData                         m_impactData;

    DirectX::SimpleMath::Vector3                m_shotStartPos;

    double                                      m_maxX;
    double                                      m_maxY;
    double                                      m_maxZ;
    double                                      m_xWindow;
    double                                      m_yWindow;
    double                                      m_zWindow;

    int                                         m_selectedCharacter;
    int                                         m_selectedEnvironment;
    std::vector<DirectX::SimpleMath::Vector3>   m_shotPath;
    std::vector<std::string>                    m_uiStrings;
    std::vector<std::vector<std::string>>       m_environSelectStrings;
};
