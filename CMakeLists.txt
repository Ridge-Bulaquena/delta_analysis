#~root/CMakeLists.txt
cmake_minimum_required (VERSION 3.10)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("delta_analysis")

#make libraries
add_library(delta_core INTERFACE)

# Указываем, где лежат ваши .h файлы (корень папки)
target_include_directories(delta_core INTERFACE 
    "${CMAKE_SOURCE_DIR}/include"
)

if(MSVC)
    target_compile_options(delta_core INTERFACE /EHsc)
endif()

# Включите подпроекты.
add_subdirectory ("src")
add_subdirectory ("tests/basic")
