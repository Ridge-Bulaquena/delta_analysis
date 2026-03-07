// src.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#include <iostream>
#include <Eigen/Dense>
#include <string>
#include <fmt/core.h>
#include <iostream>

class MatrixSolver {
public:
    // Решает уравнение A * x = b
    static Eigen::VectorXd solveSystem(const Eigen::MatrixXd& A, const Eigen::VectorXd& b);

    // Красиво печатает матрицу через fmt
    static void printStatus(const std::string& message);
};
