#benchmarks/CMakeLists.txt
find_package(benchmark REQUIRED)

add_executable(advance_benchmark advance_benchmark.cpp)
target_link_libraries(advance_benchmark PRIVATE benchmark::benchmark delta_core)
target_include_directories(advance_benchmark PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(advance_benchmark PRIVATE /Zc:__cplusplus)

add_executable(opfunc_benchmark operational_function_benchmark.cpp)
target_link_libraries(opfunc_benchmark PRIVATE benchmark::benchmark delta_core)
target_include_directories(opfunc_benchmark PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(opfunc_benchmark PRIVATE /Zc:__cplusplus)

add_executable(riemann_sum_benchmark riemann_sum_benchmark.cpp)
target_link_libraries(riemann_sum_benchmark PRIVATE benchmark::benchmark delta_core)
target_include_directories(riemann_sum_benchmark PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(riemann_sum_benchmark PRIVATE /Zc:__cplusplus)
