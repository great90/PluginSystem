/**
 * @file MathPlugin.h
 * @brief Defines the MathPlugin class that implements the IPlugin interface
 */

#pragma once

#include "IPlugin.h"
#include "PluginInfo.h"
#include "MathPluginExport.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

#include <string>
#include <random>

/**
 * @class MathPlugin
 * @brief Plugin that provides mathematical operations and structures
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

private:
    static MathPlugin* instance_;

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