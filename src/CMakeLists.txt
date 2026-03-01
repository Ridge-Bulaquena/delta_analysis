# Ищем либы (vcpkg их подсунет, т.к. ты указал тулчейн в пресетах)
find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Создаем экзешник. Имя "analysis_app", файлы "src.cpp" и "src.h"
add_executable(analysis_app "src.cpp" "src.h")

# Ставим стандарт 20
set_target_properties(analysis_app PROPERTIES CXX_STANDARD 20)

# Привязываем либы
target_link_libraries(analysis_app PRIVATE Eigen3::Eigen fmt::fmt)
