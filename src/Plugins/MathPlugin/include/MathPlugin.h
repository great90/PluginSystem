/**
 * @file MathPlugin.h
 * @brief Defines the MathPlugin class that implements the IPlugin interface
 */

#pragma once

#include "IPlugin.h"
#include "PluginInfo.h"
#include "MathPluginExport.h"

// Include Realtime Math (RTM) headers
#include <rtm/types.h>
#include <rtm/vector4f.h>
#include <rtm/quatf.h>
#include <rtm/matrix3x3f.h>
#include <rtm/matrix4x4f.h>

#include <string>
#include <random>

// Define Vector types using RTM
typedef rtm::vector4f Vector2;
typedef rtm::vector4f Vector3;
typedef rtm::vector4f Vector4;
typedef rtm::matrix4x4f Matrix4x4;
typedef rtm::quatf Quaternion;

/**
 * @class MathPlugin
 * @brief Plugin that provides mathematical operations and structures using Realtime Math library
 */
class MATH_PLUGIN_API MathPlugin : public IPlugin {
public:
    /**
     * @brief Constructor
     */
    MathPlugin();
    
    /**
     * @brief Destructor
     */
    virtual ~MathPlugin();
    
    // IPlugin interface implementation
    bool Initialize() override;
    void Shutdown() override;
    const PluginInfo& GetPluginInfo() const override;
    std::string Serialize() override;
    bool Deserialize(const std::string& data) override;
    bool PrepareForHotReload() override;
    bool CompleteHotReload() override;
    
    /**
     * @brief Get the singleton instance of the MathPlugin
     * 
     * @return Pointer to the MathPlugin instance
     */
    static MathPlugin* GetInstance();
    
    /**
     * @brief Convert degrees to radians
     * 
     * @param degrees Angle in degrees
     * @return Angle in radians
     */
    float DegreesToRadians(float degrees) const;
    
    /**
     * @brief Convert radians to degrees
     * 
     * @param radians Angle in radians
     * @return Angle in degrees
     */
    float RadiansToDegrees(float radians) const;
    
    /**
     * @brief Linear interpolation between two values
     * 
     * @param a First value
     * @param b Second value
     * @param t Interpolation factor (0-1)
     * @return Interpolated value
     */
    float Lerp(float a, float b, float t) const;
    
    /**
     * @brief Linear interpolation between two vectors
     * 
     * @param a First vector
     * @param b Second vector
     * @param t Interpolation factor (0-1)
     * @return Interpolated vector
     */
    Vector3 Lerp(const Vector3& a, const Vector3& b, float t) const;
    
    /**
     * @brief Spherical linear interpolation between two quaternions
     * 
     * @param a First quaternion
     * @param b Second quaternion
     * @param t Interpolation factor (0-1)
     * @return Interpolated quaternion
     */
    Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t) const;
    
    /**
     * @brief Clamp a value between a minimum and maximum
     * 
     * @param value Value to clamp
     * @param min Minimum value
     * @param max Maximum value
     * @return Clamped value
     */
    float Clamp(float value, float min, float max) const;
    
    /**
     * @brief Generate a random float between min and max
     * 
     * @param min Minimum value
     * @param max Maximum value
     * @return Random value
     */
    float Random(float min, float max) const;
    
    /**
     * @brief Generate a random integer between min and max (inclusive)
     * 
     * @param min Minimum value
     * @param max Maximum value
     * @return Random integer
     */
    int RandomInt(int min, int max) const;

    // Helper functions for Vector2 (using RTM vector4f)
    Vector2 MakeVector2(float x, float y) const;
    float GetVector2X(const Vector2& v) const;
    float GetVector2Y(const Vector2& v) const;
    void SetVector2X(Vector2& v, float x) const;
    void SetVector2Y(Vector2& v, float y) const;
    
    // Helper functions for Vector3 (using RTM vector4f)
    Vector3 MakeVector3(float x, float y, float z) const;
    float GetVector3X(const Vector3& v) const;
    float GetVector3Y(const Vector3& v) const;
    float GetVector3Z(const Vector3& v) const;
    
    // Aliases for GetVector3X/Y/Z for compatibility with tests
    float GetX(const Vector3& v) const { return GetVector3X(v); }
    float GetY(const Vector3& v) const { return GetVector3Y(v); }
    float GetZ(const Vector3& v) const { return GetVector3Z(v); }
    void SetVector3X(Vector3& v, float x) const;
    void SetVector3Y(Vector3& v, float y) const;
    void SetVector3Z(Vector3& v, float z) const;
    
    // Additional Vector3 operations for Python bindings
    static Vector3 CreateVector3(float x, float y, float z);
    static void GetVector3Components(const Vector3& v, float& x, float& y, float& z);
    static Vector3 Vector3Add(const Vector3& a, const Vector3& b);
    static Vector3 Vector3Subtract(const Vector3& a, const Vector3& b);
    static float Vector3Dot(const Vector3& a, const Vector3& b);
    static Vector3 Vector3Cross(const Vector3& a, const Vector3& b);
    static float Vector3Length(const Vector3& v);
    static Vector3 Vector3Normalize(const Vector3& v);
    
    // Helper functions for Quaternion (using RTM quatf)
    Quaternion MakeQuaternion(float x, float y, float z, float w) const;
    Quaternion QuaternionFromAxisAngle(const Vector3& axis, float angle) const;
    Quaternion QuaternionFromEulerAngles(float pitch, float yaw, float roll) const;
    float GetQuaternionX(const Quaternion& q) const;
    float GetQuaternionY(const Quaternion& q) const;
    float GetQuaternionZ(const Quaternion& q) const;
    float GetQuaternionW(const Quaternion& q) const;
    Vector3 QuaternionRotateVector(const Quaternion& q, const Vector3& v) const;
    Quaternion QuaternionMultiply(const Quaternion& a, const Quaternion& b) const;
    
    // Helper functions for Matrix4x4 (using RTM matrix4x4f)
    Matrix4x4 MakeTranslationMatrix(const Vector3& translation) const;
    Matrix4x4 MakeScalingMatrix(const Vector3& scale) const;
    Matrix4x4 MakeRotationXMatrix(float angle) const;
    Matrix4x4 MakeRotationYMatrix(float angle) const;
    Matrix4x4 MakeRotationZMatrix(float angle) const;
    Matrix4x4 MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b) const;
    Vector3 MatrixTransformVector(const Matrix4x4& m, const Vector3& v) const;

private:
    static MathPlugin* instance_;
    mutable std::mt19937 rng_;
    mutable std::uniform_real_distribution<float> dist_;

public:
    static PluginInfo pluginInfo_;
    
    /**
     * @brief Get static plugin information
     * 
     * @return Static plugin information
     */
    static const PluginInfo& GetPluginStaticInfo() {
        return pluginInfo_;
    }
};