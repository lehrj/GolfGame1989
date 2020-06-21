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
    void BuildUIdata();
    void BuildUIstrings();
    void BuildUI();
    void CalculateData();

    Vector4d GetLaunchVector();
    std::vector<Vector4d> GetSwingData();
    const int GetSwingStepIncCount() { return pSwing->GetSwingStepIncrementCount(); };

    std::vector<DirectX::SimpleMath::Vector3> GetAlpha() { return pSwing->GetAlphaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetBeta() { return pSwing->GetBetaCords(); };
    std::vector<DirectX::SimpleMath::Vector3> GetTheta() { return pSwing->GetThetaCords(); };
    int GetDrawColorIndex();
    std::vector<int> GetDrawColorVector();
    //std::vector<double> GetUIdata() { return m_uiData; };
    std::vector<std::string> GetUIstrings() { return m_uiStrings; };
    std::vector<double> GetVect(const int aInput);
    void InputData();
    void NormalizeData();
    void ScaleCordinates();
    void SelectNextClub();
    void SelectInputClub(int aInput);
    void TransformCordinates();
    void SetMaxX();
    void SetMaxY();
    void SetMaxZ();
    void UpdateImpact(Utility::ImpactData aImpact);

private:
    Environment* pEnvironment;
    GolfSwing* pSwing;
    GolfBall* pBall;
    GolfPlay* pPlay;
    Utility::ImpactData m_impactData;
    void CopyXvec(std::vector<double> aVec);
    void CopyYvec(std::vector<double> aVec);
    void CopyZvec(std::vector<double> aVec);
    double m_maxX;
    double m_maxY;
    double m_maxZ;
    double m_xWindow;
    double m_yWindow;
    double m_zWindow;

    std::vector<double> m_xNorm;
    std::vector<double> m_yNorm;
    std::vector<double> m_zNorm;

    std::vector<double> m_xVals;
    std::vector<double> m_yVals;
    std::vector<double> m_zVals;

    //std::vector<double> m_uiData;
    std::vector<std::string> m_uiStrings;

};
