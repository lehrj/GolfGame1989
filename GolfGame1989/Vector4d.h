#pragma once

// Class to handle vector needs prior to DirectX implementation, plan to remove after all functionality and data moved over to DirectX interface
class Vector4d
{
public:
    Vector4d();
    Vector4d(double aX, double aY, double aZ, double aW);
    ~Vector4d();

    // to avoid confusion while still bastardizing Vector4d class for prototyping implementation
    const double GetFirst() { return x; };
    const double GetSecond() { return y; };
    const double GetThird() { return z; };
    const double GetForth() { return w; };

    const double GetX() { return x; };
    const double GetY() { return y; };
    const double GetZ() { return z; };
    const double GetW() { return w; };
    Vector4d GetVector4d();
    double Magnitude3();

    Vector4d NormalizeVector(const Vector4d& aVec);

    void PrintVector();
    void SetAll(const double& aX, const double& aY, const double& aZ, const double& aW);
    void SetX(const double& aX);
    void SetY(const double& aY);
    void SetZ(const double& aZ);
    void SetW(const double& aW);

private:
    double x;
    double y;
    double z;
    double w;
};