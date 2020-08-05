#pragma once

struct Character
{
    std::string name;
    std::string bioLine0;
    std::string bioLine1;
    std::string bioLine2;
    std::string bioLine3;
    double armBalancePoint;
    std::string armBalancePointStr;
    double armLength;
    std::string armLengthStr;
    double armMass;
    std::string armMassStr;
    double armMassMoI;   
    std::string armMassMoIStr;
    double clubLengthModifier;
    std::string clubLengthModifierStr;
};

class GolfCharacter
{
public:
    GolfCharacter();

    double GetArmBalancePoint(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armBalancePoint; };
    std::string GetArmBalancePointString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armBalancePointStr; };
    double GetArmLength(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armLength; };
    std::string GetArmLengthString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armLengthStr; };
    double GetArmMass(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMass; };
    std::string GetArmMassString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassStr; };
    double GetArmMassMoI(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassMoI; };
    std::string GetArmMassMoIString(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassMoIStr; };
    int GetCharacterCount() const { return m_charactersAvailable; };
    double GetClubLenghtModifier(const int aCharacterIndex) const { return m_characters[aCharacterIndex].clubLengthModifier; };
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

