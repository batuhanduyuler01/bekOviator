cmake_minimum_required(VERSION 3.16)
project(CrashGameServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(/opt/homebrew/include)

# Source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/server.cpp
    src/player.cpp
    src/bet.cpp
    src/json_utils.cpp
)

# Create executable
add_executable(crash_server ${SOURCES})

# Link directories and libraries
link_directories(/opt/homebrew/lib)
# Pistache kütüphanesinin tam yolu
target_link_libraries(crash_server /opt/homebrew/lib/libpistache.dylib pthread)

# Compiler flags
target_compile_options(crash_server PRIVATE -Wall -Wextra)