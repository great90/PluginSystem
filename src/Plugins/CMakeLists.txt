# src/Plugins/CMakeLists.txt

# Get plugin list from parent scope
if(NOT DEFINED PLUGIN_LIST)
    message(FATAL_ERROR "PLUGIN_LIST not defined")
endif()

# Initialize global property for built plugins
set_property(GLOBAL PROPERTY BUILT_PLUGINS "")

# Conditionally add plugins based on build options
foreach(PLUGIN ${PLUGIN_LIST})
    string(TOUPPER ${PLUGIN} PLUGIN_UPPER)
    if(BUILD_${PLUGIN_UPPER})
        message(STATUS "构建插件: ${PLUGIN}")
        add_subdirectory(${PLUGIN})
        
        # Add plugin to global list of built plugins
        set_property(GLOBAL APPEND PROPERTY BUILT_PLUGINS ${PLUGIN})
    else()
        message(STATUS "跳过插件: ${PLUGIN}")
    endif()
endforeach()