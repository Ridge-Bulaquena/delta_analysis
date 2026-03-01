// src.cpp: определяет точку входа для приложения.
//

#include "src.h"

using namespace std;

void MatrixSolver::printStatus(const std::string& message) {
    fmt::print("[INFO] {}\n", message);
}

Eigen::VectorXd MatrixSolver::solveSystem(const Eigen::MatrixXd& A, const Eigen::VectorXd& b) {
    // Используем метод разложения (QR), он надежный для большинства матриц
    return A.colPivHouseholderQr().solve(b);
}

int main() {
    MatrixSolver::printStatus("Запуск расчёта фундамента...");

    // Создаем матрицу 3x3 (система из 3-х уравнений)
    Eigen::MatrixXd A(3, 3);
    A << 3, 2, -1,
        2, -2, 4,
        -1, 0.5, -1;

    // Вектор свободных членов
    Eigen::VectorXd b(3);
    b << 1, -2, 0;

    // Решаем
    Eigen::VectorXd x = MatrixSolver::solveSystem(A, b);

    // Вывод результатов
    fmt::print("\nМатрица коэффициентов A:\n");
    std::cout << A << "\n\n";

    fmt::print("Вектор результатов b: ");
    for (int i = 0; i < b.size(); ++i) fmt::print("{} ", b(i));

    fmt::print("\n\nРЕШЕНИЕ (x):\n");
    for (int i = 0; i < x.size(); ++i) {
        fmt::print("x[{}] = {:.4f}\n", i, x(i));
    }

    // Чтобы консоль не закрылась сразу
    fmt::print("\nНажми Enter, чтобы выйти...");
    std::cin.get();

    return 0;
}

