# PythonBuild.cmake - Python and pybind11 build configuration file

# Include FetchContent module
include(FetchContent)

# Option to build Python from source
option(BUILD_PYTHON_FROM_SOURCE "Build Python from source instead of using system Python" ON)

message(STATUS "Python configuration: BUILD_PYTHON_FROM_SOURCE=${BUILD_PYTHON_FROM_SOURCE}")

if(BUILD_PYTHON_FROM_SOURCE)
    message(STATUS "Downloading Python source code from GitHub...")
    # Download and build Python from GitHub
    FetchContent_Declare(
        python
        GIT_REPOSITORY https://github.com/python/cpython.git
        GIT_TAG v3.10.13
        GIT_SHALLOW TRUE
    )

    # Make Python available
    FetchContent_MakeAvailable(python)
    
    if(NOT python_POPULATED)
        message(FATAL_ERROR "Failed to download Python source code. Please check your internet connection.")
    endif()

    message(STATUS "Python source code downloaded successfully to ${python_SOURCE_DIR}")
else()
    message(STATUS "Using system Python instead of building from source")
    find_package(PythonLibs 3.10 REQUIRED)
    set(PYTHON_SOURCE_DIR "")
endif()

# Set Python source directory
if(BUILD_PYTHON_FROM_SOURCE)
    set(PYTHON_SOURCE_DIR ${python_SOURCE_DIR})

    # Check if PYTHON_SOURCE_DIR exists, if not use the FetchContent path
    if(NOT EXISTS ${PYTHON_SOURCE_DIR})
        set(PYTHON_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/python-src)
    endif()
    
    if(NOT EXISTS ${PYTHON_SOURCE_DIR})
        message(FATAL_ERROR "Python source directory not found at ${PYTHON_SOURCE_DIR}")
    endif()
    
    message(STATUS "Using Python source directory: ${PYTHON_SOURCE_DIR}")
endif()

# Create Python library target
add_library(python_lib INTERFACE)

# Configure Python library based on build option
if(NOT BUILD_PYTHON_FROM_SOURCE)
    # If using system Python, set include directories and libraries from find_package
    target_include_directories(python_lib INTERFACE ${PYTHON_INCLUDE_DIRS})
    target_link_libraries(python_lib INTERFACE ${PYTHON_LIBRARIES})
    
    # Set variables for main CMakeLists.txt
    set(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES python_lib)
    
    message(STATUS "Configured to use system Python libraries")
    message(STATUS "Python include directories: ${PYTHON_INCLUDE_DIRS}")
endif()

# Define platform-specific settings for building Python from source
if(BUILD_PYTHON_FROM_SOURCE)
    if(WIN32)
        # Windows platform settings
        if(MSVC)
            target_compile_definitions(python_lib INTERFACE _CRT_SECURE_NO_WARNINGS)
        endif()
        
        # Configure and build Python on Windows
        # First, check if PCbuild directory exists
        if(NOT EXISTS ${PYTHON_SOURCE_DIR}/PCbuild/build.bat)
            message(FATAL_ERROR "PCbuild/build.bat not found in Python source directory. Check if Python source was downloaded correctly.")
        endif()
        
        # Determine architecture
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(PYTHON_ARCH x64)
        else()
            set(PYTHON_ARCH win32)
        endif()
        
        # Build Python using PCbuild/build.bat
        message(STATUS "Building Python from source using PCbuild/build.bat...")
        message(STATUS "Using architecture: ${PYTHON_ARCH}")
        execute_process(
            COMMAND cmd /c ${PYTHON_SOURCE_DIR}/PCbuild/build.bat -p ${PYTHON_ARCH} -c Release
            WORKING_DIRECTORY ${PYTHON_SOURCE_DIR}/PCbuild
            RESULT_VARIABLE PYTHON_BUILD_RESULT
        )
        
        if(NOT PYTHON_BUILD_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to build Python from source. Error code: ${PYTHON_BUILD_RESULT}")
        endif()
        
        message(STATUS "Python built successfully from source")
        
        # Set Python include directories for Windows
        target_include_directories(python_lib INTERFACE 
            ${PYTHON_SOURCE_DIR}/Include
            ${PYTHON_SOURCE_DIR}/PC
        )
        
        # Link with Python libraries
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit
            target_link_directories(python_lib INTERFACE ${PYTHON_SOURCE_DIR}/PCbuild/amd64)
            target_link_directories(python_lib INTERFACE ${PYTHON_SOURCE_DIR}/PCbuild/x64)
        else()
            # 32-bit
            target_link_directories(python_lib INTERFACE ${PYTHON_SOURCE_DIR}/PCbuild/win32)
        endif()
        
        target_link_libraries(python_lib INTERFACE python310)
    
    elseif(UNIX)
        # Linux/Unix platform settings
        if(NOT APPLE)
            # Linux specific settings
            # Configure and build Python on Linux
            message(STATUS "Building Python from source on Linux...")
            
            # Create a build directory for Python
            set(PYTHON_BUILD_DIR ${CMAKE_BINARY_DIR}/_deps/python-build)
            file(MAKE_DIRECTORY ${PYTHON_BUILD_DIR})
            
            # Configure Python build
            execute_process(
                COMMAND ${PYTHON_SOURCE_DIR}/configure --prefix=${PYTHON_BUILD_DIR} --enable-shared
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_CONFIGURE_RESULT
            )
            
            if(NOT PYTHON_CONFIGURE_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to configure Python build. Error code: ${PYTHON_CONFIGURE_RESULT}")
            endif()
            
            # Build Python
            execute_process(
                COMMAND make -j4
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_BUILD_RESULT
            )
            
            if(NOT PYTHON_BUILD_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to build Python. Error code: ${PYTHON_BUILD_RESULT}")
            endif()
            
            # Install Python to the build directory
            execute_process(
                COMMAND make install
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_INSTALL_RESULT
            )
            
            if(NOT PYTHON_INSTALL_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to install Python. Error code: ${PYTHON_INSTALL_RESULT}")
            endif()
            
            message(STATUS "Python built successfully from source on Linux")
            
            # Set Python include directories for Linux
            target_include_directories(python_lib INTERFACE 
                ${PYTHON_BUILD_DIR}/include/python3.10
            )
            
            # Link with built Python libraries
            target_link_directories(python_lib INTERFACE ${PYTHON_BUILD_DIR}/lib)
            target_link_libraries(python_lib INTERFACE python3.10)
        else()
            # macOS specific settings
            # Configure and build Python on macOS
            message(STATUS "Building Python from source on macOS...")
            
            # Create a build directory for Python
            set(PYTHON_BUILD_DIR ${CMAKE_BINARY_DIR}/_deps/python-build)
            file(MAKE_DIRECTORY ${PYTHON_BUILD_DIR})
            
            # Configure Python build
            execute_process(
                COMMAND ${PYTHON_SOURCE_DIR}/configure --prefix=${PYTHON_BUILD_DIR} --enable-shared --enable-framework
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_CONFIGURE_RESULT
            )
            
            if(NOT PYTHON_CONFIGURE_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to configure Python build. Error code: ${PYTHON_CONFIGURE_RESULT}")
            endif()
            
            # Build Python
            execute_process(
                COMMAND make -j4
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_BUILD_RESULT
            )
            
            if(NOT PYTHON_BUILD_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to build Python. Error code: ${PYTHON_BUILD_RESULT}")
            endif()
            
            # Install Python to the build directory
            execute_process(
                COMMAND make install
                WORKING_DIRECTORY ${PYTHON_BUILD_DIR}
                RESULT_VARIABLE PYTHON_INSTALL_RESULT
            )
            
            if(NOT PYTHON_INSTALL_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to install Python. Error code: ${PYTHON_INSTALL_RESULT}")
            endif()
            
            message(STATUS "Python built successfully from source on macOS")
            
            # Set Python include directories for macOS
            target_include_directories(python_lib INTERFACE 
                ${PYTHON_BUILD_DIR}/include/python3.10
            )
            
            # Link with built Python libraries
            target_link_directories(python_lib INTERFACE ${PYTHON_BUILD_DIR}/lib)
            target_link_libraries(python_lib INTERFACE python3.10)
        endif()
    endif()
endif()

# Set Python variables for use in main CMakeLists.txt
if(BUILD_PYTHON_FROM_SOURCE)
    if(WIN32)
        set(PYTHON_INCLUDE_DIRS 
            ${PYTHON_SOURCE_DIR}/Include
            ${PYTHON_SOURCE_DIR}/PC
        )
    else()
        set(PYTHON_INCLUDE_DIRS ${PYTHON_BUILD_DIR}/include/python3.10)
    endif()
    
    message(STATUS "Python include directories set to: ${PYTHON_INCLUDE_DIRS}")
endif()

# Always set PYTHON_LIBRARIES to python_lib
set(PYTHON_LIBRARIES python_lib)
message(STATUS "Python libraries set to: ${PYTHON_LIBRARIES}")

# Option to build pybind11 from source
option(BUILD_PYBIND11_FROM_SOURCE "Build pybind11 from source instead of using system pybind11" ON)

message(STATUS "pybind11 configuration: BUILD_PYBIND11_FROM_SOURCE=${BUILD_PYBIND11_FROM_SOURCE}")

if(BUILD_PYBIND11_FROM_SOURCE)
    message(STATUS "Downloading pybind11 source code from GitHub...")
    # Download and build pybind11 from GitHub
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.11.1
        GIT_SHALLOW TRUE
    )

    # Make pybind11 available
    FetchContent_MakeAvailable(pybind11)
    
    if(NOT pybind11_POPULATED)
        message(FATAL_ERROR "Failed to download pybind11 source code. Please check your internet connection.")
    endif()
    
    # Set pybind11 source directory
    set(PYBIND11_SOURCE_DIR ${pybind11_SOURCE_DIR})

    # Check if PYBIND11_SOURCE_DIR exists, if not use the FetchContent path
    if(NOT EXISTS ${PYBIND11_SOURCE_DIR})
        set(PYBIND11_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/pybind11-src)
    endif()
    
    if(NOT EXISTS ${PYBIND11_SOURCE_DIR})
        message(FATAL_ERROR "pybind11 source directory not found at ${PYBIND11_SOURCE_DIR}")
    endif()
    
    # Configure pybind11 build
    set(PYBIND11_TEST OFF CACHE BOOL "Build pybind11 tests" FORCE)
    
    message(STATUS "pybind11 source code downloaded successfully to ${PYBIND11_SOURCE_DIR}")
else()
    # Try to find system pybind11
    find_package(pybind11 QUIET)
    if(NOT pybind11_FOUND)
        message(WARNING "System pybind11 not found. Falling back to downloading from source.")
        # Download and build pybind11 from GitHub
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v2.11.1
            GIT_SHALLOW TRUE
        )

        # Make pybind11 available
        FetchContent_MakeAvailable(pybind11)
        
        # Set pybind11 source directory
        set(PYBIND11_SOURCE_DIR ${pybind11_SOURCE_DIR})

        # Check if PYBIND11_SOURCE_DIR exists, if not use the FetchContent path
        if(NOT EXISTS ${PYBIND11_SOURCE_DIR})
            set(PYBIND11_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/pybind11-src)
        endif()
        
        # Configure pybind11 build
        set(PYBIND11_TEST OFF CACHE BOOL "Build pybind11 tests" FORCE)
    else()
        message(STATUS "Using system pybind11")
        # Get pybind11 include directory from the package
        set(PYBIND11_SOURCE_DIR ${pybind11_INCLUDE_DIRS})
    endif()
endif()

# Final configuration summary
message(STATUS "Python and pybind11 configuration complete")
message(STATUS "Python source directory: ${PYTHON_SOURCE_DIR}")
message(STATUS "Python include directories: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "pybind11 source directory: ${PYBIND11_SOURCE_DIR}")
