#pragma once
//#include "GolfSwing.h"
#include "Vector4d.h"

class Utility
{
public:
    Utility();
    static inline double GetPi() { return 3.1415926535897931; };
    static inline double ToDegrees(double r) { return r * 180.0 / GetPi(); };
    static inline double ToRadians(double d) { return d / 180.0 * GetPi(); };


    struct ImpactData
    {
        double velocity;
        double angleX;
        double angleY;
        double angleZ;
        double mass;
        double cor;
        double impactMissOffSet;
        double power;
    };

    static inline void ZeroImpactData(ImpactData& aImpact) {
        aImpact.velocity = 0.0;
        aImpact.angleX = 0.0;
        aImpact.angleY = 0.0;
        aImpact.angleZ = 0.0;
        aImpact.mass = 0.0;
        aImpact.impactMissOffSet = 0.0;
        aImpact.power = 0.0;
        aImpact.cor = 0.0; };
    


private:

};

