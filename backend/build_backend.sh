#!/bin/bash

# Define project directories
BUILD_DIR="build"

# Clean the build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create the build directory
echo "Creating build directory..."
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake to configure the project
echo "Configuring the project with CMake..."
cmake ..

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install shared libraries (if applicable) and update the library cache
if [ "$(id -u)" -ne 0 ]; then
    echo "You must run this script as root to update ldconfig."
else
    echo "Updating shared library cache..."
    ldconfig
fi

# Return to the project root
cd ..

echo "Build completed successfully!"
