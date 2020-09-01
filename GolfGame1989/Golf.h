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
    
    DirectX::SimpleMath::Vector3 GetBallPosition() const { return m_ballPos; };
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
    DirectX::SimpleMath::Vector3 GetShotStartPos() const { return m_shotStartPos; };
    DirectX::SimpleMath::Vector3 GetSwingOriginOffsetPos() const { return m_swingOriginOffset; };
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

    void SetBallPosition(const DirectX::SimpleMath::Vector3 aBallPos);
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

    DirectX::SimpleMath::Vector3                m_ballPos;
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
    int                                         m_swingStep;
    std::vector<std::string>                    m_uiStrings;
    std::vector<std::vector<std::string>>       m_environSelectStrings;

    const DirectX::SimpleMath::Vector3          m_swingOriginOffset = DirectX::SimpleMath::Vector3(-.0087f, .04f, 0.f);
};
