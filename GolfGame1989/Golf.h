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

// Class to manage interactions between game and golf physics engine
class Golf
{
public:
    Golf();
    ~Golf();
    
    void BuildEnvironSelectStrings();
    void BuildHyrdraShotData(const double aDirectionDeg);
    void BuildUIShotStrings();
    void BuildUISwingStrings();
    void CycleNextClub(const bool aIsCycleClubUp);

    std::vector<std::pair< DirectX::VertexPositionColor, DirectX::VertexPositionColor>> GetBallDebugLines() const { return pBall->GetDebugLines(); };
    double GetBallPlacementAngleForSwing() const { return pSwing->GetBallPlacementAngle(); };
    DirectX::SimpleMath::Vector3 GetBallPosition() const { return m_ballPos; };
    double GetBallRadius() { return pBall->GetBallRadius(); };
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
    float GetEnvironScale() const { return pEnvironment->GetScale(); };
    std::vector<std::vector<std::string>> GetEnvironSelectStrings() const { return m_environSelectStrings; };
    DirectX::SimpleMath::Vector3 GetEnvironWindVector() const { return pEnvironment->GetWindVector(); };
    DirectX::SimpleMath::Vector3 GetFaceImpact() const { return pSwing->GetFaceNormal(); };
    std::vector<DirectX::VertexPositionColor> GetFlagVertex() const { return pEnvironment->GetFlagVertex(); };
    std::vector<DirectX::VertexPositionColor> GetHoleVertex() const { return pEnvironment->GetHoleVertex(); };
    std::vector<std::vector<BallMotion>>& GetHydraShotPath() { return m_hydraData; };
    bool GetIsBallInHole() const { return pBall->GetIsInHole(); };
    int GetImpactStep() const { return pSwing->GetSwingImpactStep(); };
    double GetArmLength() { return pSwing->GetArmLength(); };
    double GetClubLength() { return pSwing->GetClubLength(); };
    double GetClubFaceAngle() { return pSwing->GetClubAngle(); };
    float GetDirectionToHoleInRads() const;   
    std::vector<Fixture> GetEnvironFixtureBucket() { return pEnvironment->GetFixtureBucket(); };
    DirectX::SimpleMath::Vector3 GetHolePosition() const { return pEnvironment->GetHolePosition(); };
    int GetParFromEnviron() const { return pEnvironment->GetPar(); };
    std::vector<DirectX::SimpleMath::Vector3> GetRawSwingAngles() { return pSwing->GetRawAlphaBetaTheta(); };
    float GetShotDistance() const;
    std::string GetShotDistanceString() const;
    std::vector<BallMotion>& GetShotPath() { return m_shotPath; };
    DirectX::SimpleMath::Vector3 GetShotStartPos() const { return m_shotStartPos; };
    DirectX::SimpleMath::Vector3 GetSwingShoulderOrigin();
    DirectX::SimpleMath::Vector3 GetSwingOriginOffsetPos() const { return m_swingOriginOffset; };
    int GetSwingStepIncCount() const { return pSwing->GetSwingStepIncrementCount(); };
    DirectX::XMVECTORF32 GetTerrainColor() const { return pEnvironment->GetEnvironColor(); };
    DirectX::SimpleMath::Vector3 GetTeePos() const { return pEnvironment->GetTeePosition(); };
    float GetTeeDirection() const { return pEnvironment->GetTeeDirectionDegrees(); };
    std::vector<std::string> GetUIShotStrings() const { return m_uiShotStrings; };
    std::vector<std::string> GetUIStrings() const { return m_uiStrings; };
    std::vector<std::string> GetUISwingStrings() const { return m_uiSwingStrings; };
    double GetWindDirectionRad() const { return pEnvironment->GetWindDirection(); };
    

    void InputData();
    void LoadEnvironment(const int aIndex);
    void ResetBallData() { pBall->ResetBallData(); };
    void ResetIsBallInHole() { pBall->ResetIsInHole(); };
    void SelectInputClub(int aInput);

    void SetBallPosition(const DirectX::SimpleMath::Vector3 aBallPos);
    void SetCharacter(const int aCharacterIndex);    
    void SetEnvironment(const int aEnvironmentIndex);
    void SetShotStartPos(const DirectX::SimpleMath::Vector3 aShotStartPos);
    
    void UpdateEnvironmentSortingForDraw(DirectX::SimpleMath::Vector3 aCameraPos);
    void UpdateImpact(Utility::ImpactData aImpact);
    void ZeroUIandRenderData();

private:
    void BuildTrajectoryData();
    void CopyShotPath(std::vector<BallMotion>& aPath);
    void LoadCharacterTraits();
    void ScaleCordinates();
    void SetShotCordMax();
    void TransformCordinates(const int aIndex);

    Environment*                                pEnvironment;    
    GolfBall*                                   pBall;
    GolfCharacter*                              pCharacter;
    GolfSwing*                                  pSwing;

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
    std::vector<BallMotion>                     m_shotPath;
    int                                         m_swingStep;

    std::vector<std::string>                    m_uiStrings;
    std::vector<std::string>                    m_uiShotStrings;
    std::vector<std::string>                    m_uiSwingStrings;

    std::vector<std::vector<std::string>>       m_environSelectStrings;

    const DirectX::SimpleMath::Vector3          m_swingOriginOffset = DirectX::SimpleMath::Vector3(-.0087f, .04f, 0.f);
    
    std::vector<std::vector<BallMotion>>        m_hydraData;

public:
    // height map testing
    bool InitializeHeightMap(ID3D11Device*, char*);
};
