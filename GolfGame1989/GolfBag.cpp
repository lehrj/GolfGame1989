#include "pch.h"
#include "GolfBag.h"
#include <iostream>

GolfBag::GolfBag()
{
    BuildBag();
    SetDiffcultyFactor();
}

void GolfBag::BuildBag()
{
    m_bag.clear();
    m_bag.resize(m_bagSize);
    // Club data modeled off of Ping G400 series driver and woods, G irons, and Karsten TR B60 putter, along with best guesses to fill in data gaps
    int i = 0;

    m_bag[i].clubName = "Driver"; //m_bag[i].clubName = "Custom Debug";
    m_bag[i].angle = 10.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 1.16205;
    m_bag[i].mass = 0.330;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m 
    ++i;
       
    m_bag[i].clubName = "3 Wood";
    m_bag[i].angle = 14.5;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 1.0922;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
    
    /*
    m_bag[i].clubName = "Driver";
    m_bag[i].angle = 25.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 1.1;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
    */

    m_bag[i].clubName = "5 Wood";
    m_bag[i].angle = 17.5;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 1.0668;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "4 Iron";
    m_bag[i].angle = 21.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.987425;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "5 Iron";
    m_bag[i].angle = 24.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.97155;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "6 Iron";
    m_bag[i].angle = 27.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.955675;
    m_bag[i].mass = 0.41;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "7 Iron";
    m_bag[i].angle = 30.5;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.9398;
    m_bag[i].mass = 0.42;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "8 Iron";
    m_bag[i].angle = 35.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.9271;
    m_bag[i].mass = 0.43;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "9 Iron";
    m_bag[i].angle = 38.5;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.9144;
    m_bag[i].mass = 0.44;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "PW";
    m_bag[i].angle = 45.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.9017;
    m_bag[i].mass = 0.45;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
    
    m_bag[i].clubName = "UW";
    m_bag[i].angle = 50.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.9017;
    m_bag[i].mass = 0.45;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
  
    m_bag[i].clubName = "SW";
    m_bag[i].angle = 54.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.89535;
    m_bag[i].mass = 0.45;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;

    m_bag[i].clubName = "LW";
    m_bag[i].angle = 58.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.889;
    m_bag[i].mass = 0.45;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
    
    m_bag[i].clubName = "Putter1";
    m_bag[i].angle = 3.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 0.889;
    //m_bag[i].mass = 0.345;
    m_bag[i].mass = 0.425;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;


    /*
    m_bag[i].clubName = "Default";
    m_bag[i].angle = 25.0;
    m_bag[i].balancePoint = 0.75;
    m_bag[i].coefficiantOfRestitution = 0.78;
    m_bag[i].lengthBase = 1.1;
    m_bag[i].mass = 0.4;
    m_bag[i].massMoI = 0.08;
    m_bag[i].firstMoment = m_bag[i].mass * m_bag[i].lengthBase * m_bag[i].balancePoint;
    ++i;
    */


}

GolfClub GolfBag::GetClub(const int aClubIndex)
{ 
    if (aClubIndex < 0 || aClubIndex > m_bagSize)
    {
        std::cerr << "GolfBag::GetClub input out of range, setting to default club 0\n";
        return m_bag[0];
    }
    else
    {
        return m_bag[aClubIndex];
    }
}

void GolfBag::PrintClubList()
{
    printf("======================================== Club List =========================================\n");
    for (int i = 0; i < m_bagSize; ++i)
    {
        std::cout << " " << i + 1 << ") " << m_bag[i].clubName << std::endl; // Add 1 to index for impoved visual display and so index matches up with iron numbers
    }
    printf("============================================================================================\n");
}

void GolfBag::SetDiffcultyFactor()
{
    for (int i = 0; i < m_bagSize; ++i)
    {
        m_bag[i].difficultyFactor = 1;
        // ToDo: Add equation and balance for multiplier for miss hits based on club angle, club shaft length, and club massZ
    }
}

