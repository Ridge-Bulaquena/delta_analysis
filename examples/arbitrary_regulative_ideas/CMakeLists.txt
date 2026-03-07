#examples/arbitrary_regulative_ideas/CMakeLists.txt
# Создаем экзешник.
add_executable(delta_examples 
dirichlet_on_strings.cpp)

# Ставим стандарт 20
set_target_properties(delta_examples PROPERTIES CXX_STANDARD 20)
target_include_directories(delta_examples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Привязываем либы
# delta_core уже содержит: OpenMP, Eigen, Boost, fmt и пути к include
target_link_libraries(delta_examples PRIVATE delta_core)
