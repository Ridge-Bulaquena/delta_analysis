//tests/basic/main_tests_basic.cpp
#include <gtest/gtest.h>
#include <omp.h>
#include <iostream>

int main(int argc, char** argv) {
    // ПРИНУДИТЕЛЬНАЯ инициализация OpenMP до запуска тестов
    // Это "прогревает" рантайм и предотвращает Access Violation
    // "Прогревочный" вызов: заставляем OMP создать пул потоков прямо сейчас
#pragma omp parallel
    {
#pragma omp master
        std::cout << "[OpenMP] Warmup. Total threads: " << omp_get_num_threads() << std::endl;
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}