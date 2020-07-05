#pragma once

#include <string>
#include <vector>

struct GolfClub
{
    std::string clubName;
    double angle;         // in degrees
    double balancePoint;  // center of balance along the length of club between 0 and 1
    double coefficiantOfRestitution; // club face coefficiant of restitution, aka club face spring, current USGA rules limit this to .830 in tournemnt play
    double firstMoment; // First moment of the rod representing the club about the wrist axis (where the club rod connects to the arm rod) in kg m
    double length; // length of club in m
    double lengthBase; // Unmodified  club length
    double mass; // mass in kg
    double massMoI; // Mass moment of inertia of the rod representing the club in kg m^2
    float difficultyFactor;
};

class GolfBag
{
public:
    GolfBag();
    ~GolfBag();

    void BuildBag();
    GolfClub GetClub(const int aClubIndex);
    int GetClubCount() { return m_bagSize; };
    void PrintClubList();
    void SetDiffcultyFactor();
    
private:
    std::vector<GolfClub> m_bag;
    const int m_bagSize = 14;
};

