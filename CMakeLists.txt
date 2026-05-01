# Root CMakeLists.txt for delta_analysis project
cmake_minimum_required(VERSION 3.15)   # Require CMake 3.15+ for proper FetchContent

# 1. MSVC debug information format policy (CMP0141)
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
endif()

project("delta_analysis" VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(DELTA_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(DELTA_ENABLE_SANITIZERS "Enable ASAN/UBSAN in Debug builds for non-MSVC" OFF)

# 2. OpenMP configuration (LLVM backend for MSVC)
if(MSVC)
    # Use the new /openmp:llvm flag (requires MSVC 19.29+ and LLVM tools)
    set(OpenMP_CXX_FLAGS "/openmp:llvm")
    
    if(NOT TARGET OpenMP::OpenMP_CXX)
        # Manually create an imported target for OpenMP with LLVM backend
        add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
        target_compile_options(OpenMP::OpenMP_CXX INTERFACE /openmp:llvm)
        # Link against libomp.lib; if linker error LNK1104 occurs,
        # add the LLVM library path via link_directories().
        target_link_libraries(OpenMP::OpenMP_CXX INTERFACE libomp.lib)
        target_compile_definitions(OpenMP::OpenMP_CXX INTERFACE _OPENMP_NO_LLVM_HELPER)
    endif()
else()
    # For non-MSVC compilers, use the standard FindOpenMP module.
    find_package(OpenMP REQUIRED)
endif()


# 3. External dependencies (centralised)
find_package(Boost REQUIRED)          # Boost libraries (headers mostly)
find_package(Eigen3 REQUIRED)         # Eigen3 for linear algebra
find_package(fmt CONFIG REQUIRED)     # fmtlib for formatting

# 4. Google Test (included once for the whole project)
include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)   # Ensure consistent CRT with MSVC
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)           # Do not install GTest

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2               # Use a specific release tag
  GIT_SHALLOW    TRUE                   # Shallow clone to save bandwidth/time
)
FetchContent_MakeAvailable(googletest)

# 5. Core interface library (delta_core)
add_library(delta_core INTERFACE)
target_include_directories(delta_core INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(delta_core INTERFACE 
    Boost::boost 
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
    fmt::fmt
)

if(MSVC)
    # MSVC‑specific compile options: enable C++ exceptions, set __cplusplus correctly.
    target_compile_options(delta_core INTERFACE /EHsc /Zc:__cplusplus)
    # Define EIGEN_HAS_OPENMP to let Eigen use OpenMP.
    target_compile_definitions(delta_core INTERFACE EIGEN_HAS_OPENMP)
elseif(DELTA_ENABLE_WARNINGS)
    target_compile_options(delta_core INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(DELTA_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    target_compile_options(delta_core INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(delta_core INTERFACE -fsanitize=address,undefined)
endif()

# Option to select static vs dynamic rational backend (see delta/core/rational.h)
option(DELTA_USE_STATIC_RATIONAL "Use static rational with fixed bit width" OFF)
if(DELTA_USE_STATIC_RATIONAL)
    target_compile_definitions(delta_core INTERFACE DELTA_RATIONAL_BITS=256)
endif()

# Helper variable: directory containing the MSVC compiler (used for DLL discovery in tests)
get_filename_component(MSVC_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY CACHE INTERNAL "")

# Add subdirectories for tests, examples, and benchmarks
add_subdirectory("tests/basic")
add_subdirectory("tests/calculus")
add_subdirectory("tests/regulative_ideas")
add_subdirectory("tests/numerical")
add_subdirectory("examples/arbitrary_regulative_ideas")
add_subdirectory("benchmarks")