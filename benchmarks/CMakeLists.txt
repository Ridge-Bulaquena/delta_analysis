# benchmarks/CMakeLists.txt
# Configuration for performance benchmarks of core Δ‑analysis components.
# Benchmarks use the Google Benchmark library and measure execution time
# of key operations: path advancement, operational function extension,
# Riemann sum computation, and adaptive path refinement.

# Locate the Google Benchmark package (required).
find_package(benchmark REQUIRED)

add_executable(benchmark_advance benchmark_advance.cpp)
target_link_libraries(benchmark_advance PRIVATE benchmark::benchmark delta_core)
target_include_directories(benchmark_advance PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(benchmark_advance PRIVATE /Zc:__cplusplus)

add_executable(benchmark_operational_function benchmark_operational_function.cpp)
target_link_libraries(benchmark_operational_function PRIVATE benchmark::benchmark delta_core)
target_include_directories(benchmark_operational_function PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(benchmark_operational_function PRIVATE /Zc:__cplusplus)

add_executable(benchmark_riemann_sum benchmark_riemann_sum.cpp)
target_link_libraries(benchmark_riemann_sum PRIVATE benchmark::benchmark delta_core)
target_include_directories(benchmark_riemann_sum PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(benchmark_riemann_sum PRIVATE /Zc:__cplusplus)

add_executable(benchmark_adaptive_path benchmark_adaptive_path.cpp)
target_link_libraries(benchmark_adaptive_path PRIVATE benchmark::benchmark delta_core)
target_include_directories(benchmark_adaptive_path PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(benchmark_adaptive_path PRIVATE /Zc:__cplusplus)