#~root/CMakeLists.txt
cmake_minimum_required(VERSION 3.15) # Поднял до 3.15 для нормального FetchContent

# 1. Фикс отладки для VS
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
endif()

project("delta_analysis" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Настройка OpenMP:llvm
if(MSVC)
    # Пытаемся найти путь к libomp.lib (обычно в папке LLVM внутри VS)
    # Если не найдет, линкер может ругнуться, тогда укажи путь к LLVM вручную
    set(OpenMP_CXX_FLAGS "/openmp:llvm")
    
    if(NOT TARGET OpenMP::OpenMP_CXX)
        add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
        target_compile_options(OpenMP::OpenMP_CXX INTERFACE /openmp:llvm)
        # Линкуем импорт-библиотеку. 
        # Если будет LNK1104, добавь путь через link_directories
        target_link_libraries(OpenMP::OpenMP_CXX INTERFACE libomp.lib)
        target_compile_definitions(OpenMP::OpenMP_CXX INTERFACE _OPENMP_NO_LLVM_HELPER)
    endif()
else()
    find_package(OpenMP REQUIRED)
endif()


# 3. Внешние зависимости (Централизованно)
find_package(Boost REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(fmt CONFIG REQUIRED)

# 4. Google Test (ОДИН РАЗ НА ВЕСЬ ПРОЕКТ)
include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(googletest)

# 5. Ядро (delta_core)
add_library(delta_core INTERFACE)
target_include_directories(delta_core INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(delta_core INTERFACE 
    Boost::boost 
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
    fmt::fmt
)

if(MSVC)
    target_compile_options(delta_core INTERFACE /EHsc /Zc:__cplusplus)
    target_compile_definitions(delta_core INTERFACE EIGEN_HAS_OPENMP)
endif()

# Configuring memory use on delta/core/rational.h - static vs dynamic.
option(DELTA_USE_STATIC_RATIONAL "Use static rational with fixed bit width" OFF)
if(DELTA_USE_STATIC_RATIONAL)
    target_compile_definitions(delta_core INTERFACE DELTA_RATIONAL_BITS=256)
endif()

# 6. Подключаем поддиректории
# Важно: переменная для путей к DLL, если discovery падает
get_filename_component(MSVC_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY CACHE INTERNAL "")

add_subdirectory("src")
add_subdirectory("tests/basic")
add_subdirectory("tests/numerical")
