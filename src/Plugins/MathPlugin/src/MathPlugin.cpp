/**
 * @file MathPlugin.cpp
 * @brief Implementation of the MathPlugin class
 */

#include "MathPlugin.h"
#include "PluginExport.h"
#include <random>
#include <ctime>
#include <iostream>

// Initialize static members
MathPlugin* MathPlugin::instance_ = nullptr;

// Define plugin info
PluginInfo MathPlugin::pluginInfo_ = {
    "MathPlugin",                // name
    "Math Utilities Plugin",     // displayName
    "Provides mathematical utilities and types", // description
    PluginInfo::Version(1, 0, 0),  // version
    "Plugin System Team"        // author
    // No dependencies
};

// Random number generator
static std::mt19937 rng;
static std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);

MathPlugin::MathPlugin() {
    // Set the singleton instance
    if (instance_ == nullptr) {
        instance_ = this;
    }
}

MathPlugin::~MathPlugin() {
    // Clear the singleton instance if it's this instance
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool MathPlugin::Initialize() {
    // Seed the random number generator
    rng.seed(static_cast<unsigned int>(std::time(nullptr)));
    
    std::cout << "MathPlugin initialized successfully" << std::endl;
    return true;
}

void MathPlugin::Shutdown() {
    std::cout << "MathPlugin shut down" << std::endl;
}

const PluginInfo& MathPlugin::GetPluginInfo() const {
    return pluginInfo_;
}

std::string MathPlugin::Serialize() {
    // For now, we don't have any state to serialize
    return "";
}

bool MathPlugin::Deserialize(const std::string& data) {
    // For now, we don't have any state to deserialize
    return true;
}

bool MathPlugin::PrepareForHotReload() {
    // Nothing special needed for hot reload preparation
    return true;
}

bool MathPlugin::CompleteHotReload() {
    // Nothing special needed for hot reload completion
    return true;
}

MathPlugin* MathPlugin::GetInstance() {
    return instance_;
}

float MathPlugin::DegreesToRadians(float degrees) const {
    return degrees * 0.01745329251994329576923690768489f; // PI / 180
}

float MathPlugin::RadiansToDegrees(float radians) const {
    return radians * 57.295779513082320876798154814105f; // 180 / PI
}

float MathPlugin::Lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

Vector3 MathPlugin::Lerp(const Vector3& a, const Vector3& b, float t) const {
    return Vector3(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y),
        a.z + t * (b.z - a.z)
    );
}

Quaternion MathPlugin::Slerp(const Quaternion& a, const Quaternion& b, float t) const {
    // Ensure quaternions are normalized
    Quaternion q1 = a;
    Quaternion q2 = b;
    q1.Normalize();
    q2.Normalize();
    
    // Calculate the dot product
    float dot = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
    
    // If the dot product is negative, negate one of the quaternions
    // to take the shorter path
    if (dot < 0.0f) {
        q2.x = -q2.x;
        q2.y = -q2.y;
        q2.z = -q2.z;
        q2.w = -q2.w;
        dot = -dot;
    }
    
    // Clamp dot to valid range
    dot = Clamp(dot, -1.0f, 1.0f);
    
    // Calculate the angle between the quaternions
    float angle = std::acos(dot);
    
    // If the angle is very small, use linear interpolation
    if (angle < 0.0001f) {
        return Quaternion(
            q1.x + t * (q2.x - q1.x),
            q1.y + t * (q2.y - q1.y),
            q1.z + t * (q2.z - q1.z),
            q1.w + t * (q2.w - q1.w)
        );
    }
    
    // Calculate the interpolation factors
    float sinAngle = std::sin(angle);
    float invSinAngle = 1.0f / sinAngle;
    float t1 = std::sin((1.0f - t) * angle) * invSinAngle;
    float t2 = std::sin(t * angle) * invSinAngle;
    
    // Perform the interpolation
    Quaternion result(
        q1.x * t1 + q2.x * t2,
        q1.y * t1 + q2.y * t2,
        q1.z * t1 + q2.z * t2,
        q1.w * t1 + q2.w * t2
    );
    
    // Normalize the result
    result.Normalize();
    
    return result;
}

float MathPlugin::Clamp(float value, float min, float max) const {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float MathPlugin::Random(float min, float max) const {
    return min + floatDist(rng) * (max - min);
}

int MathPlugin::RandomInt(int min, int max) const {
    std::uniform_int_distribution<int> intDist(min, max);
    return intDist(rng);
}

// Register the plugin
REGISTER_PLUGIN(MathPlugin)
