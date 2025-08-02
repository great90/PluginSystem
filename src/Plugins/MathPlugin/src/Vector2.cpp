/**
 * @file Vector2.cpp
 * @brief Implementation of the Vector2 structure
 */

#include "Vector2.h"

Vector2::Vector2() : x(0.0f), y(0.0f) {}

Vector2::Vector2(float x, float y) : x(x), y(y) {}

Vector2::Vector2(const Vector2& other) : x(other.x), y(other.y) {}

float Vector2::Length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2::LengthSquared() const {
    return x * x + y * y;
}

Vector2& Vector2::Normalize() {
    float length = Length();
    if (length > 0.0001f) {
        x /= length;
        y /= length;
    }
    return *this;
}

Vector2 Vector2::Normalized() const {
    Vector2 result(*this);
    result.Normalize();
    return result;
}

float Vector2::Dot(const Vector2& other) const {
    return x * other.x + y * other.y;
}

float Vector2::Cross(const Vector2& other) const {
    return x * other.y - y * other.x;
}

Vector2& Vector2::operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2& Vector2::operator/=(float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        x *= inv;
        y *= inv;
    }
    return *this;
}

Vector2 operator+(const Vector2& a, const Vector2& b) {
    return Vector2(a.x + b.x, a.y + b.y);
}

Vector2 operator-(const Vector2& a, const Vector2& b) {
    return Vector2(a.x - b.x, a.y - b.y);
}

Vector2 operator-(const Vector2& v) {
    return Vector2(-v.x, -v.y);
}

Vector2 operator*(const Vector2& v, float scalar) {
    return Vector2(v.x * scalar, v.y * scalar);
}

Vector2 operator*(float scalar, const Vector2& v) {
    return Vector2(v.x * scalar, v.y * scalar);
}

Vector2 operator/(const Vector2& v, float scalar) {
    if (scalar != 0.0f) {
        float inv = 1.0f / scalar;
        return Vector2(v.x * inv, v.y * inv);
    }
    return v;
}