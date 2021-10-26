#pragma once
#include "Utility.h"

struct Character
{
    std::string name;
    std::string bioLine0;
    std::string bioLine1;
    std::string bioLine2;
    std::string bioLine3;
    float armBalancePoint;
    std::string armBalancePointStr;
    float armLength;
    std::string armLengthStr;
    float armMass;
    std::string armMassStr;
    float armMassMoI;   
    std::string armMassMoIStr;
    float clubLengthModifier;
    std::string clubLengthModifierStr;
};

class GolfCharacter
{
public:
    GolfCharacter();

    float GetArmBalancePoint(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armBalancePoint; };
    std::string GetArmBalancePointString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armBalancePointStr; };
    float GetArmLength(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armLength; };
    std::string GetArmLengthString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armLengthStr; };
    float GetArmMass(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMass; };
    std::string GetArmMassString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassStr; };
    float GetArmMassMoI(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassMoI; };
    std::string GetArmMassMoIString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassMoIStr; };
    int GetCharacterCount() const { return m_charactersAvailable; };
    float GetClubLenghtModifier(const int aCharacterIndex) const { return m_characters[aCharacterIndex].clubLengthModifier; };
    std::string GetClubLenghtModifierString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].clubLengthModifierStr; };
    std::string GetName(const int aCharacterIndex) const { return m_characters[aCharacterIndex].name; };
    std::string GetBioLine0(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine0; };
    std::string GetBioLine1(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine1; };
    std::string GetBioLine2(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine2; };
    std::string GetBioLine3(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine3; };

private:
    void CreateDataStrings();
    void LoadCharacterData();
  
    std::vector<Character>              m_characters;
    const int                           m_charactersAvailable = 3;
};

