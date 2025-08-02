/**
 * @file Quaternion.cpp
 * @brief Implementation of the Quaternion structure
 */

#include "Quaternion.h"

Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle) {
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    
    return Quaternion(
        axis.x * s,
        axis.y * s,
        axis.z * s,
        std::cos(halfAngle)
    );
}

Quaternion Quaternion::FromEulerAngles(float pitch, float yaw, float roll) {
    // Convert Euler angles to quaternion using the ZYX convention
    float halfRoll = roll * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfYaw = yaw * 0.5f;
    
    float cr = std::cos(halfRoll);
    float sr = std::sin(halfRoll);
    float cp = std::cos(halfPitch);
    float sp = std::sin(halfPitch);
    float cy = std::cos(halfYaw);
    float sy = std::sin(halfYaw);
    
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    
    return q;
}

float Quaternion::Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

Quaternion& Quaternion::Normalize() {
    float length = Length();
    if (length > 0.0001f) {
        float invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        z *= invLength;
        w *= invLength;
    }
    return *this;
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverse() const {
    float lengthSq = x * x + y * y + z * z + w * w;
    if (lengthSq > 0.0001f) {
        float invLengthSq = 1.0f / lengthSq;
        return Quaternion(-x * invLengthSq, -y * invLengthSq, -z * invLengthSq, w * invLengthSq);
    }
    return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}

Vector3 Quaternion::RotateVector(const Vector3& v) const {
    // Convert vector to quaternion with w=0
    Quaternion vecQuat(v.x, v.y, v.z, 0.0f);
    
    // q * v * q^-1
    Quaternion result = *this * vecQuat * this->Inverse();
    
    return Vector3(result.x, result.y, result.z);
}

Matrix4x4 Quaternion::ToMatrix() const {
    Matrix4x4 result;
    
    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yy = y * y;
    float yz = y * z;
    float yw = y * w;
    float zz = z * z;
    float zw = z * w;
    
    result.m[0][0] = 1.0f - 2.0f * (yy + zz);
    result.m[0][1] = 2.0f * (xy - zw);
    result.m[0][2] = 2.0f * (xz + yw);
    
    result.m[1][0] = 2.0f * (xy + zw);
    result.m[1][1] = 1.0f - 2.0f * (xx + zz);
    result.m[1][2] = 2.0f * (yz - xw);
    
    result.m[2][0] = 2.0f * (xz - yw);
    result.m[2][1] = 2.0f * (yz + xw);
    result.m[2][2] = 1.0f - 2.0f * (xx + yy);
    
    return result;
}