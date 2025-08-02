/**
 * @file Vector4.h
 * @brief Defines the Vector4 structure for 4D vector operations
 */

#pragma once

#include "MathPluginExport.h"
#include <cmath>

// Forward declarations
struct Vector3;

/**
 * @struct Vector4
 * @brief 4D vector structure with x, y, z, w components
 */
struct MATH_PLUGIN_API Vector4 {
    float x, y, z, w;
    
    /**
     * @brief Default constructor
     */
    Vector4();
    
    /**
     * @brief Constructor with initial values
     * 
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
    Vector4(float x, float y, float z, float w);
    
    /**
     * @brief Constructor from Vector3 with w component
     * 
     * @param v Vector3 for x, y, z components
     * @param w W component
     */
    Vector4(const Vector3& v, float w);
    
    /**
     * @brief Copy constructor
     * 
     * @param other Vector to copy from
     */
    Vector4(const Vector4& other);
    
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
    Vector4& Normalize();
    
    /**
     * @brief Get a normalized copy of this vector
     * 
     * @return A normalized copy of this vector
     */
    Vector4 Normalized() const;
    
    /**
     * @brief Calculate the dot product with another vector
     * 
     * @param other The other vector
     * @return The dot product
     */
    float Dot(const Vector4& other) const;
    
    /**
     * @brief Convert to Vector3 by dropping the w component
     * 
     * @return Vector3 with x, y, z components
     */
    Vector3 ToVector3() const;
    
    /**
     * @brief Add another vector to this one
     * 
     * @param other The vector to add
     * @return Reference to this vector after addition
     */
    Vector4& operator+=(const Vector4& other);
    
    /**
     * @brief Subtract another vector from this one
     * 
     * @param other The vector to subtract
     * @return Reference to this vector after subtraction
     */
    Vector4& operator-=(const Vector4& other);
    
    /**
     * @brief Multiply this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after multiplication
     */
    Vector4& operator*=(float scalar);
    
    /**
     * @brief Divide this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after division
     */
    Vector4& operator/=(float scalar);
};

/**
 * @brief Add two vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of addition
 */
MATH_PLUGIN_API Vector4 operator+(const Vector4& a, const Vector4& b);

/**
 * @brief Subtract one vector from another
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of subtraction
 */
MATH_PLUGIN_API Vector4 operator-(const Vector4& a, const Vector4& b);

/**
 * @brief Negate a vector
 * 
 * @param v Vector to negate
 * @return Negated vector
 */
MATH_PLUGIN_API Vector4 operator-(const Vector4& v);

/**
 * @brief Multiply a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector4 operator*(const Vector4& v, float scalar);

/**
 * @brief Multiply a scalar by a vector
 * 
 * @param scalar Scalar value
 * @param v Vector
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector4 operator*(float scalar, const Vector4& v);

/**
 * @brief Divide a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector4 operator/(const Vector4& v, float scalar);