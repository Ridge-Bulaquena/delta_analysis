#~root/CMakeLists.txt
cmake_minimum_required (VERSION 3.10)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
project ("delta_analysis")
find_package(Boost REQUIRED) 
#make libraries
add_library(delta_core INTERFACE)

# Указываем, где лежат ваши .h файлы (корень папки)
target_include_directories(delta_core INTERFACE 
    "${CMAKE_SOURCE_DIR}/include"
)
target_link_libraries(delta_core INTERFACE Boost::boost)

if(MSVC)
    target_compile_options(delta_core INTERFACE /EHsc)
    add_compile_options(/Zc:__cplusplus)
endif()

# Включите подпроекты.
add_subdirectory ("src")
add_subdirectory ("tests/basic")
add_subdirectory("benchmarks")
