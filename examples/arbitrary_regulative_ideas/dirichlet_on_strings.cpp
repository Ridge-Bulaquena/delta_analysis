//examples/arbitrary_regulative_idea/dirichlet_on_strings.cpp
#include <iostream>
#include <iomanip>
#include <functional>
#include "delta/core/tree_grid.h"
#include "delta/core/delta_path.h"  // содержит tree::TreeDeltaPath
#include "delta/calculus/riemann_sum.h"
#include "delta/core/regulative_idea.h"

using namespace delta;
using namespace delta::calculus;
int main() {
    // Создаём путь для дерева
    TreeDeltaPath<double> path;

    // Операционная функция: значение = последний символ (0 для '0', 1 для '1', корень = 0)
    std::function<double(const std::string&)> func = [](const std::string& addr) -> double {
        if (addr.empty()) return 0.0;
        return (addr.back() == '0') ? 0.0 : 1.0;
        };

    std::cout << "Dirichlet function on binary strings:\n";
    std::cout << "Level\tIntegral\tWeighted sum\n";

    for (int level = 0; level <= 10; ++level) {
        double integral = tree_riemann_sum(path, func);
        std::cout << level << "\t" << std::fixed << std::setprecision(6) << integral << "\n";
        path.advance();
    }

    return 0;
}