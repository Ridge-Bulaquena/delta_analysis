# tests/calculus/CMakeLists.txt
# CMake configuration for the calculus unit tests.
# This suite verifies continuity, differentiability, modulus classes,
# Riemann sums, and the construction of real numbers via completion.

# Create the test executable for calculus tests.
add_executable(tests_calculus
    main_tests_calculus.cpp                # Test main with OpenMP warm‑up
    test_continuity.cpp                    # Continuity checks with moduli
    "test_riemann_sum.cpp"                 # Riemann sum tests (left, right, tagged, tree)
    "test_differentiability.cpp"           # Differentiability checks
    "test_rational_embedding.cpp"          # Rational → real number embedding
    "test_sqrt2_construction.cpp"          # √2 as a fundamental sequence
    "test_modulus_continuity.cpp"          # Detailed modulus‑continuity tests
    "test_modulus.cpp"                      # Modulus class unit tests
)

# Add the parent directory (tests/) to the include path so that "test_fixtures.h" can be included.
target_include_directories(tests_calculus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Link against the core library (delta_core) and Google Test main.
# delta_core brings in Boost, Eigen, OpenMP, fmt, and the necessary include directories.
target_link_libraries(tests_calculus
    PRIVATE
    delta_core
    gtest_main
)

if(MSVC)
    # Enable C++ exceptions for MSVC (required by Google Test).
    target_compile_options(tests_calculus PRIVATE /EHsc)
endif()

# Integrate with CTest and Visual Studio Test Explorer.
include(GoogleTest)

# Discover all tests defined in the executable and register them with CTest.
# The ENVIRONMENT property ensures that the directory containing the MSVC compiler
# (and thus the OpenMP DLLs) is in PATH, preventing test discovery failures.
gtest_discover_tests(tests_calculus
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)

# Optimise build times with precompiled headers.
# Commonly used headers are precompiled to speed up compilation.
target_precompile_headers(tests_calculus PRIVATE
    <gtest/gtest.h>
    <fmt/format.h>
    <vector>
    <functional>
    <iostream>
    <omp.h>
)