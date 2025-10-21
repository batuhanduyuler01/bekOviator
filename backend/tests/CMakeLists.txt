cmake_minimum_required(VERSION 3.16)
project(CrashGameTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Google Test'i indir ve kur
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)

# Windows'ta gMock'u devre dışı bırak
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Ana proje kaynak dosyaları (main.cpp hariç)
set(GAME_SOURCES
    ../src/game.cpp
    ../src/player.cpp
    ../src/bet.cpp
    ../src/server.cpp
)

# Test dosyaları
set(TEST_SOURCES
    test_main.cpp
    test_player.cpp
    test_bet.cpp
    test_game.cpp
)

# Include directories
include_directories(../include)

# Test executable oluştur
add_executable(crash_tests ${TEST_SOURCES} ${GAME_SOURCES})

# Google Test'i link et
target_link_libraries(crash_tests 
    gtest 
    gtest_main
    pthread
)

# Compiler flags
target_compile_options(crash_tests PRIVATE -Wall -Wextra)

# Test'leri otomatik çalıştır
enable_testing()
add_test(NAME crash_game_tests COMMAND crash_tests)