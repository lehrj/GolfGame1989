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
    
    m_characters[i].name = "Natty Noonan";
    m_characters[i].bioLine0 = "The Natural";
    m_characters[i].bioLine1 = "1";
    m_characters[i].bioLine2 = "2";
    m_characters[i].bioLine3 = "3";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.0;

    /*
    m_characters[i].name = "Natty Noonan";
    m_characters[i].bio = "The Natural";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.82;
    m_characters[i].armMass = 5.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.3;
    */
    ++i;

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
    m_characters[i].bioLine0 = "Big fan of leisure suits";
    m_characters[i].bioLine1 = "but knows that a drink ";
    m_characters[i].bioLine2 = "tossed in his face means no";
    m_characters[i].bioLine3 = "3";
    m_characters[i].armBalancePoint = 0.500069;
    m_characters[i].armLength = 0.620069;
    m_characters[i].armMass = 7.300069;
    m_characters[i].armMassMoI = 1.150069;
    m_characters[i].clubLengthModifier = 1.100069;
    
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
    
    /// ////////////////////////12345678901234567801234567890
    m_characters[i].bioLine0 = "An Italian plumber that ate";
    m_characters[i].bioLine1 = "to many glowing flowers and ";
    m_characters[i].bioLine2 = "then got hit by some kind ";
    m_characters[i].bioLine3 = "of drunk turtle thing";
    m_characters[i].armBalancePoint = 0.500069;
    m_characters[i].armLength = 0.620069;
    m_characters[i].armMass = 7.300069;
    m_characters[i].armMassMoI = 1.150069;
    m_characters[i].clubLengthModifier = .900069;
}

