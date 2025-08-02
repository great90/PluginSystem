# tests/CMakeLists.txt

# Download and build googletest from GitHub
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

# Configure googletest build
set(BUILD_GMOCK ON CACHE BOOL "Build GMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Install GTest" FORCE)

# Make googletest available
FetchContent_MakeAvailable(googletest)

# Include googletest headers
include_directories(${googletest_SOURCE_DIR}/googletest/include)

# Define test sources
set(TEST_SOURCES
    PluginCoreTests.cpp
    MathPluginTests.cpp
    LogPluginTests.cpp
    ScriptPluginTests.cpp
)

# Add manual test executable
add_executable(manual_test
    manual_test.cpp
)

# googletest includes are now handled above

# Set include directories for manual test
target_include_directories(manual_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/PluginCore/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/LogPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/MathPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/PythonPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/LuaPlugin/include
)

# Link dependencies for manual test
target_link_libraries(manual_test PRIVATE
    PluginCore
)

# Set output directory for manual test
set_target_properties(manual_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Create plugins directory in the test output directory
add_custom_command(TARGET manual_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/plugins"
    COMMENT "Creating plugins directory for tests"
)

# Copy plugin libraries to the plugins directory
add_custom_command(TARGET manual_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:LogPlugin>"
        "$<TARGET_FILE:MathPlugin>"
        "$<TARGET_FILE:ScriptPlugin>"
        "$<TARGET_FILE:PythonPlugin>"
        "$<TARGET_FILE:LuaPlugin>"
        "${CMAKE_BINARY_DIR}/bin/plugins/"
    COMMENT "Copying plugin libraries to plugins directory for tests"
)

# Create test executable
add_executable(PluginSystemTests ${TEST_SOURCES})

# Set include directories for unit tests
target_include_directories(PluginSystemTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/PluginCore/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/LogPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/MathPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/PythonPlugin/include
    ${CMAKE_SOURCE_DIR}/src/Plugins/ScriptPlugin/LuaPlugin/include
)

# Link dependencies
target_link_libraries(PluginSystemTests PRIVATE
    PluginCore
    MathPlugin
    LogPlugin
    ScriptPlugin
    PythonPlugin
    LuaPlugin
    gtest
    gtest_main
)

# Set output directory for unit tests
set_target_properties(PluginSystemTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add test
add_test(NAME PluginSystemTests COMMAND PluginSystemTests)

# Copy plugin libraries for unit tests
add_custom_command(TARGET PluginSystemTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:LogPlugin>"
        "$<TARGET_FILE:MathPlugin>"
        "$<TARGET_FILE:ScriptPlugin>"
        "$<TARGET_FILE:PythonPlugin>"
        "$<TARGET_FILE:LuaPlugin>"
        "${CMAKE_BINARY_DIR}/bin/plugins/"
    COMMENT "Copying plugin libraries for unit tests"
)

# Copy example scripts to the test output directory
add_custom_command(TARGET manual_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/examples/python_example.py"
        "${CMAKE_SOURCE_DIR}/examples/lua_example.lua"
        "${CMAKE_BINARY_DIR}/bin/"
    COMMENT "Copying example scripts to test output directory"
)