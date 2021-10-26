#pragma once




// Class to handle miscellaneous functions and data structures needed across multiple classes
class Utility
{
public:
    Utility();
    static inline float GetAngle(DirectX::SimpleMath::Vector3 aRef, DirectX::SimpleMath::Vector3 aPointA, DirectX::SimpleMath::Vector3 aPointB) { return acos(aRef.Dot(aPointA + aPointB)); };
    static inline float GetAngle1(DirectX::SimpleMath::Vector3 aRef, DirectX::SimpleMath::Vector3 aPointA) { return acos(aRef.Dot(aPointA)); };
    static inline int GetNumericalPrecisionForUI() { return 2; }; // For setting the numerical precison displayed by UI
    static inline float GetPi() { return 3.1415926535897931f; };
    static inline float ToDegrees(float r) { return r * 180.0f / GetPi(); };
    static inline float ToRadians(float d) { return d / 180.0f * GetPi(); };

    struct ImpactData
    {   
        float angleX;
        float angleY;
        float angleZ;
        float directionDegrees = 0.0; // shot aim direction
        float directionRads = 0.0; // shot aim direction
        float mass;
        float cor; // Coefficient of restitution
        float impactMissOffSet;
        DirectX::SimpleMath::Vector3 launchPosition;
        float power;
        float velocity;
        DirectX::SimpleMath::Vector3 vHead; 
        DirectX::SimpleMath::Vector3 vHeadNormal;
        DirectX::SimpleMath::Vector3 vHeadParallel;
        DirectX::SimpleMath::Vector3 vFaceNormal;

        // test variables for gameplay
        float swingPlaneAngle;
        float ballAxisTilt;
    };

    static inline void ZeroImpactData(ImpactData& aImpact) 
    {
        aImpact.angleX = 0.0;
        aImpact.angleY = 0.0;
        aImpact.angleZ = 0.0;
        aImpact.directionDegrees = 0.0;
        aImpact.directionRads = 0.0;
        aImpact.mass = 0.0;
        aImpact.impactMissOffSet = 0.0;
        aImpact.launchPosition = DirectX::SimpleMath::Vector3::Zero;
        aImpact.power = 0.0;
        aImpact.cor = 0.0; 
        aImpact.velocity = 0.0;
        aImpact.vFaceNormal = DirectX::SimpleMath::Vector3::Zero;
        aImpact.vHead = DirectX::SimpleMath::Vector3::Zero;
        aImpact.vHeadNormal = DirectX::SimpleMath::Vector3::Zero;
        aImpact.vHeadParallel = DirectX::SimpleMath::Vector3::Zero;

        aImpact.swingPlaneAngle = 0.0;
        aImpact.ballAxisTilt = 0.0;
     };

    template<typename T>
    static T WrapAngle(T theta) noexcept
    {
        const T twoPi = (T)2 * (T)Utility::GetPi();
        const T mod = fmod(theta, twoPi);
        if (mod > (T)Utility::GetPi())
        {
            return mod - twoPi;
        }
        else if (mod < -(T)Utility::GetPi())
        {
            return mod + twoPi;
        }
        return mod;
    }
    
private:

    
};

