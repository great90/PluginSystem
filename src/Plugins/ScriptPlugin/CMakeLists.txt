# src/Plugins/ScriptPlugin/CMakeLists.txt

# Define source files
set(SCRIPT_PLUGIN_SOURCES
    src/ScriptPlugin.cpp
)

# Define header files
set(SCRIPT_PLUGIN_HEADERS
    include/ScriptPlugin.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(ScriptPlugin SHARED ${SCRIPT_PLUGIN_SOURCES} ${SCRIPT_PLUGIN_HEADERS})
    target_compile_definitions(ScriptPlugin PRIVATE SCRIPT_PLUGIN_EXPORTS)
    target_compile_definitions(ScriptPlugin PUBLIC SCRIPT_PLUGIN_SHARED)
else()
    add_library(ScriptPlugin STATIC ${SCRIPT_PLUGIN_SOURCES} ${SCRIPT_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(ScriptPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(ScriptPlugin PRIVATE PluginCore)

# Installation rules
install(TARGETS ScriptPlugin
    EXPORT ScriptPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${SCRIPT_PLUGIN_HEADERS} DESTINATION include/ScriptPlugin)

# Add subdirectories for specific script language implementations
add_subdirectory(PythonPlugin)
add_subdirectory(LuaPlugin)