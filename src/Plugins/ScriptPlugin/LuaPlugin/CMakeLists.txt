# src/Plugins/ScriptPlugin/LuaPlugin/CMakeLists.txt

# Include Lua build configuration file
include(${CMAKE_CURRENT_SOURCE_DIR}/LuaBuild.cmake)

# Define LuaPlugin source files
set(LUA_PLUGIN_SOURCES
    src/LuaPlugin.cpp
)

# Define LuaPlugin header files
set(LUA_PLUGIN_HEADERS
    include/LuaPlugin.h
)

# Create LuaPlugin library target
if(BUILD_SHARED_LIBS)
    add_library(LuaPlugin SHARED ${LUA_PLUGIN_SOURCES} ${LUA_PLUGIN_HEADERS})
    target_compile_definitions(LuaPlugin PRIVATE LUA_PLUGIN_EXPORTS)
    target_compile_definitions(LuaPlugin PUBLIC LUA_PLUGIN_SHARED)
else()
    add_library(LuaPlugin STATIC ${LUA_PLUGIN_SOURCES} ${LUA_PLUGIN_HEADERS})
endif()

# Set include directories
target_include_directories(LuaPlugin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LUA_SOURCE_DIR}
)

# Add dependency to ensure lua_lib is built before LuaPlugin
add_dependencies(LuaPlugin lua_lib)

# Link dependencies
target_link_libraries(LuaPlugin PRIVATE 
    PluginCore
    ScriptPlugin
    MathPlugin
    lua_lib
)

# Installation rules
install(TARGETS LuaPlugin lua_lib lua_exe
    EXPORT LuaPluginTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${LUA_PLUGIN_HEADERS} DESTINATION include/LuaPlugin)