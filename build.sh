#!/bin/bash

# Plugin System Build Script
echo "==================================="
echo "    Plugin System Build Script"
echo "==================================="

# Set default values
BUILD_DIR="build"
BUILD_TYPE="Debug"
BUILD_SHARED="ON"
BUILD_EXAMPLES="ON"
BUILD_TESTS="ON"
GENERATOR="Unix Makefiles"
FORCE_CONFIG="OFF"

# Display help information
show_help() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --help, -h          Display this help information"
    echo "  --build-dir DIR     Set build directory (default: build)"
    echo "  --build-type TYPE   Set build type (default: Debug)"
    echo "                      Options: Debug, Release, RelWithDebInfo, MinSizeRel"
    echo "  --shared ON/OFF     Whether to build shared libraries (default: ON)"
    echo "  --examples ON/OFF   Whether to build examples (default: ON)"
    echo "  --tests ON/OFF      Whether to build tests (default: OFF)"
    echo "  --generator GEN     Set CMake generator (default: \"Unix Makefiles\")"
    echo "  --force, -f         Force CMake reconfiguration even if build directory exists"
    echo ""
    echo "Examples:"
    echo "  $0 --build-type Release --shared OFF"
    echo "  $0 --build-dir build-ninja --generator Ninja"
    echo "  $0 --force"
    echo ""
    exit 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            ;;
        --force|-f)
            FORCE_CONFIG="ON"
            shift
            continue
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift
            ;;
        --shared)
            BUILD_SHARED="$2"
            shift
            ;;
        --examples)
            BUILD_EXAMPLES="$2"
            shift
            ;;
        --tests)
            BUILD_TESTS="$2"
            shift
            ;;
        --generator)
            GENERATOR="$2"
            shift
            ;;
        *)
            echo "Unknown parameter: $1"
            exit 1
            ;;
    esac
    shift
done

# Display configuration information
echo ""
echo "Build configuration:"
echo "  Build directory: $BUILD_DIR"
echo "  Build type: $BUILD_TYPE"
echo "  Shared libraries: $BUILD_SHARED"
echo "  Build examples: $BUILD_EXAMPLES"
echo "  Build tests: $BUILD_TESTS"
echo "  Generator: $GENERATOR"
echo ""

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found. Please install CMake and ensure it's in your PATH."
    exit 1
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    FORCE_CONFIG="ON"
fi

# Configure project
if [ "$FORCE_CONFIG" = "ON" ]; then
    echo "Configuring project..."
    cd "$BUILD_DIR"
    cmake .. \
        -G "$GENERATOR" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DBUILD_SHARED_LIBS=$BUILD_SHARED \
        -DBUILD_EXAMPLES=$BUILD_EXAMPLES \
        -DBUILD_TESTS=$BUILD_TESTS

    if [ $? -ne 0 ]; then
        echo "Error: CMake configuration failed."
        exit 1
    fi
else
    echo "Build directory already exists, skipping configuration step."
    echo "Use --force or -f to force reconfiguration."
    cd "$BUILD_DIR"
fi

# Build project
echo "Building project..."
cmake --build . --config $BUILD_TYPE -j

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

echo ""
echo "Build successful!"
echo "Executables are located at: $BUILD_DIR/bin"