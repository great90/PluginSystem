/**
 * @file Vector2.h
 * @brief Defines the Vector2 structure for 2D vector operations
 */

#pragma once

#include "MathPluginExport.h"
#include <cmath>

/**
 * @struct Vector2
 * @brief 2D vector structure with x, y components
 */
struct MATH_PLUGIN_API Vector2 {
    float x, y;
    
    /**
     * @brief Default constructor
     */
    Vector2();
    
    /**
     * @brief Constructor with initial values
     * 
     * @param x X component
     * @param y Y component
     */
    Vector2(float x, float y);
    
    /**
     * @brief Copy constructor
     * 
     * @param other Vector to copy from
     */
    Vector2(const Vector2& other);
    
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
    Vector2& Normalize();
    
    /**
     * @brief Get a normalized copy of this vector
     * 
     * @return A normalized copy of this vector
     */
    Vector2 Normalized() const;
    
    /**
     * @brief Calculate the dot product with another vector
     * 
     * @param other The other vector
     * @return The dot product
     */
    float Dot(const Vector2& other) const;
    
    /**
     * @brief Calculate the 2D cross product (returns a scalar)
     * 
     * @param other The other vector
     * @return The cross product scalar (x1*y2 - y1*x2)
     */
    float Cross(const Vector2& other) const;
    
    /**
     * @brief Add another vector to this one
     * 
     * @param other The vector to add
     * @return Reference to this vector after addition
     */
    Vector2& operator+=(const Vector2& other);
    
    /**
     * @brief Subtract another vector from this one
     * 
     * @param other The vector to subtract
     * @return Reference to this vector after subtraction
     */
    Vector2& operator-=(const Vector2& other);
    
    /**
     * @brief Multiply this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after multiplication
     */
    Vector2& operator*=(float scalar);
    
    /**
     * @brief Divide this vector by a scalar
     * 
     * @param scalar The scalar value
     * @return Reference to this vector after division
     */
    Vector2& operator/=(float scalar);
};

/**
 * @brief Add two vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of addition
 */
MATH_PLUGIN_API Vector2 operator+(const Vector2& a, const Vector2& b);

/**
 * @brief Subtract one vector from another
 * 
 * @param a First vector
 * @param b Second vector
 * @return Result of subtraction
 */
MATH_PLUGIN_API Vector2 operator-(const Vector2& a, const Vector2& b);

/**
 * @brief Negate a vector
 * 
 * @param v Vector to negate
 * @return Negated vector
 */
MATH_PLUGIN_API Vector2 operator-(const Vector2& v);

/**
 * @brief Multiply a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector2 operator*(const Vector2& v, float scalar);

/**
 * @brief Multiply a scalar by a vector
 * 
 * @param scalar Scalar value
 * @param v Vector
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector2 operator*(float scalar, const Vector2& v);

/**
 * @brief Divide a vector by a scalar
 * 
 * @param v Vector
 * @param scalar Scalar value
 * @return Scaled vector
 */
MATH_PLUGIN_API Vector2 operator/(const Vector2& v, float scalar);