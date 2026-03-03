# tests/numerical/CMakeLists.txt
enable_testing()
find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(delta_tests_numerical
test_tensor_field.cpp)

set_target_properties(delta_tests_numerical PROPERTIES CXX_STANDARD 20)

target_link_libraries(delta_tests_numerical
    PRIVATE
    delta_core
    GTest::gtest
    GTest::gtest_main
    Eigen3::Eigen
 #   Boost::boost   Этого достаточно для всех header-only библиотек Boost
)

target_include_directories(delta_tests_numerical PRIVATE ${CMAKE_SOURCE_DIR}/include)
include(GoogleTest)
gtest_discover_tests(delta_tests_numerical)