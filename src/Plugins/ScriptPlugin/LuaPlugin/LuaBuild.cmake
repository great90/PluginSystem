# LuaBuild.cmake - Lua build configuration file

# Ensure Lua uses C compiler instead of C++ compiler
set(CMAKE_C_STANDARD_REQUIRED ON)

# Download and build Lua from GitHub
include(FetchContent)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.6
    GIT_SHALLOW TRUE
)

# Make Lua available
FetchContent_MakeAvailable(lua)

# Set Lua source directory
set(LUA_SOURCE_DIR ${lua_SOURCE_DIR})

# Check if LUA_SOURCE_DIR exists, if not use the FetchContent path
if(NOT EXISTS ${LUA_SOURCE_DIR})
    set(LUA_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/lua-src)
endif()

# Use GLOB to collect Lua library source files, excluding lua.c, luac.c and onelua.c
file(GLOB LUA_LIB_SOURCES "${LUA_SOURCE_DIR}/*.c")

# Exclude unwanted files
list(FILTER LUA_LIB_SOURCES EXCLUDE REGEX "(lua\.c|luac\.c|onelua\.c)$")

# Collect source files for compiling lua executable
file(GLOB LUA_EXECUTABLE_SOURCES "${LUA_SOURCE_DIR}/lua.c")

# Output debug information
message(STATUS "Lua library sources: ${LUA_LIB_SOURCES}")
message(STATUS "Lua executable sources: ${LUA_EXECUTABLE_SOURCES}")

# Set the compilation language of Lua source files to C
set_source_files_properties(${LUA_LIB_SOURCES} ${LUA_EXECUTABLE_SOURCES} PROPERTIES LANGUAGE C)

# Create Lua library
add_library(lua_lib STATIC ${LUA_LIB_SOURCES})
set_target_properties(lua_lib PROPERTIES 
    OUTPUT_NAME lualib
    LINKER_LANGUAGE C
    C_STANDARD 99
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_include_directories(lua_lib PUBLIC ${LUA_SOURCE_DIR})

# Create Lua executable
add_executable(lua_exe ${LUA_EXECUTABLE_SOURCES})
set_target_properties(lua_exe PROPERTIES
    OUTPUT_NAME lua
    LINKER_LANGUAGE C
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_link_libraries(lua_exe PRIVATE lua_lib)

# Define platform-specific settings
if(WIN32)
    # Windows platform settings
    target_compile_definitions(lua_lib PRIVATE LUA_BUILD_AS_DLL)
    # Add LUA_DLL definition for executable, indicating use of DLL version of Lua
    target_compile_definitions(lua_exe PRIVATE LUA_DLL)
    if(MSVC)
        target_compile_definitions(lua_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(lua_exe PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
elseif(UNIX)
    # Linux/Unix platform settings
    if(NOT APPLE)
        # Linux specific settings
        target_compile_definitions(lua_lib PRIVATE LUA_USE_LINUX)
        target_compile_definitions(lua_exe PRIVATE LUA_USE_LINUX)
        # Check readline library
        find_package(Readline)
        if(Readline_FOUND)
            target_compile_definitions(lua_lib PRIVATE LUA_USE_READLINE)
            target_compile_definitions(lua_exe PRIVATE LUA_USE_READLINE)
            target_include_directories(lua_lib PRIVATE ${Readline_INCLUDE_DIRS})
            target_include_directories(lua_exe PRIVATE ${Readline_INCLUDE_DIRS})
            target_link_libraries(lua_exe PRIVATE ${Readline_LIBRARIES})
        endif()
        # Add dl library
        target_link_libraries(lua_exe PRIVATE dl)
    else()
        # macOS specific settings
        target_compile_definitions(lua_lib PRIVATE LUA_USE_MACOSX)
        target_compile_definitions(lua_exe PRIVATE LUA_USE_MACOSX)
    endif()
endif()

# Add math library link (only on non-Windows platforms)
if(NOT WIN32)
    target_link_libraries(lua_exe PRIVATE m)
endif()