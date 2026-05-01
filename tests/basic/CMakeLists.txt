# tests/basic/CMakeLists.txt
# CMake configuration for the basic unit tests of the Δ‑analysis library.
# This executable tests core components: grids, paths, operators, strategies,
# Riemann sums, and basic calculus concepts.

# 1. Create the test executable.
#    List all source files required for the basic tests.
add_executable(delta_tests_basic
    "../test_fixtures.h"               # Shared test fixtures and utilities
    main_tests_basic.cpp                # Test main with OpenMP warm‑up
    test_grid.cpp                       # ListGrid tests
    test_grid_concepts.cpp               # GridConcept and refine_grid tests
    test_grid_edge_cases.cpp             # Edge cases for grids
    test_operational_function.cpp        # OperationalFunction basic tests
    test_operational_function_edge_cases.cpp # Edge cases for OperationalFunction
    test_delta_path.cpp                  # DeltaPath tests
    test_adaptive_operator.cpp           # AdaptiveOperator tests
    test_operators_edge_cases.cpp        # Edge cases for delta operators
    test_adaptive_path.cpp                # AdaptiveDeltaPath tests
    test_strategies_edge_cases.cpp        # Edge cases for strategies
    test_integral.cpp                     # Riemann sum (integration) tests
    test_sqrt2.cpp                        # √2 approximation test
    test_non_commutativity.cpp            # Non‑commutativity of operators
    test_differentiability.cpp            # Differentiability checks
    test_math_philosophy_analyzer.cpp     # Math/philosophy text analyzer checks
    test_study_math_analyzer.cpp          # Study math validation checks
    test_logical_consistency_analyzer.cpp # Formal logic consistency checks
    test_paper_validity_analyzer.cpp      # Ten-dimension paper validity analyzer
)

# Add the parent directory (tests/) to the include path so that "test_fixtures.h" can be included.
target_include_directories(delta_tests_basic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# 2. Link against required libraries.
#    delta_core is an INTERFACE library that brings in Boost, Eigen, OpenMP, fmt,
#    and the necessary include directories. gtest_main provides the Google Test
#    entry point (and automatically pulls in gtest).
target_link_libraries(delta_tests_basic
    PRIVATE
    delta_core
    gtest_main
)

if(MSVC)
    # Enable C++ exceptions for MSVC (required by Google Test).
    target_compile_options(delta_tests_basic PRIVATE /EHsc)
endif()

# 3. Integration with Visual Studio Test Explorer.
include(GoogleTest)

# Discover all tests defined in the executable and register them with CTest.
# The ENVIRONMENT property ensures that the directory containing the MSVC compiler
# (and thus the OpenMP DLLs) is in PATH, preventing test discovery failures.
gtest_discover_tests(delta_tests_basic
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)

# 4. Optimise build times with precompiled headers.
#    Commonly used headers are precompiled to speed up compilation.
target_precompile_headers(delta_tests_basic PRIVATE
    <gtest/gtest.h>
    <fmt/format.h>
    <vector>
    <functional>
    <iostream>
    <omp.h>
)