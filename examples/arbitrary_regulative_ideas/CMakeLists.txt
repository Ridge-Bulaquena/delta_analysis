# Ищем либы (vcpkg их подсунет, т.к. ты указал тулчейн в пресетах)
find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Создаем экзешник. Имя "delta_examples", файлы "src.cpp" и "src.h"
add_executable(delta_examples "src.cpp")

# Ставим стандарт 20
set_target_properties(delta_examples PROPERTIES CXX_STANDARD 20)
target_include_directories(delta_examples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Привязываем либы
target_link_libraries(delta_examples PRIVATE delta_core Eigen3::Eigen fmt::fmt)
