# src/Plugins/MathPlugin/CMakeLists.txt

# Define source files
set(MATH_PLUGIN_SOURCES
    src/MathPlugin.cpp
    src/Vector2.cpp
    src/Vector3.cpp
    src/Vector4.cpp
    src/Matrix4x4.cpp
    src/Quaternion.cpp
)

# Define header files
set(MATH_PLUGIN_HEADERS
    include/MathPlugin.h
    include/Vector2.h
    include/Vector3.h
    include/Vector4.h
    include/Matrix4x4.h
    include/Quaternion.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(MathPlugin SHARED ${MATH_PLUGIN_SOURCES} ${MATH_PLUGIN_HEADERS})
    target_compile_definitions(MathPlugin PRIVATE MATH_PLUGIN_EXPORTS)
    target_compile_definitions(MathPlugin PUBLIC MATH_PLUGIN_SHARED)
else()
    add_library(MathPlugin STATIC ${MATH_PLUGIN_SOURCES} ${MATH_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(MathPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(MathPlugin PRIVATE PluginCore)

# Installation rules
install(TARGETS MathPlugin
    EXPORT MathPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${MATH_PLUGIN_HEADERS} DESTINATION include/MathPlugin)