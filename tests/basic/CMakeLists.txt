# tests/basic/CMakeLists.txt

# 1. Создаем исполняемый файл
add_executable(delta_tests
    test_fixtures.h
    main_tests_basic.cpp
    test_grid.cpp
    test_grid_concepts.cpp
    test_grid_edge_cases.cpp
    test_operational_function.cpp
    test_operational_function_edge_cases.cpp
    test_delta_path.cpp
    test_adaptive_operator.cpp
    test_operators_edge_cases.cpp
    test_adaptive_path.cpp
    
    test_strategies_edge_cases.cpp
    test_integral.cpp
    test_sqrt2.cpp
    test_non_commutativity.cpp
    test_differentiability.cpp
)
target_include_directories(delta_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# 2. Привязываем всё необходимое
# delta_core уже содержит: OpenMP, Eigen, Boost, fmt и пути к include
target_link_libraries(delta_tests
    PRIVATE
    delta_core
    gtest_main   # gtest_main сам подтянет gtest, если используешь стандартный Google Test Main
)

# 3. Интеграция с Test Explorer в Visual Studio
include(GoogleTest)

# Прокидываем путь к DLL через PATH, иначе gtest_discover_tests не сможет запустить бинарник
# для поиска тестов и всё свалится с ошибкой в CMake
gtest_discover_tests(delta_tests
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)
# Оптимизация сборки
target_precompile_headers(delta_tests PRIVATE
    <gtest/gtest.h>
    <fmt/format.h>
    <vector>
    <functional>
    <iostream>
    <omp.h>
)