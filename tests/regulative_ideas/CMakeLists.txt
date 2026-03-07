# tests/regulative_ideas/CMakeLists.txt

# 1. Список файлов
add_executable(delta_tests_regulative_ideas
    main_tests_regulative_ideas.cpp
    test_tree.cpp
 test_matrix.cpp
test_padic.cpp)

# 2. Линковка
# Все зависимости (Eigen, OpenMP, fmt) прилетают из delta_core автоматически
target_link_libraries(delta_tests_regulative_ideas
    PRIVATE
    delta_core
    gtest_main
)

# 3. Регистрация тестов для IDE
include(GoogleTest)

# Используем MSVC_BIN_DIR из корня, чтобы тесты не падали при поиске рантайм-библиотек
gtest_discover_tests(delta_tests_regulative_ideas
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)
