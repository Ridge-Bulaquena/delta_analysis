# tests/basic/CMakeLists.txt
enable_testing()
find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(boost_rational CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(delta_tests
    test_grid.cpp
    test_operational_function.cpp
    test_delta_path.cpp
    test_integral.cpp
    test_sqrt2.cpp
    test_non_commutativity.cpp
    test_differentiability.cpp
)

set_target_properties(delta_tests PROPERTIES CXX_STANDARD 20)

target_link_libraries(delta_tests
    PRIVATE
    delta_core
    GTest::gtest
    GTest::gtest_main
    # boost_throw_exception  # удаляем, так как это header-only
)

target_include_directories(delta_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(delta_tests)