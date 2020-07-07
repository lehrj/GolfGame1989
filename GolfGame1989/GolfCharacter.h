#pragma once

struct Character
{
    std::string name;
    std::string bioLine0;
    std::string bioLine1;
    std::string bioLine2;
    std::string bioLine3;
    double armBalancePoint;
    double armLength;
    double armMass;
    double armMassMoI;    
    double clubLengthModifier;
};

class GolfCharacter
{
public:
    GolfCharacter();

    double GetArmBalancePoint(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armBalancePoint; };
    double GetArmLength(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armLength; };
    double GetArmMass(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMass; };
    double GetArmMassMoI(const int aCharacterIndex) const { return m_characters[aCharacterIndex].armMassMoI; };
    int GetCharacterCount() const { return m_charactersAvailable; };
    double GetClubLenghtModifier(const int aCharacterIndex) const { return m_characters[aCharacterIndex].clubLengthModifier; };
    std::string GetName(const int aCharacterIndex) const { return m_characters[aCharacterIndex].name; };
    std::string GetBioLine0(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine0; };
    std::string GetBioLine1(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine1; };
    std::string GetBioLine2(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine2; };
    std::string GetBioLine3(const int aCharacterIndex) const { return m_characters[aCharacterIndex].bioLine3; };

private:
    void LoadCharacterData();

    std::vector<Character>              m_characters;
    const int                           m_charactersAvailable = 3;
};

