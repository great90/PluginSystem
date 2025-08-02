/**
 * @file Quaternion.h
 * @brief Defines the Quaternion structure for 3D rotations
 */

#pragma once

#include "MathPluginExport.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cmath>

/**
 * @struct Quaternion
 * @brief Quaternion structure for 3D rotations
 */
struct MATH_PLUGIN_API Quaternion {
    float x, y, z, w;
    
    /**
     * @brief Default constructor (identity quaternion)
     */
    Quaternion();
    
    /**
     * @brief Constructor with initial values
     * 
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
    Quaternion(float x, float y, float z, float w);
    
    /**
     * @brief Create a quaternion from axis and angle
     * 
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     * @return Quaternion representing the rotation
     */
    static Quaternion FromAxisAngle(const Vector3& axis, float angle);
    
    /**
     * @brief Create a quaternion from Euler angles (in radians)
     * 
     * @param x Rotation around X axis (pitch)
     * @param y Rotation around Y axis (yaw)
     * @param z Rotation around Z axis (roll)
     * @return Quaternion representing the rotation
     */
    static Quaternion FromEulerAngles(float x, float y, float z);
    
    /**
     * @brief Calculate the length (magnitude) of the quaternion
     * 
     * @return The length of the quaternion
     */
    float Length() const;
    
    /**
     * @brief Normalize the quaternion
     * 
     * @return Reference to this quaternion after normalization
     */
    Quaternion& Normalize();
    
    /**
     * @brief Get the conjugate of this quaternion
     * 
     * @return Conjugate quaternion
     */
    Quaternion Conjugate() const;
    
    /**
     * @brief Get the inverse of this quaternion
     * 
     * @return Inverse quaternion
     */
    Quaternion Inverse() const;
    
    /**
     * @brief Multiply this quaternion by another (composition of rotations)
     * 
     * @param other The other quaternion
     * @return Result of multiplication
     */
    Quaternion operator*(const Quaternion& other) const;
    
    /**
     * @brief Rotate a vector by this quaternion
     * 
     * @param v Vector to rotate
     * @return Rotated vector
     */
    Vector3 RotateVector(const Vector3& v) const;
    
    /**
     * @brief Convert this quaternion to a 4x4 rotation matrix
     * 
     * @return Rotation matrix
     */
    Matrix4x4 ToMatrix() const;
};