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

    void BuildVector();
    void BuildUIstrings();

    const int GetSwingStepIncCount() { return pSwing->GetSwingStepIncrementCount(); };

    std::vector<DirectX::SimpleMath::Vector3> GetAlpha() { return pSwing->GetAlphaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetBeta() { return pSwing->GetBetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetTheta() { return pSwing->GetThetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetRawSwingAngles() { return pSwing->GetRawAlphaBetaTheta(); };

    std::string GetCharacterName(const int aCharacterIndex) const;
    std::string GetCharacterBio(const int aCharacterIndex) const;
    int GetDrawColorIndex();
    std::vector<int> GetDrawColorVector();
    std::vector<std::string> GetUIstrings() { return m_uiStrings; };
    std::vector<DirectX::SimpleMath::Vector3>& GetShotPath() { return m_shotPath; };
    std::vector<float>& GetShotPathTimeSteps() { return pBall->GetShotTimeSteps(); };
    const int GetImpactStep() { return pSwing->GetSwingImpactStep(); };
    
    void InputData();
    void ScaleCordinates();
    void SelectNextClub();
    void SelectInputClub(int aInput);
    void SetCharacter(const int aCharacterIndex);    
    void SetShotCordMax();
    void TransformCordinates(const int aIndex);
    void UpdateImpact(Utility::ImpactData aImpact);
    
private:
    Environment* pEnvironment;    
    GolfBall* pBall;
    GolfCharacter* pCharacter;
    GolfPlay* pPlay;
    GolfSwing* pSwing;
    Utility::ImpactData m_impactData;

    void CopyShotPath(std::vector<DirectX::SimpleMath::Vector3>& aPath);
    void LoadCharacterTraits();

    double m_maxX;
    double m_maxY;
    double m_maxZ;
    double m_xWindow;
    double m_yWindow;
    double m_zWindow;

    int m_selectedCharacter;
    std::vector<DirectX::SimpleMath::Vector3> m_shotPath;
    std::vector<std::string> m_uiStrings;
};
