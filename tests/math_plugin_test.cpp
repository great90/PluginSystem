/**
 * @file math_plugin_test.cpp
 * @brief Unit tests for the MathPlugin class
 */

#include <gtest/gtest.h>
#include "PluginManager.h"
#include "MathPlugin.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include <cmath>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define plugin extension based on platform
#ifdef _WIN32
#define PLUGIN_EXTENSION ".dll"
#elif defined(__APPLE__)
#define PLUGIN_EXTENSION ".dylib"
#else
#define PLUGIN_EXTENSION ".so"
#endif

// Test fixture for MathPlugin tests
class MathPluginTest : public ::testing::Test {
protected:
    PluginManager pluginManager;
    MathPlugin* mathPlugin;
    
    void SetUp() override {
        // Set the plugin directory and load the MathPlugin
        pluginManager.SetPluginDirectory("plugins");
        pluginManager.LoadPlugin("plugins/MathPlugin" + std::string(PLUGIN_EXTENSION));
        
        // Get the MathPlugin instance
        mathPlugin = pluginManager.GetPlugin<MathPlugin>("MathPlugin");
        ASSERT_NE(nullptr, mathPlugin);
    }
    
    void TearDown() override {
        // Unload all plugins to clean up
        pluginManager.UnloadAllPlugins();
        mathPlugin = nullptr;
    }
};

// Test the plugin info
TEST_F(MathPluginTest, PluginInfoTest) {
    const PluginInfo& info = mathPlugin->GetPluginInfo();
    
    EXPECT_EQ("MathPlugin", info.name);
    EXPECT_EQ("Math Utilities Plugin", info.displayName);
    EXPECT_EQ("Provides mathematical utilities and types", info.description);
    EXPECT_EQ("Plugin System Team", info.author);
    
    // Check version
    EXPECT_EQ(1, info.version.major);
    EXPECT_EQ(0, info.version.minor);
    EXPECT_EQ(0, info.version.patch);
    EXPECT_EQ("1.0.0", info.version.ToString());
}

// Test angle conversion functions
TEST_F(MathPluginTest, AngleConversionTest) {
    // Test degrees to radians
    EXPECT_NEAR(0.0f, mathPlugin->DegreesToRadians(0.0f), 0.0001f);
    EXPECT_NEAR(M_PI / 4.0f, mathPlugin->DegreesToRadians(45.0f), 0.0001f);
    EXPECT_NEAR(M_PI / 2.0f, mathPlugin->DegreesToRadians(90.0f), 0.0001f);
    EXPECT_NEAR(M_PI, mathPlugin->DegreesToRadians(180.0f), 0.0001f);
    EXPECT_NEAR(2.0f * M_PI, mathPlugin->DegreesToRadians(360.0f), 0.0001f);
    
    // Test radians to degrees
    EXPECT_NEAR(0.0f, mathPlugin->RadiansToDegrees(0.0f), 0.0001f);
    EXPECT_NEAR(45.0f, mathPlugin->RadiansToDegrees(M_PI / 4.0f), 0.0001f);
    EXPECT_NEAR(90.0f, mathPlugin->RadiansToDegrees(M_PI / 2.0f), 0.0001f);
    EXPECT_NEAR(180.0f, mathPlugin->RadiansToDegrees(M_PI), 0.0001f);
    EXPECT_NEAR(360.0f, mathPlugin->RadiansToDegrees(2.0f * M_PI), 0.0001f);
}

// Test linear interpolation functions
TEST_F(MathPluginTest, LerpTest) {
    // Test scalar lerp
    EXPECT_NEAR(1.0f, mathPlugin->Lerp(1.0f, 2.0f, 0.0f), 0.0001f);
    EXPECT_NEAR(1.5f, mathPlugin->Lerp(1.0f, 2.0f, 0.5f), 0.0001f);
    EXPECT_NEAR(2.0f, mathPlugin->Lerp(1.0f, 2.0f, 1.0f), 0.0001f);
    
    // Test Vector3 lerp
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    
    Vector3 result = mathPlugin->Lerp(v1, v2, 0.0f);
    EXPECT_NEAR(1.0f, result.x, 0.0001f);
    EXPECT_NEAR(2.0f, result.y, 0.0001f);
    EXPECT_NEAR(3.0f, result.z, 0.0001f);
    
    result = mathPlugin->Lerp(v1, v2, 0.5f);
    EXPECT_NEAR(2.5f, result.x, 0.0001f);
    EXPECT_NEAR(3.5f, result.y, 0.0001f);
    EXPECT_NEAR(4.5f, result.z, 0.0001f);
    
    result = mathPlugin->Lerp(v1, v2, 1.0f);
    EXPECT_NEAR(4.0f, result.x, 0.0001f);
    EXPECT_NEAR(5.0f, result.y, 0.0001f);
    EXPECT_NEAR(6.0f, result.z, 0.0001f);
}

// Test quaternion spherical linear interpolation
TEST_F(MathPluginTest, SlerpTest) {
    // Create two quaternions representing rotations
    Quaternion q1 = Quaternion::FromAxisAngle(Vector3(0, 1, 0), 0.0f);
    Quaternion q2 = Quaternion::FromAxisAngle(Vector3(0, 1, 0), mathPlugin->DegreesToRadians(90.0f));
    
    // Test slerp at different t values
    Quaternion result = mathPlugin->Slerp(q1, q2, 0.0f);
    EXPECT_NEAR(q1.x, result.x, 0.0001f);
    EXPECT_NEAR(q1.y, result.y, 0.0001f);
    EXPECT_NEAR(q1.z, result.z, 0.0001f);
    EXPECT_NEAR(q1.w, result.w, 0.0001f);
    
    result = mathPlugin->Slerp(q1, q2, 1.0f);
    EXPECT_NEAR(q2.x, result.x, 0.0001f);
    EXPECT_NEAR(q2.y, result.y, 0.0001f);
    EXPECT_NEAR(q2.z, result.z, 0.0001f);
    EXPECT_NEAR(q2.w, result.w, 0.0001f);
    
    // Test slerp at 0.5 (should be halfway between)
    result = mathPlugin->Slerp(q1, q2, 0.5f);
    Quaternion expected = Quaternion::FromAxisAngle(Vector3(0, 1, 0), mathPlugin->DegreesToRadians(45.0f));
    EXPECT_NEAR(expected.x, result.x, 0.0001f);
    EXPECT_NEAR(expected.y, result.y, 0.0001f);
    EXPECT_NEAR(expected.z, result.z, 0.0001f);
    EXPECT_NEAR(expected.w, result.w, 0.0001f);
}

// Test clamp function
TEST_F(MathPluginTest, ClampTest) {
    EXPECT_NEAR(5.0f, mathPlugin->Clamp(5.0f, 0.0f, 10.0f), 0.0001f);
    EXPECT_NEAR(0.0f, mathPlugin->Clamp(-5.0f, 0.0f, 10.0f), 0.0001f);
    EXPECT_NEAR(10.0f, mathPlugin->Clamp(15.0f, 0.0f, 10.0f), 0.0001f);
}

// Test random functions
TEST_F(MathPluginTest, RandomTest) {
    // Test random float
    for (int i = 0; i < 100; ++i) {
        float value = mathPlugin->Random(5.0f, 10.0f);
        EXPECT_GE(value, 5.0f);
        EXPECT_LE(value, 10.0f);
    }
    
    // Test random int
    for (int i = 0; i < 100; ++i) {
        int value = mathPlugin->RandomInt(5, 10);
        EXPECT_GE(value, 5);
        EXPECT_LE(value, 10);
    }
}

// Test Vector3 operations
TEST_F(MathPluginTest, Vector3Test) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    
    // Test addition
    Vector3 sum = v1 + v2;
    EXPECT_NEAR(5.0f, sum.x, 0.0001f);
    EXPECT_NEAR(7.0f, sum.y, 0.0001f);
    EXPECT_NEAR(9.0f, sum.z, 0.0001f);
    
    // Test subtraction
    Vector3 diff = v2 - v1;
    EXPECT_NEAR(3.0f, diff.x, 0.0001f);
    EXPECT_NEAR(3.0f, diff.y, 0.0001f);
    EXPECT_NEAR(3.0f, diff.z, 0.0001f);
    
    // Test scalar multiplication
    Vector3 scaled = v1 * 2.0f;
    EXPECT_NEAR(2.0f, scaled.x, 0.0001f);
    EXPECT_NEAR(4.0f, scaled.y, 0.0001f);
    EXPECT_NEAR(6.0f, scaled.z, 0.0001f);
    
    // Test dot product
    float dot = v1.Dot(v2);
    EXPECT_NEAR(32.0f, dot, 0.0001f);  // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    
    // Test cross product
    Vector3 cross = v1.Cross(v2);
    EXPECT_NEAR(-3.0f, cross.x, 0.0001f);  // (2*6 - 3*5) = 12 - 15 = -3
    EXPECT_NEAR(6.0f, cross.y, 0.0001f);   // (3*4 - 1*6) = 12 - 6 = 6
    EXPECT_NEAR(-3.0f, cross.z, 0.0001f);  // (1*5 - 2*4) = 5 - 8 = -3
    
    // Test length
    float length = v1.Length();
    EXPECT_NEAR(std::sqrt(14.0f), length, 0.0001f);  // sqrt(1^2 + 2^2 + 3^2) = sqrt(14)
    
    // Test normalization
    Vector3 normalized = v1.Normalized();
    float invLength = 1.0f / std::sqrt(14.0f);
    EXPECT_NEAR(1.0f * invLength, normalized.x, 0.0001f);
    EXPECT_NEAR(2.0f * invLength, normalized.y, 0.0001f);
    EXPECT_NEAR(3.0f * invLength, normalized.z, 0.0001f);
}