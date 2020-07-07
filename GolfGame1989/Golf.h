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

    int GetSwingStepIncCount() const { return pSwing->GetSwingStepIncrementCount(); };

    std::vector<DirectX::SimpleMath::Vector3> GetAlpha() { return pSwing->GetAlphaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetBeta() { return pSwing->GetBetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetTheta() { return pSwing->GetThetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetRawSwingAngles() { return pSwing->GetRawAlphaBetaTheta(); };
    
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

    int GetDrawColorIndex();
    std::vector<int> GetDrawColorVector();
    std::vector<std::string> GetUIstrings() { return m_uiStrings; };
    std::vector<DirectX::SimpleMath::Vector3>& GetShotPath() { return m_shotPath; };
    std::vector<float>& GetShotPathTimeSteps() { return pBall->GetShotTimeSteps(); };
    const int GetImpactStep() { return pSwing->GetSwingImpactStep(); };
    double GetArmLength() { return pSwing->GetArmLength(); };
    double GetClubLength() { return pSwing->GetClubLength(); };
    void InputData();

    void SelectInputClub(int aInput);
    void SetCharacter(const int aCharacterIndex);    
 
    void UpdateImpact(Utility::ImpactData aImpact);
    
private:
    Environment*                    pEnvironment;    
    GolfBall*                       pBall;
    GolfCharacter*                  pCharacter;
    GolfPlay*                       pPlay;
    GolfSwing*                      pSwing;
    Utility::ImpactData             m_impactData;

    void BuildVector();
    void CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath);
    void LoadCharacterTraits();
    void ScaleCordinates();
    void SetShotCordMax();
    void TransformCordinates(const int aIndex);

    double                          m_maxX;
    double                          m_maxY;
    double                          m_maxZ;
    double                          m_xWindow;
    double                          m_yWindow;
    double                          m_zWindow;

    int                             m_selectedCharacter;
    std::vector<DirectX::SimpleMath::Vector3> m_shotPath;
    std::vector<std::string>        m_uiStrings;
};
