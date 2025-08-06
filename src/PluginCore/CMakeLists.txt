
# Define source files
set(PLUGIN_CORE_SOURCES
    src/PluginManager.cpp
    src/DependencyResolver.cpp
    src/ScriptObjectWrapper.cpp
)

# Define header files
set(PLUGIN_CORE_HEADERS
    include/IPlugin.h
    include/PluginInfo.h
    include/PluginManager.h
    include/DependencyResolver.h
    include/PluginExport.h
    include/ScriptObjectWrapper.h
)

# Create library target
if(BUILD_SHARED_LIBS)
    add_library(PluginCore SHARED ${PLUGIN_CORE_SOURCES} ${PLUGIN_CORE_HEADERS})
    target_compile_definitions(PluginCore PRIVATE PLUGIN_CORE_EXPORTS)
    target_compile_definitions(PluginCore PUBLIC PLUGIN_CORE_SHARED)
else()
    add_library(PluginCore STATIC ${PLUGIN_CORE_SOURCES} ${PLUGIN_CORE_HEADERS})
endif()

# Set include directories
target_include_directories(PluginCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Installation rules
install(TARGETS PluginCore
    EXPORT PluginCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${PLUGIN_CORE_HEADERS} DESTINATION include/PluginCore)