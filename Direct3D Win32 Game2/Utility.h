#pragma once
#include "GolfSwing.h"
#include "Vector4d.h"

class Utility
{
public:
    Utility();
    static inline double GetPi() { return 3.1415926535897931; };
    static inline double ToDegrees(double r) { return r * 180.0 / GetPi(); };
    static inline double ToRadians(double d) { return d / 180.0 * GetPi(); };

    


private:

};

