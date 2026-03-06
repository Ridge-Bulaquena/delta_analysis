// tests/calculus/main_tests_calculus.cpp
#include <gtest/gtest.h>
#include <omp.h>
#include <iostream>

int main(int argc, char** argv) {
#pragma omp parallel
    {
#pragma omp master
        std::cout << "[OpenMP] Warmup. Total threads: " << omp_get_num_threads() << std::endl;
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}