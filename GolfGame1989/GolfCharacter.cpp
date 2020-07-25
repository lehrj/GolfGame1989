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
    
    m_characters[i].name = "Grace Jones";
    m_characters[i].bioLine0 = "If you saw Conan then you ";
    m_characters[i].bioLine1 = "just know she can swing the ";
    m_characters[i].bioLine2 = "big stick better than you";
    m_characters[i].bioLine3 = "and looks better doing it";
    m_characters[i].armBalancePoint = 0.5;
    m_characters[i].armLength = 0.62;
    m_characters[i].armMass = 7.3;
    m_characters[i].armMassMoI = 1.15;
    m_characters[i].clubLengthModifier = 1.0;
    ++i;
    
    m_characters[i].name = "Larry Leisureson";
    m_characters[i].bioLine0 = "Although he is a big fan of";
    m_characters[i].bioLine1 = "leisure suits and flirting";
    m_characters[i].bioLine2 = "he still knows that a drink";
    m_characters[i].bioLine3 = "tossed in his face means no";
    m_characters[i].armBalancePoint = 0.500069;
    m_characters[i].armLength = 0.740069;
    m_characters[i].armMass = 5.200069;
    m_characters[i].armMassMoI = 1.150069;
    m_characters[i].clubLengthModifier = 1.100069;
    ++i;

    m_characters[i].name = "Chubs McShortRound";
    m_characters[i].bioLine0 = "An Italian plumber that ate";
    m_characters[i].bioLine1 = "to many glowing flowers and ";
    m_characters[i].bioLine2 = "then got hit by some kind ";
    m_characters[i].bioLine3 = "of drunk turtle thing";
    m_characters[i].armBalancePoint = 0.500069;
    m_characters[i].armLength = 0.510069;
    m_characters[i].armMass = 8.600069;
    m_characters[i].armMassMoI = 1.150069;
    m_characters[i].clubLengthModifier = .900069;
}

