# src/PluginApp/CMakeLists.txt

# Define source files
set(PLUGIN_APP_SOURCES
    src/main.cpp
    src/Application.cpp
)

# Define header files
set(PLUGIN_APP_HEADERS
    src/Application.h
)

# Create executable
add_executable(PluginApp ${PLUGIN_APP_SOURCES} ${PLUGIN_APP_HEADERS})

# Set include directories
target_include_directories(PluginApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link dependencies
target_link_libraries(PluginApp PRIVATE
    PluginCore
    MathPlugin
    LogPlugin
    ScriptPlugin
    PythonPlugin
    LuaPlugin
)

# Installation rules
install(TARGETS PluginApp
    RUNTIME DESTINATION bin
)