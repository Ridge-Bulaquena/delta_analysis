# tests/numerical/CMakeLists.txt

# 1. Список файлов для численных тестов
add_executable(delta_tests_numerical
    main_tests_numerical.cpp
    test_tensor_field.cpp
)

# 2. Линковка
# Все зависимости (Eigen, OpenMP, fmt) прилетают из delta_core автоматически
target_link_libraries(delta_tests_numerical
    PRIVATE
    delta_core
    gtest_main
)

# 3. Регистрация тестов для IDE
include(GoogleTest)

# Используем MSVC_BIN_DIR из корня, чтобы тесты не падали при поиске рантайм-библиотек
gtest_discover_tests(delta_tests_numerical
    PROPERTIES 
    ENVIRONMENT "PATH=${MSVC_BIN_DIR};$ENV{PATH}"
)
