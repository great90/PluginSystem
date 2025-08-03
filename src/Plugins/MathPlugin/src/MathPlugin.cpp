/**
 * @file MathPlugin.cpp
 * @brief Implementation of the MathPlugin class
 */

#include "MathPlugin.h"
#include "PluginExport.h"

#include <rtm/math.h>
#include <rtm/scalarf.h>
#include <rtm/vector4f.h>
#include <rtm/quatf.h>
#include <rtm/matrix3x3f.h>
#include <rtm/matrix4x4f.h>

#include <random>
#include <ctime>
#include <iostream>

namespace math {

// Initialize static members
MathPlugin* MathPlugin::instance_ = nullptr;

// Define plugin info
PluginInfo MathPlugin::pluginInfo_ = {
    "MathPlugin",                // name
    "Math Utilities Plugin",     // displayName
    "Provides mathematical utilities and types using Realtime Math library", // description
    PluginInfo::Version(1, 0, 0),  // version
    "Plugin System Team"        // author
    // No dependencies
};

MathPlugin::MathPlugin() : rng_(static_cast<unsigned int>(std::time(nullptr))), dist_(0.0f, 1.0f) {
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
    return rtm::scalar_deg_to_rad(degrees);
}

float MathPlugin::RadiansToDegrees(float radians) const {
    return rtm::scalar_rad_to_deg(radians);
}

float MathPlugin::Lerp(float a, float b, float t) const {
    return rtm::scalar_lerp(a, b, t);
}

Vector3 MathPlugin::Lerp(const Vector3& a, const Vector3& b, float t) const {
    return rtm::vector_lerp(a, b, t);
}

Quaternion MathPlugin::Slerp(const Quaternion& a, const Quaternion& b, float t) const {
    return rtm::quat_slerp(a, b, t);
}

float MathPlugin::Clamp(float value, float min, float max) const {
    return rtm::scalar_clamp(value, min, max);
}

float MathPlugin::Random(float min, float max) const {
    return min + dist_(rng_) * (max - min);
}

int MathPlugin::RandomInt(int min, int max) const {
    std::uniform_int_distribution<int> intDist(min, max);
    return intDist(rng_);
}

// Helper functions for Vector2
Vector2 MathPlugin::MakeVector2(float x, float y) const {
    return rtm::vector_set(x, y, 0.0f, 0.0f);
}

float MathPlugin::GetVector2X(const Vector2& v) const {
    return rtm::vector_get_x(v);
}

float MathPlugin::GetVector2Y(const Vector2& v) const {
    return rtm::vector_get_y(v);
}

void MathPlugin::SetVector2X(Vector2& v, float x) const {
    v = rtm::vector_set_x(v, x);
}

void MathPlugin::SetVector2Y(Vector2& v, float y) const {
    v = rtm::vector_set_y(v, y);
}

// Helper functions for Vector3
Vector3 MathPlugin::MakeVector3(float x, float y, float z) const {
    return rtm::vector_set(x, y, z, 0.0f);
}

float MathPlugin::GetVector3X(const Vector3& v) const {
    return rtm::vector_get_x(v);
}

float MathPlugin::GetVector3Y(const Vector3& v) const {
    return rtm::vector_get_y(v);
}

float MathPlugin::GetVector3Z(const Vector3& v) const {
    return rtm::vector_get_z(v);
}

void MathPlugin::SetVector3X(Vector3& v, float x) const {
    v = rtm::vector_set_x(v, x);
}

void MathPlugin::SetVector3Y(Vector3& v, float y) const {
    v = rtm::vector_set_y(v, y);
}

void MathPlugin::SetVector3Z(Vector3& v, float z) const {
    v = rtm::vector_set_z(v, z);
}

// Additional Vector3 operations for Python bindings
Vector3 MathPlugin::CreateVector3(float x, float y, float z) {
    return rtm::vector_set(x, y, z, 0.0f);
}

void MathPlugin::GetVector3Components(const Vector3& v, float& x, float& y, float& z) {
    x = rtm::vector_get_x(v);
    y = rtm::vector_get_y(v);
    z = rtm::vector_get_z(v);
}

Vector3 MathPlugin::Vector3Add(const Vector3& a, const Vector3& b) {
    return rtm::vector_add(a, b);
}

Vector3 MathPlugin::Vector3Subtract(const Vector3& a, const Vector3& b) {
    return rtm::vector_sub(a, b);
}

float MathPlugin::Vector3Dot(const Vector3& a, const Vector3& b) {
    return rtm::vector_dot3(a, b);
}

Vector3 MathPlugin::Vector3Cross(const Vector3& a, const Vector3& b) {
    return rtm::vector_cross3(a, b);
}

float MathPlugin::Vector3Length(const Vector3& v) {
    return rtm::vector_length3(v);
}

Vector3 MathPlugin::Vector3Normalize(const Vector3& v) {
    return rtm::vector_normalize3(v);
}

// Helper functions for Quaternion
Quaternion MathPlugin::MakeQuaternion(float x, float y, float z, float w) const {
    return rtm::quat_set(x, y, z, w);
}

Quaternion MathPlugin::QuaternionFromAxisAngle(const Vector3& axis, float angle) const {
    return rtm::quat_from_axis_angle(axis, angle);
}

Quaternion MathPlugin::QuaternionFromEulerAngles(float pitch, float yaw, float roll) const {
    return rtm::quat_from_euler(pitch, yaw, roll);
}

float MathPlugin::GetQuaternionX(const Quaternion& q) const {
    return rtm::quat_get_x(q);
}

float MathPlugin::GetQuaternionY(const Quaternion& q) const {
    return rtm::quat_get_y(q);
}

float MathPlugin::GetQuaternionZ(const Quaternion& q) const {
    return rtm::quat_get_z(q);
}

float MathPlugin::GetQuaternionW(const Quaternion& q) const {
    return rtm::quat_get_w(q);
}

Vector3 MathPlugin::QuaternionRotateVector(const Quaternion& q, const Vector3& v) const {
    return rtm::quat_mul_vector3(v, q);
}

Quaternion MathPlugin::QuaternionMultiply(const Quaternion& a, const Quaternion& b) const {
    return rtm::quat_mul(a, b);
}

// Helper functions for Matrix4x4
Matrix4x4 MathPlugin::MakeTranslationMatrix(const Vector3& translation) const {
    // Create translation matrix manually
    rtm::vector4f x_axis = rtm::vector_set(1.0f, 0.0f, 0.0f, 0.0f);
    rtm::vector4f y_axis = rtm::vector_set(0.0f, 1.0f, 0.0f, 0.0f);
    rtm::vector4f z_axis = rtm::vector_set(0.0f, 0.0f, 1.0f, 0.0f);
    rtm::vector4f w_axis = rtm::vector_set(rtm::vector_get_x(translation), rtm::vector_get_y(translation), rtm::vector_get_z(translation), 1.0f);
    
    return Matrix4x4{x_axis, y_axis, z_axis, w_axis};
}

Matrix4x4 MathPlugin::MakeScalingMatrix(const Vector3& scale) const {
    // Create scaling matrix manually
    rtm::vector4f x_axis = rtm::vector_set(rtm::vector_get_x(scale), 0.0f, 0.0f, 0.0f);
    rtm::vector4f y_axis = rtm::vector_set(0.0f, rtm::vector_get_y(scale), 0.0f, 0.0f);
    rtm::vector4f z_axis = rtm::vector_set(0.0f, 0.0f, rtm::vector_get_z(scale), 0.0f);
    rtm::vector4f w_axis = rtm::vector_set(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Matrix4x4{x_axis, y_axis, z_axis, w_axis};
}

Matrix4x4 MathPlugin::MakeRotationXMatrix(float angle) const {
    // Create a quaternion for rotation around X axis
    Quaternion rotation = rtm::quat_from_axis_angle(rtm::vector_set(1.0f, 0.0f, 0.0f), angle);
    
    // Convert quaternion to rotation matrix
    float x2 = rtm::quat_get_x(rotation) * rtm::quat_get_x(rotation);
    float y2 = rtm::quat_get_y(rotation) * rtm::quat_get_y(rotation);
    float z2 = rtm::quat_get_z(rotation) * rtm::quat_get_z(rotation);
    float xy = rtm::quat_get_x(rotation) * rtm::quat_get_y(rotation);
    float xz = rtm::quat_get_x(rotation) * rtm::quat_get_z(rotation);
    float yz = rtm::quat_get_y(rotation) * rtm::quat_get_z(rotation);
    float wx = rtm::quat_get_w(rotation) * rtm::quat_get_x(rotation);
    float wy = rtm::quat_get_w(rotation) * rtm::quat_get_y(rotation);
    float wz = rtm::quat_get_w(rotation) * rtm::quat_get_z(rotation);
    
    // Create rotation matrix
    rtm::vector4f x_axis = rtm::vector_set(1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f);
    rtm::vector4f y_axis = rtm::vector_set(2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx), 0.0f);
    rtm::vector4f z_axis = rtm::vector_set(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2), 0.0f);
    rtm::vector4f w_axis = rtm::vector_set(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Matrix4x4{x_axis, y_axis, z_axis, w_axis};
}

Matrix4x4 MathPlugin::MakeRotationYMatrix(float angle) const {
    // Create a quaternion for rotation around Y axis
    Quaternion rotation = rtm::quat_from_axis_angle(rtm::vector_set(0.0f, 1.0f, 0.0f), angle);
    
    // Convert quaternion to rotation matrix
    float x2 = rtm::quat_get_x(rotation) * rtm::quat_get_x(rotation);
    float y2 = rtm::quat_get_y(rotation) * rtm::quat_get_y(rotation);
    float z2 = rtm::quat_get_z(rotation) * rtm::quat_get_z(rotation);
    float xy = rtm::quat_get_x(rotation) * rtm::quat_get_y(rotation);
    float xz = rtm::quat_get_x(rotation) * rtm::quat_get_z(rotation);
    float yz = rtm::quat_get_y(rotation) * rtm::quat_get_z(rotation);
    float wx = rtm::quat_get_w(rotation) * rtm::quat_get_x(rotation);
    float wy = rtm::quat_get_w(rotation) * rtm::quat_get_y(rotation);
    float wz = rtm::quat_get_w(rotation) * rtm::quat_get_z(rotation);
    
    // Create rotation matrix
    rtm::vector4f x_axis = rtm::vector_set(1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f);
    rtm::vector4f y_axis = rtm::vector_set(2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx), 0.0f);
    rtm::vector4f z_axis = rtm::vector_set(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2), 0.0f);
    rtm::vector4f w_axis = rtm::vector_set(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Matrix4x4{x_axis, y_axis, z_axis, w_axis};
}

Matrix4x4 MathPlugin::MakeRotationZMatrix(float angle) const {
    // Create a quaternion for rotation around Z axis
    Quaternion rotation = rtm::quat_from_axis_angle(rtm::vector_set(0.0f, 0.0f, 1.0f), angle);
    
    // Convert quaternion to rotation matrix
    float x2 = rtm::quat_get_x(rotation) * rtm::quat_get_x(rotation);
    float y2 = rtm::quat_get_y(rotation) * rtm::quat_get_y(rotation);
    float z2 = rtm::quat_get_z(rotation) * rtm::quat_get_z(rotation);
    float xy = rtm::quat_get_x(rotation) * rtm::quat_get_y(rotation);
    float xz = rtm::quat_get_x(rotation) * rtm::quat_get_z(rotation);
    float yz = rtm::quat_get_y(rotation) * rtm::quat_get_z(rotation);
    float wx = rtm::quat_get_w(rotation) * rtm::quat_get_x(rotation);
    float wy = rtm::quat_get_w(rotation) * rtm::quat_get_y(rotation);
    float wz = rtm::quat_get_w(rotation) * rtm::quat_get_z(rotation);
    
    // Create rotation matrix
    rtm::vector4f x_axis = rtm::vector_set(1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f);
    rtm::vector4f y_axis = rtm::vector_set(2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx), 0.0f);
    rtm::vector4f z_axis = rtm::vector_set(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2), 0.0f);
    rtm::vector4f w_axis = rtm::vector_set(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Matrix4x4{x_axis, y_axis, z_axis, w_axis};
}

Matrix4x4 MathPlugin::MatrixMultiply(const Matrix4x4& a, const Matrix4x4& b) const {
    return rtm::matrix_mul(a, b);
}

Vector3 MathPlugin::MatrixTransformVector(const Matrix4x4& m, const Vector3& v) const {
    return rtm::matrix_mul_vector(rtm::vector_set(rtm::vector_get_x(v), rtm::vector_get_y(v), rtm::vector_get_z(v), 1.0f), m);
}

// Register the plugin
REGISTER_PLUGIN(math::MathPlugin)

} // namespace math
