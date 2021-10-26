#pragma once
#include <string>
#include <vector>

struct GolfClub
{
    std::string clubName;
    float      angle;         // in degrees
    float      balancePoint;  // center of balance along the length of club between 0 and 1
    float      coefficiantOfRestitution; // club face coefficiant of restitution, aka club face spring, current USGA rules limit this to .830 in tournemnt play
    float      firstMoment; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    float      length; // length of club in m
    float      lengthBase; // Unmodified  club length
    float      mass; // mass in kg
    float      massMoI; // Mass moment of inertia of the rod representing the club in kg m^2
    float       difficultyFactor;
};

class GolfBag
{
public:
    GolfBag();

    GolfClub GetClub(const int aClubIndex);
    int GetClubCount() { return m_bagSize; };
    void PrintClubList();
    
private:
    void BuildBag();
    void SetDiffcultyFactor();

    std::vector<GolfClub>               m_bag;
    const int                           m_bagSize = 14;
};

