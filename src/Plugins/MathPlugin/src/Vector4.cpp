/**
 * @file Vector4.cpp
 * @brief Implementation of the Vector4 structure
 */

#include "Vector4.h"
#include "Vector3.h"

Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

Vector4::Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

Vector4::Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

float Vector4::Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Vector4::LengthSquared() const {
    return x * x + y * y + z * z + w * w;
}

Vector4& Vector4::Normalize() {
    float length = Length();
    if (length > 0.0001f) {
        x /= length;
        y /= length;
        z /= length;
        w /= length;
    }
    return *this;
}

Vector4 Vector4::Normalized() const {
    Vector4 result(*this);
    result.Normalize();
    return result;
}

float Vector4::Dot(const Vector4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

Vector3 Vector4::ToVector3() const {
    return Vector3(x, y, z);
}

Vector4& Vector4::operator+=(const Vector4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vector4& Vector4::operator-=(const Vector4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vector4& Vector4::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vector4& Vector4::operator/=(float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
    }
    return *this;
}

Vector4 operator+(const Vector4& a, const Vector4& b) {
    return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vector4 operator-(const Vector4& a, const Vector4& b) {
    return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vector4 operator-(const Vector4& v) {
    return Vector4(-v.x, -v.y, -v.z, -v.w);
}

Vector4 operator*(const Vector4& v, float scalar) {
    return Vector4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

Vector4 operator*(float scalar, const Vector4& v) {
    return Vector4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

Vector4 operator/(const Vector4& v, float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        return Vector4(v.x * inv, v.y * inv, v.z * inv, v.w * inv);
    }
    return v;
}