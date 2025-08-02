/**
 * @file Vector3.cpp
 * @brief Implementation of the Vector3 structure
 */

#include "Vector3.h"

Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

Vector3::Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}

float Vector3::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3::LengthSquared() const {
    return x * x + y * y + z * z;
}

Vector3& Vector3::Normalize() {
    float length = Length();
    if (length > 0.0001f) {
        x /= length;
        y /= length;
        z /= length;
    }
    return *this;
}

Vector3 Vector3::Normalized() const {
    Vector3 result(*this);
    result.Normalize();
    return result;
}

float Vector3::Dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::Cross(const Vector3& other) const {
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

Vector3& Vector3::operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
        z *= inv;
    }
    return *this;
}

Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 operator-(const Vector3& v) {
    return Vector3(-v.x, -v.y, -v.z);
}

Vector3 operator*(const Vector3& v, float scalar) {
    return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3 operator*(float scalar, const Vector3& v) {
    return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3 operator/(const Vector3& v, float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        return Vector3(v.x * inv, v.y * inv, v.z * inv);
    }
    return v;
}