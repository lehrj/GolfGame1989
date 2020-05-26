#include "pch.h"

#include "Vector4d.h"
#include <vector>

Vector4d::Vector4d()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
    , w(0.0f) {}

Vector4d::Vector4d(double aX, double aY, double aZ, double aW)
{
    // initialization list
    this->x = aX;
    this->y = aY;
    this->z = aZ;
    this->w = aW;
}

Vector4d::~Vector4d()
{
}

Vector4d Vector4d::GetVector4d()
{
    Vector4d aVector;
    aVector.SetX(this->GetX());
    aVector.SetY(this->GetY());
    aVector.SetZ(this->GetZ());
    aVector.SetW(this->GetW());
    return aVector;
}

double Vector4d::Magnitude3()
{
    return(sqrt(this->x * this->x + this->y * this->y + this->z * this->z));
}

Vector4d Vector4d::NormalizeVector(const Vector4d& aVec)
{
    double const c_zeroTolerance = 0.0001f; // adjust as needed for 0 tolerance
    Vector4d normalVector = aVec;

    double m = sqrt((aVec.x * aVec.x) + (aVec.y * aVec.y) + (aVec.z * aVec.z));

    if (m <= c_zeroTolerance)
    {
        m = 1;
    }
    normalVector.x /= m;
    normalVector.y /= m;
    normalVector.z /= m;

    if (fabs(normalVector.x) < c_zeroTolerance)
    {
        normalVector.x = 0.0f;
    }
    if (fabs(normalVector.y) < c_zeroTolerance)
    {
        normalVector.y = 0.0f;
    }
    if (fabs(normalVector.z) < c_zeroTolerance)
    {
        normalVector.z = 0.0f;
    }

    normalVector.w = 1;

    return normalVector;
}

void Vector4d::PrintVector()
{
    printf("x = %f, y = %f, z = %f, w = %f \n", this->GetX(), this->GetY(), this->GetZ(), this->GetW());
}

void Vector4d::SetAll(const double& aX, const double& aY, const double& aZ, const double& aW)
{
    this->x = aX;
    this->y = aY;
    this->z = aZ;
    this->w = aW;
}

void Vector4d::SetX(const double& aX)
{
    this->x = aX;
}

void Vector4d::SetY(const double& aY)
{
    this->y = aY;
}

void Vector4d::SetZ(const double& aZ)
{
    this->z = aZ;
}

void Vector4d::SetW(const double& aW)
{
    this->w = aW;
}


