# tests/regulative_ideas/CMakeLists.txt
# CMake configuration for tests of non‑standard regulative ideas.
# This suite verifies that Δ‑analysis concepts work correctly with
# matrix addresses (Eigen::MatrixXd), p‑adic metrics, and tree‑structured
# addresses (binary strings).

# 1. Create the test executable for regulative ideas tests.
add_executable(delta_tests_regulative_ideas
    main_tests_regulative_ideas.cpp    # Test main with OpenMP initialisation
    test_tree.cpp                      # Tests for tree‑based regulative idea
    test_matrix.cpp                    # Tests for matrix‑valued addresses
    test_padic.cpp                      # Tests for p‑adic metric
)

# 2. Link against required libraries.
#    delta_core is an INTERFACE library that brings in Boost, Eigen, OpenMP, fmt,
#    and the necessary include directories. gtest_main provides the Google Test
#    entry point (and automatically pulls in gtest).
target_link_libraries(delta_tests_regulative_ideas
    PRIVATE
    delta_core
    gtest_main
)

# 3. Integration with Visual Studio Test Explorer and CTest.
include(GoogleTest)

# Discover all tests defined in the executable and register them with CTest.
# The ENVIRONMENT property ensures that the directory containing the MSVC compiler
# (and thus the OpenMP DLLs) is in PATH, preventing test discovery failures.
# MSVC_BIN_DIR is set in the root CMakeLists.txt.
gtest_discover_tests(delta_tests_regulative_ideas
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)