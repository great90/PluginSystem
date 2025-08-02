/**
 * @file Vector3.h
 * @brief Defines the Vector3 structure for 3D vector operations
 */

#pragma once

#include "MathPluginExport.h"
#include <cmath>

/**
 * @struct Vector3
 * @brief 3D vector structure with x, y, z components
 */
struct MATH_PLUGIN_API Vector3 {
    float x, y, z;
    
    /**
     * @brief Default constructor
     */
    Vector3();
    
    /**
     * @brief Constructor with initial values
     * 
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
    Vector3(float x, float y, float z);
    
    /**
     * @brief Copy constructor
     * 
     * @param other Vector to copy from
     */
    Vector3(const Vector3& other);
    
    /**
     * @brief Calculate the length (magnitude) of the vector
     * 
     * @return The length of the vector
     */
    float Length() const;
    
    /**
     * @brief Calculate the squared length of the vector
     * 
     * @return The squared length of the vector
     */
    float LengthSquared() const;
    
    /**
     * @brief Normalize the vector (make it unit length)
     * 
     * @return Reference to this vector after normalization
     */
    Vector3& Normalize();
    
    /**
     * @brief Get a normalized copy of this vector
     * 
     * @return A normalized copy of this vector
     */
    Vector3 Normalized() const;
    
    /**
     * @brief Calculate the dot product with another vector
     * 
     * @param other The other vector
     * @return The dot product
     */
    float Dot(const Vector3& other) const;
    
    /**
     * @brief Calculate the cross product with another vector
     * 
     * @param other The other vector
     * @return The cross product vector
     */
    Vector3 Cross(const Vector3& other) const;
    
    /**
     * @brief Add another vector to this one
     * 
     * @param other The vector to add
     * @return Reference to this vector after addition
     */
    Vector3& operator+=(const Vector3& other);
    
    /**
     * @brief Subtract another vector from this one
     * 
     * @param other The vector to subtract
     * @return Reference to this vector after subtraction
     */
    Vector3& operator-=(const Vector3& other);
    
    /**
     * @brief Multiply this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after multiplication
     */
    Vector3& operator*=(float scalar);
    
    /**
     * @brief Divide this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after division
     */
    Vector3& operator/=(float scalar);
};

/**
 * @brief Add two vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of addition
 */
MATH_PLUGIN_API Vector3 operator+(const Vector3& a, const Vector3& b);

/**
 * @brief Subtract one vector from another
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of subtraction
 */
MATH_PLUGIN_API Vector3 operator-(const Vector3& a, const Vector3& b);

/**
 * @brief Negate a vector
 * 
 * @param v Vector to negate
 * @return Negated vector
 */
MATH_PLUGIN_API Vector3 operator-(const Vector3& v);

/**
 * @brief Multiply a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector3 operator*(const Vector3& v, float scalar);

/**
 * @brief Multiply a scalar by a vector
 * 
 * @param scalar Scalar value
 * @param v Vector
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector3 operator*(float scalar, const Vector3& v);

/**
 * @brief Divide a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector3 operator/(const Vector3& v, float scalar);