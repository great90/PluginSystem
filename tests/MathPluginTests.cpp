/**
 * @file MathPluginTests.cpp
 * @brief Unit tests for the MathPlugin
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "MathPlugin.h"
#include <cmath>
#include <memory>

class MathPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    MathPlugin* mathPlugin;

    void SetUp() override {
        // Set plugin directory
        pluginManager.SetPluginDirectory("./plugins");
        
        // Load MathPlugin
        ASSERT_TRUE(pluginManager.LoadPlugin("MathPlugin"));
        
        // Get MathPlugin instance
        IPlugin* plugin = pluginManager.GetPlugin("MathPlugin");
        ASSERT_NE(plugin, nullptr);
        
        // Cast to MathPlugin
        mathPlugin = dynamic_cast<MathPlugin*>(plugin);
        ASSERT_NE(mathPlugin, nullptr);
    }

    void TearDown() override {
        // Unload all plugins
        pluginManager.UnloadAllPlugins();
        mathPlugin = nullptr;
    }
};

// Test Vector3 operations
TEST_F(MathPluginTest, Vector3Operations) {
    // Create vectors
    Vector3 v1 = mathPlugin->CreateVector3(1.0f, 2.0f, 3.0f);
    Vector3 v2 = mathPlugin->CreateVector3(4.0f, 5.0f, 6.0f);
    
    // Test addition
    Vector3 sum = mathPlugin->AddVectors(v1, v2);
    EXPECT_FLOAT_EQ(sum.x, 5.0f);
    EXPECT_FLOAT_EQ(sum.y, 7.0f);
    EXPECT_FLOAT_EQ(sum.z, 9.0f);
    
    // Test subtraction
    Vector3 diff = mathPlugin->SubtractVectors(v2, v1);
    EXPECT_FLOAT_EQ(diff.x, 3.0f);
    EXPECT_FLOAT_EQ(diff.y, 3.0f);
    EXPECT_FLOAT_EQ(diff.z, 3.0f);
    
    // Test scalar multiplication
    Vector3 scaled = mathPlugin->MultiplyVector(v1, 2.0f);
    EXPECT_FLOAT_EQ(scaled.x, 2.0f);
    EXPECT_FLOAT_EQ(scaled.y, 4.0f);
    EXPECT_FLOAT_EQ(scaled.z, 6.0f);
    
    // Test dot product
    float dot = mathPlugin->DotProduct(v1, v2);
    EXPECT_FLOAT_EQ(dot, 1.0f*4.0f + 2.0f*5.0f + 3.0f*6.0f);
    
    // Test cross product
    Vector3 cross = mathPlugin->CrossProduct(v1, v2);
    EXPECT_FLOAT_EQ(cross.x, 2.0f*6.0f - 3.0f*5.0f);
    EXPECT_FLOAT_EQ(cross.y, 3.0f*4.0f - 1.0f*6.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f*5.0f - 2.0f*4.0f);
    
    // Test magnitude
    float mag = mathPlugin->Magnitude(v1);
    EXPECT_FLOAT_EQ(mag, std::sqrt(1.0f*1.0f + 2.0f*2.0f + 3.0f*3.0f));
    
    // Test normalization
    Vector3 normalized = mathPlugin->Normalize(v1);
    float expectedLength = 1.0f;
    float actualLength = std::sqrt(normalized.x*normalized.x + normalized.y*normalized.y + normalized.z*normalized.z);
    EXPECT_NEAR(actualLength, expectedLength, 1e-6f);
}

// Test Quaternion operations
TEST_F(MathPluginTest, QuaternionOperations) {
    // Create quaternions
    Quaternion q1 = mathPlugin->CreateQuaternion(1.0f, 0.0f, 0.0f, 0.0f); // Identity rotation
    Quaternion q2 = mathPlugin->CreateQuaternion(0.0f, 0.0f, 0.0f, 1.0f); // 180 degrees around Z
    
    // Test quaternion multiplication (composition of rotations)
    Quaternion qMul = mathPlugin->MultiplyQuaternions(q1, q2);
    EXPECT_FLOAT_EQ(qMul.w, 0.0f);
    EXPECT_FLOAT_EQ(qMul.x, 0.0f);
    EXPECT_FLOAT_EQ(qMul.y, 0.0f);
    EXPECT_FLOAT_EQ(qMul.z, 1.0f);
    
    // Test quaternion normalization
    Quaternion q3 = mathPlugin->CreateQuaternion(2.0f, 2.0f, 2.0f, 2.0f);
    Quaternion qNorm = mathPlugin->NormalizeQuaternion(q3);
    float magnitude = std::sqrt(qNorm.w*qNorm.w + qNorm.x*qNorm.x + qNorm.y*qNorm.y + qNorm.z*qNorm.z);
    EXPECT_NEAR(magnitude, 1.0f, 1e-6f);
    
    // Test quaternion to Euler angles
    Vector3 euler = mathPlugin->QuaternionToEuler(q2);
    EXPECT_NEAR(euler.z, 3.14159f, 0.1f); // Approximately 180 degrees in radians
    
    // Test Euler angles to quaternion
    Quaternion qFromEuler = mathPlugin->EulerToQuaternion(euler);
    EXPECT_NEAR(qFromEuler.w, q2.w, 1e-2f);
    EXPECT_NEAR(qFromEuler.x, q2.x, 1e-2f);
    EXPECT_NEAR(qFromEuler.y, q2.y, 1e-2f);
    EXPECT_NEAR(qFromEuler.z, q2.z, 1e-2f);
}

// Test Matrix4x4 operations
TEST_F(MathPluginTest, MatrixOperations) {
    // Create identity matrix
    Matrix4x4 identity = mathPlugin->CreateIdentityMatrix();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_FLOAT_EQ(identity.m[i][j], 1.0f);
            } else {
                EXPECT_FLOAT_EQ(identity.m[i][j], 0.0f);
            }
        }
    }
    
    // Create translation matrix
    Vector3 translation = mathPlugin->CreateVector3(1.0f, 2.0f, 3.0f);
    Matrix4x4 translationMatrix = mathPlugin->CreateTranslationMatrix(translation);
    EXPECT_FLOAT_EQ(translationMatrix.m[0][3], 1.0f);
    EXPECT_FLOAT_EQ(translationMatrix.m[1][3], 2.0f);
    EXPECT_FLOAT_EQ(translationMatrix.m[2][3], 3.0f);
    
    // Create rotation matrix
    Quaternion rotation = mathPlugin->CreateQuaternion(0.7071f, 0.0f, 0.7071f, 0.0f); // 90 degrees around Y
    Matrix4x4 rotationMatrix = mathPlugin->CreateRotationMatrix(rotation);
    
    // Test matrix multiplication
    Matrix4x4 combined = mathPlugin->MultiplyMatrices(translationMatrix, rotationMatrix);
    
    // Transform a point
    Vector3 point = mathPlugin->CreateVector3(1.0f, 0.0f, 0.0f);
    Vector3 transformed = mathPlugin->TransformPoint(combined, point);
    
    // The point should be rotated and then translated
    EXPECT_NEAR(transformed.x, 1.0f, 0.1f); // Translation.x
    EXPECT_NEAR(transformed.y, 2.0f, 0.1f); // Translation.y
    EXPECT_NEAR(transformed.z, 4.0f, 0.1f); // Translation.z + 1.0f (rotated)
}

// Test interpolation functions
TEST_F(MathPluginTest, InterpolationFunctions) {
    // Test linear interpolation
    float lerp = mathPlugin->Lerp(0.0f, 10.0f, 0.5f);
    EXPECT_FLOAT_EQ(lerp, 5.0f);
    
    // Test vector interpolation
    Vector3 v1 = mathPlugin->CreateVector3(0.0f, 0.0f, 0.0f);
    Vector3 v2 = mathPlugin->CreateVector3(10.0f, 20.0f, 30.0f);
    Vector3 vLerp = mathPlugin->LerpVector(v1, v2, 0.5f);
    EXPECT_FLOAT_EQ(vLerp.x, 5.0f);
    EXPECT_FLOAT_EQ(vLerp.y, 10.0f);
    EXPECT_FLOAT_EQ(vLerp.z, 15.0f);
    
    // Test quaternion interpolation (slerp)
    Quaternion q1 = mathPlugin->CreateQuaternion(1.0f, 0.0f, 0.0f, 0.0f); // Identity
    Quaternion q2 = mathPlugin->CreateQuaternion(0.0f, 1.0f, 0.0f, 0.0f); // 180 degrees around X
    Quaternion qSlerp = mathPlugin->SlerpQuaternion(q1, q2, 0.5f);
    EXPECT_NEAR(qSlerp.w, 0.7071f, 1e-4f);
    EXPECT_NEAR(qSlerp.x, 0.7071f, 1e-4f);
    EXPECT_NEAR(qSlerp.y, 0.0f, 1e-4f);
    EXPECT_NEAR(qSlerp.z, 0.0f, 1e-4f);
}

// Test random number generation
TEST_F(MathPluginTest, RandomNumberGeneration) {
    // Test random float
    float min = 0.0f;
    float max = 10.0f;
    float random = mathPlugin->RandomFloat(min, max);
    EXPECT_GE(random, min);
    EXPECT_LE(random, max);
    
    // Test random int
    int minInt = 0;
    int maxInt = 100;
    int randomInt = mathPlugin->RandomInt(minInt, maxInt);
    EXPECT_GE(randomInt, minInt);
    EXPECT_LE(randomInt, maxInt);
    
    // Test random vector
    Vector3 minVec = mathPlugin->CreateVector3(0.0f, 0.0f, 0.0f);
    Vector3 maxVec = mathPlugin->CreateVector3(10.0f, 10.0f, 10.0f);
    Vector3 randomVec = mathPlugin->RandomVector(minVec, maxVec);
    EXPECT_GE(randomVec.x, minVec.x);
    EXPECT_LE(randomVec.x, maxVec.x);
    EXPECT_GE(randomVec.y, minVec.y);
    EXPECT_LE(randomVec.y, maxVec.y);
    EXPECT_GE(randomVec.z, minVec.z);
    EXPECT_LE(randomVec.z, maxVec.z);
}