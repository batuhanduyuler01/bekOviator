cmake_minimum_required(VERSION 3.16)
project(CrashGameServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
    set(PISTACHE_LIBRARY /opt/homebrew/lib/libpistache.dylib)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")  # Ubuntu/Linux
    # Pistache and json libraries should be installed via apt
    # libpistache-dev nlohmann-json3-dev
    set(PISTACHE_LIBRARY pistache)
endif()

# Include directories
include_directories(include)

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

# Link libraries
target_link_libraries(crash_server ${PISTACHE_LIBRARY} pthread)

# Compiler flags
target_compile_options(crash_server PRIVATE -Wall -Wextra)