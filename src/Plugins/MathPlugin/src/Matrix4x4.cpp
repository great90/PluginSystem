/**
 * @file Matrix4x4.cpp
 * @brief Implementation of the Matrix4x4 structure
 */

#include "Matrix4x4.h"

Matrix4x4::Matrix4x4() {
    Identity();
}

void Matrix4x4::Identity() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

Matrix4x4 Matrix4x4::Translation(float x, float y, float z) {
    Matrix4x4 result;
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    return result;
}

Matrix4x4 Matrix4x4::Translation(const Vector3& v) {
    return Translation(v.x, v.y, v.z);
}

Matrix4x4 Matrix4x4::Scaling(float x, float y, float z) {
    Matrix4x4 result;
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    return result;
}

Matrix4x4 Matrix4x4::Scaling(const Vector3& v) {
    return Scaling(v.x, v.y, v.z);
}

Matrix4x4 Matrix4x4::RotationX(float angle) {
    Matrix4x4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    return result;
}

Matrix4x4 Matrix4x4::RotationY(float angle) {
    Matrix4x4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    return result;
}

Matrix4x4 Matrix4x4::RotationZ(float angle) {
    Matrix4x4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

Vector3 Matrix4x4::TransformVector(const Vector3& v) const {
    float x = v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + m[0][3];
    float y = v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + m[1][3];
    float z = v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + m[2][3];
    float w = v.x * m[3][0] + v.y * m[3][1] + v.z * m[3][2] + m[3][3];
    
    if (w != 0.0f && w != 1.0f) {
        return Vector3(x / w, y / w, z / w);
    }
    
    return Vector3(x, y, z);
}