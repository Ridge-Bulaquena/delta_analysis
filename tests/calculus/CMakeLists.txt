# tests/calculus/CMakeLists.txt

add_executable(tests_calculus
    main_tests_calculus.cpp
    test_continuity.cpp
 "test_riemann_sum.cpp" "test_differentiability.cpp" "test_rational_embedding.cpp" "test_sqrt2_construction.cpp")

# Чтобы видеть test_fixtures.h из родительской папки tests
target_include_directories(tests_calculus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(tests_calculus
    PRIVATE
    delta_core
    gtest_main
)

if(MSVC)
    target_compile_options(tests_calculus PRIVATE /EHsc)
endif()

include(GoogleTest)
gtest_discover_tests(tests_calculus
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)

target_precompile_headers(tests_calculus PRIVATE
    <gtest/gtest.h>
    <fmt/format.h>
    <vector>
    <functional>
    <iostream>
    <omp.h>
)