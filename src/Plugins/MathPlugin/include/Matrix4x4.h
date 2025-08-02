/**
 * @file Matrix4x4.h
 * @brief Defines the Matrix4x4 structure for 3D transformations
 */

#pragma once

#include "MathPluginExport.h"
#include "Vector3.h"
#include <cmath>

/**
 * @struct Matrix4x4
 * @brief 4x4 matrix structure for 3D transformations
 */
struct MATH_PLUGIN_API Matrix4x4 {
    float m[4][4];
    
    /**
     * @brief Default constructor (identity matrix)
     */
    Matrix4x4();
    
    /**
     * @brief Set this matrix to identity
     */
    void Identity();
    
    /**
     * @brief Create a translation matrix
     * 
     * @param x X translation
     * @param y Y translation
     * @param z Z translation
     * @return Translation matrix
     */
    static Matrix4x4 Translation(float x, float y, float z);
    
    /**
     * @brief Create a translation matrix from a vector
     * 
     * @param v Translation vector
     * @return Translation matrix
     */
    static Matrix4x4 Translation(const Vector3& v);
    
    /**
     * @brief Create a scaling matrix
     * 
     * @param x X scale
     * @param y Y scale
     * @param z Z scale
     * @return Scaling matrix
     */
    static Matrix4x4 Scaling(float x, float y, float z);
    
    /**
     * @brief Create a scaling matrix from a vector
     * 
     * @param v Scale vector
     * @return Scaling matrix
     */
    static Matrix4x4 Scaling(const Vector3& v);
    
    /**
     * @brief Create a rotation matrix around the X axis
     * 
     * @param angle Angle in radians
     * @return Rotation matrix
     */
    static Matrix4x4 RotationX(float angle);
    
    /**
     * @brief Create a rotation matrix around the Y axis
     * 
     * @param angle Angle in radians
     * @return Rotation matrix
     */
    static Matrix4x4 RotationY(float angle);
    
    /**
     * @brief Create a rotation matrix around the Z axis
     * 
     * @param angle Angle in radians
     * @return Rotation matrix
     */
    static Matrix4x4 RotationZ(float angle);
    
    /**
     * @brief Multiply this matrix by another
     * 
     * @param other The other matrix
     * @return Result of multiplication
     */
    Matrix4x4 operator*(const Matrix4x4& other) const;
    
    /**
     * @brief Transform a vector by this matrix
     * 
     * @param v Vector to transform
     * @return Transformed vector
     */
    Vector3 TransformVector(const Vector3& v) const;
};