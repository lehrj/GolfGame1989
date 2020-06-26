#pragma once
#include <vector>
#include "GolfBag.h"
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
    void CalculateData();

    Vector4d GetLaunchVector();
    std::vector<Vector4d> GetSwingData();
    const int GetSwingStepIncCount() { return pSwing->GetSwingStepIncrementCount(); };

    std::vector<DirectX::SimpleMath::Vector3> GetAlpha() { return pSwing->GetAlphaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetBeta() { return pSwing->GetBetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetTheta() { return pSwing->GetThetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetRawSwingAngles() { return pSwing->GetRawAlphaBetaTheta(); };

    int GetDrawColorIndex();
    std::vector<int> GetDrawColorVector();
    std::vector<std::string> GetUIstrings() { return m_uiStrings; };
    std::vector<DirectX::SimpleMath::Vector3> GetShotPath() { return m_shotPathNorm; };
    const int GetImpactStep() { return pSwing->GetSwingImpactStep(); };
    void InputData();
    void NormalizeData();
    void ScaleCordinates();
    void SelectNextClub();
    void SelectInputClub(int aInput);
    void TransformCordinates();
    void SetShotCordMax();
    void UpdateImpact(Utility::ImpactData aImpact);
    
private:
    Environment* pEnvironment;
    GolfSwing* pSwing;
    GolfBall* pBall;
    GolfPlay* pPlay;
    Utility::ImpactData m_impactData;

    void CopyShotPath(std::vector<DirectX::SimpleMath::Vector3> aPath);
    double m_maxX;
    double m_maxY;
    double m_maxZ;
    double m_xWindow;
    double m_yWindow;
    double m_zWindow;

    std::vector<DirectX::SimpleMath::Vector3> m_shotPathNorm;

    std::vector<DirectX::SimpleMath::Vector3> m_shotPathRaw;
    std::vector<std::vector<DirectX::SimpleMath::Vector3>> m_testShotPathsRaw;
    std::vector<std::vector<DirectX::SimpleMath::Vector3>> m_testShotPathsNorm;
    std::vector<std::string> m_uiStrings;
};
