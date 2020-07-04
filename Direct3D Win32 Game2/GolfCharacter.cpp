#include "pch.h"
#include "GolfCharacter.h"

GolfCharacter::GolfCharacter()
{
    LoadCharacterData();
}

void GolfCharacter::LoadCharacterData()
{
    m_characters.clear();
    m_characters.resize(m_charactersAvailable);

    int i = 0;
    /*
    m_characters[i].name = "Larry Leisureson";
    m_characters[i].bio = "Fan of Leisure suits and knows that a drink tossed in his face means no";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.92;
    m_characters[i].armMass = 10.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.0;
    */
    
    m_characters[i].name = "Larry Leisureson";
    m_characters[i].bio = "Fan of Leisure suits and knows that a drink tossed in his face means no";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 2.0;
    
    ++i;
    /*
    m_characters[i].name = "Natty Noonan";
    m_characters[i].bio = "The Natural";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.0;
    */
    m_characters[i].name = "Natty Noonan";
    m_characters[i].bio = "The Natural";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.82;
    m_characters[i].armMass = 5.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.3;
    ++i;
    /*
    m_characters[i].name = "Chubs McShortRound";
    m_characters[i].bio = "Mario ate the skinny one and then got hit by a turtle thing";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.0;
    */
    m_characters[i].name = "Chubs McShortRound";
    m_characters[i].bio = "Mario ate the skinny one and then got hit by a turtle thing";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = .5;
}

