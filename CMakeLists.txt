cmake_minimum_required(VERSION 3.20)
project(GameOfLife VERSION 1.0 LANGUAGES CXX)

include(CTest) # enables ctest integration via BUILD_TESTING/enable_testing

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Using C++ standard: C++${CMAKE_CXX_STANDARD}")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/std:c++latest /W4 /WX)
    
    # Debug build flags for MSVC
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    
    # Release build flags for MSVC
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    
    # Enable AddressSanitizer for Debug builds
    message(STATUS "Sanitizers ENABLED - AddressSanitizer")
    set(MSVC_SANITIZER_FLAGS "/fsanitize=address")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
    
    message(STATUS "MSVC Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "MSVC Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")

# Ensure proper C++23 flags for GCC/Clang
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-std=c++23 -Werror)
    
    # Debug build flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -Wpedantic -fno-omit-frame-pointer")
    
    # Release build flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    
    # Enable sanitizers for Debug builds
    message(STATUS "Sanitizers ENABLED - AddressSanitizer and UndefinedBehaviorSanitizer")
    set(SANITIZER_FLAGS "-fsanitize=address -fsanitize=undefined -fsanitize=leak")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    
    message(STATUS "GCC/Clang Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "GCC/Clang Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# Restrict to Debug and Release only
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Find OpenGL globally so all subprojects can use OpenGL::GL
find_package(OpenGL REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLX REQUIRED glx)
endif()

# Add dependencies first
add_subdirectory(Dependencies)
add_subdirectory(GOLLoggingLib)
add_subdirectory(GOLAlgoLib)
add_subdirectory(GOLGraphicsLib)
add_subdirectory(GOLGuiLib)
add_subdirectory(GOLCoreLib)
add_subdirectory(GOLExecutable)
add_subdirectory(GOLTest)