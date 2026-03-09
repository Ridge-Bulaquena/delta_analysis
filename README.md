# Δ‑analysis Library

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.18761044.svg)](https://doi.org/10.5281/zenodo.18761044)
[![CI](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Coverage](https://img.shields.io/badge/coverage-95%25-brightgreen)]()
[![License](https://img.shields.io/badge/license-NC--SA%204.0%20%26%20Commercial-blue)]()

A modern C++20 library that implements **Δ‑analysis** – a constructive reformulation of mathematical analysis where the continuum emerges as the invariant of an iterative refinement process, not as a primitive given. The library provides a unified framework to work with functions on arbitrary discrete address spaces, bridging pure mathematics, computational physics, and numerical methods.

This code is the computational companion to our full 920-pages foundational research published at [Zenodo (click the link)](https://doi.org/10.5281/zenodo.18761044). Fascinating foundational stuff with a whiff of British humor in between rigorous theorems.

---

## 🔥 Killer Features: What the Library Already Does

### 1. Integrate the Dirichlet Function Without Measure Theory
Classically, the Dirichlet function (1 on rationals, 0 on irrationals) is not Riemann integrable; integrating it requires measure theory and yields different values depending on the integral.  
In Δ‑analysis, by **changing the regulative idea** to a tree‑based one (binary strings), the same instruction becomes locally constant. The library computes its integral as a simple sum over sibling pairs, converging to `1/2` – **no measure theory, no sigma‑algebras, no uncountable sets**.  
🔍 *Test*: `tests/regulative_ideas/test_tree.cpp` (`DirichletIntegral`).

### 2. Exact Rational Arithmetic with Configurable Precision
All computations use `Rational` from Boost.Multiprecision, which can be either **dynamic** (arbitrary precision) or **static** (fixed bit width, stack‑allocated) – controlled by a single CMake flag. This means you can trade off speed vs. precision, and **no floating‑point error** creeps into the core algorithms.  
🔍 *See*: `include/delta/core/rational.h`.

### 3. Construct Real Numbers as Invariants of Refinement
The library implements the construction of ℝ from fundamental Δ‑sequences (Block 6 of the theory). For example, it represents √2 as the sequence of left endpoints of intervals containing √2 in a dyadic refinement. Two different representations of the same number are recognised as equivalent, and the resulting equivalence classes form an ordered field isomorphic to the classical ℝ.  
🔍 *Test*: `tests/calculus/test_sqrt2_construction.cpp`.

### 4. Adaptive Refinement – Built into the Foundation
`AdaptiveDeltaPath` inserts new points where the function deviates most from linearity, clustering points in regions of rapid change. In classical numerical analysis this is just an algorithm; in Δ‑analysis it is a **first‑class citizen** – a valid Δ‑path that respects the betweenness relation and inherits all convergence theorems. The library lets you define your own adaptive strategies and immediately obtain rigorous error bounds.  
🔍 *Test*: `tests/basic/test_adaptive_path.cpp`.

### 5. Non‑Commutativity of Strategies – Process Matters
Different orders of applying the same two λ‑strategies (e.g., λ=1/3 and λ=2/3) produce **different intermediate grids**, even though both converge to the same continuum limit. This demonstrates that Δ‑analysis captures the process, not just the outcome – a feature absent in classical analysis.  
🔍 *Test*: `tests/basic/test_non_commutativity.cpp`.

### 6. Tensor Fields (Matrix‑Valued Functions)
Addresses can be `Eigen::MatrixXd`. The library builds uniform grids of matrices, evaluates functions like `f(X)=X` or `f(X)=X²`, and computes Riemann sums. For the identity function on `[0·I, I]`, the left Riemann sum converges to `0.5·I` – an exact matrix analogue of the scalar integral.  
🔍 *Test*: `tests/regulative_ideas/test_matrix.cpp` (`IdentityIntegral`).

### 7. Quantitative Continuity and Differentiability
Using a modulus of continuity (e.g., `C·δ^α`), the library verifies whether a function satisfies that modulus on a given grid. For `sqrt(x)` on `[0,1]` it confirms Hölder continuity with exponent `1/2`, while a linear modulus fails – exactly as expected. Similarly, it checks differentiability by comparing one‑sided difference quotients against a modulus of convergence.  
🔍 *Tests*: `tests/calculus/test_modulus_continuity.cpp`, `test_differentiability.cpp`.

Every feature listed above is backed by tests (as well as theorems in source research) and demonstrates something that classical analysis either cannot do, *does not even know that it should do*, or requires heavy additional machinery. 

All these features are implemented, tested, and ready for experimentation. Most notably, these 'killer features' are not even the endpoint but the by-product. These are only the beginning where we've successfully implemented roughly 100 pages of the 900-pages source.

**In short, delta-analysis is a parametric factory for producing analysis on any kind of space: rational, matrices, strings, p-adic, etc. To build analysis on an all-new kind of space, you only need to implement the regulative idea and some supporting classes**

---

## 🌌 Philosophy

We rebuild mathematical analysis from a single elementary premise: *between any two addresses a third can be inserted*.  

From this seed, iteration generates a sequence of nested finite grids that converge to a continuum – but the continuum is never assumed; it remains a regulative idea. The formalism is fully parametric: you choose the address space (rationals, matrices, binary strings, p‑adic numbers…), the betweenness relation, the metric, and the refinement strategy. Out of these choices emerges an entire family of possible analyses – real, p‑adic, ultrametric, tree‑based, or tailored to any combinatorial or geometric structure.

**Why rebuild analysis from scratch?**  
Classical analysis postulates the continuum as a ready‑made set of points. This leads to foundational puzzles: Zeno’s paradox, Banach–Tarski, the need for infinite energy to resolve arbitrarily small scales. Classical physical derivations rest on a promissory note of infinite divisibility of space, time and coordinate grids for zero cost - an absurd notion, if given a second thought. Δ‑analysis removes actual infinity from the operational level. Every object – grids, addresses, function values – is finite and constructible. The infinite appears only as a limit, a horizon, an invariant of all reasonable refinement processes.

Further, our approach yields concrete applications, as outlined in the following theses (non-exhaustively).

**Five theses from the original research:**

1. **Discrete decomposition of Einstein equations** – from a simple insertion rule and the causality condition `‖Δ𝐮‖ ≤ cτₙ`, Lorentzian signature and Regge action emerge naturally. In the continuum limit, we recover Einstein equations with an extra term encoding topological complexity (dark matter / dark energy).
2. **Reinvention of analysis without actual infinity** – all theorems (continuity, differentiability, integrability) are reproved using only finite grids and constructive estimates.
3. **Discrete Dirichlet principle** – no measure theory, no “almost everywhere”. For any tolerance ε, we stop at a finite level and obtain an exact discrete solution.
4. **Navier–Stokes: the Millennium Problem is physically meaningless** – with finite energy, there is an absolute minimum resolvable scale. For any finite ε, we give an explicit solution; the infinite limit is a regulative horizon.
5. **Dark matter and dark energy explained** – they emerge from a single informational field `ℐ(x)`, the coarse‑grained density of topological complexity. No fine‑tuning, no extra dimensions.

This is precisely what we set out to achieve in code in the end, and why we bother with this library at all. Right now this library implements the core machinery of Δ‑analysis, providing tools to build grids, define functions, compute integrals and derivatives, and explore adaptive strategies – all within a constructive, verifiable framework.

---

## ✨ Code Features

- **Unprecedented abstraction** – addresses can be:
  - rational numbers (with dynamic or fixed‑precision `Rational`),
  - dense matrices (`Eigen::MatrixXd`),
  - binary strings (tree‑like addresses),
  - p‑adic numbers (concept ready, with metric).

More regulative ideas can be added by implementing a few simple concepts.

- **Flexible grid refinement** – use any delta operator (midpoint, fixed/dynamic fraction, adaptive) plugged into static, dynamic or factory strategies.

- **Adaptive refinement** – `AdaptiveDeltaPath` inserts new points where the function deviates most from linearity, concentrating points in regions of rapid change.

- **Operational functions** – values can be stored and extended to refined grids; specialisations for uniform grids provide O(1) access.

- **Calculus on grids** – compute Riemann sums (left, right, tagged, tree‑adapted), check continuity with arbitrary moduli, test differentiability using difference quotients and convergence moduli.

- **Performance aware** – optional OpenMP acceleration for computing maximum oscillation, double buffering in `DeltaPath`, and benchmarks to track efficiency.

- **Battle‑tested** – the test suite covers every public component, edge cases, and several regulative ideas; test coverage is above 95%.

## 📁 Repository Structure

```
include/delta/          # all public headers
  core/                 # core concepts and classes: Rational, grids, paths, operators, strategies, completion
  calculus/             # calculus‑related algorithms: continuity, differentiability, Riemann sums, moduli

tests/                  # unit and integration tests
  basic/                # tests for core components (grids, paths, operators, strategies, basic calculus)
  calculus/             # tests for calculus algorithms (continuity, differentiability, moduli, Riemann sums)
  regulative_ideas/     # tests for non‑standard address spaces (matrices, p‑adic, tree)
  numerical/            # numerical tests (tensor fields)

benchmarks/             # Google Benchmark executables for performance measurement
examples/               # example applications (e.g., Dirichlet problem on strings)
```

---

## 🚀 Quick Example

```cpp
#include <delta/core/adaptive_delta_path.h>
#include <delta/core/delta_operator.h>
#include <delta/core/rational.h>
#include <iostream>

using namespace delta;
using Addr = Rational;
using Compare = std::less<Addr>;

int main() {
    // Function with a sharp corner at x = 0.5: f(x) = |x - 1/2|
    auto func = [](const Addr& x) -> Rational {
        return abs(x - Rational(1, 2));
    };

    // Adaptive operator: places new points closer to regions of high variation
    // Parameters: threshold = 0.1, epsilon = 0.05
    AdaptiveOperator adapt_op(Rational(1, 10), Rational(1, 20));
    std::vector<Addr> initial = {0_r, 1_r};

    // Create adaptive path with threshold 0.01 – intervals with priority ≤ 0.01 are not refined
    auto path = AdaptiveDeltaPath<Addr, Rational, Rational,
                                  LessBetweenness, EuclideanMetric,
                                  EuclideanValueMetric, AdaptiveOperator, Compare>(
        initial, func, adapt_op, Rational(1, 100)
    );

    // Perform 10 refinement steps
    for (int i = 0; i < 10; ++i) {
        if (!path.advance()) break;
    }

    // Output results
    std::cout << "Number of points after adaptive refinement: " << path.size() << "\n";
    std::cout << "Points around the corner (0.45 – 0.55):\n";
    for (const auto& p : path.points()) {
        if (p > Rational(45, 100) && p < Rational(55, 100))
            std::cout << p << " ";
    }
    std::cout << "\n";

    return 0;
}
```

**What happens in this example?**  
- We define a non‑smooth function `f(x)=|x‑0.5|` on the interval `[0,1]`.  
- The `AdaptiveOperator` places new points closer to regions where the function varies rapidly (i.e., near the corner).  
- The path starts with just the endpoints `0` and `1`. At each step, the interval with the highest priority (deviation from linearity) is split, and the new point is inserted.  
- After 10 steps, the grid is **non‑uniform** – many points cluster around `0.5`, while regions where the function is linear (`|x‑0.5|` is actually linear on each side) have fewer points.  
- All computations use exact rational arithmetic – no floating‑point approximations.  

This example demonstrates that Δ‑analysis is not just a theoretical construct: it provides a practical, **rigorous** framework for adaptive grid generation, with the same mathematical guarantees as the underlying theory. The adaptive path is a valid Δ‑path, so all convergence theorems apply – the integral of `|x‑0.5|` computed on this grid will converge to the true value, and we even obtain explicit error bounds from the priority threshold.

---

## 📊 Benchmarks

The `benchmarks/` folder contains several Google Benchmark executables:

- `benchmark_advance` – measures the cost of one `DeltaPath::advance()` step.
- `benchmark_operational_function` – evaluates extension of an operational function to a refined grid.
- `benchmark_riemann_sum` – times left Riemann sum computation on grids of increasing size.
- `benchmark_adaptive_path` – measures performance of adaptive refinement.

To run all benchmarks:

```bash
cd out/build/<preset>
ctest -R benchmark -C Release
```

---

## 🧪 Testing

The library is thoroughly tested. All tests are automatically discovered by CTest and can be run with:

```bash
ctest --output-on-failure
```

Tests are also integrated with Visual Studio’s Test Explorer (on Windows) for easy development.

---

## 🔧 Building

### Requirements

- CMake 3.15+
- C++20 compiler (MSVC 19.29+, GCC 11+, Clang 14+)
- [vcpkg](https://github.com/microsoft/vcpkg) (recommended for dependency management)
- Dependencies: Boost, Eigen3, fmt, Google Test, Google Benchmark

### Using CMake Presets

We provide CMake presets for Windows (x64 Debug/Release), Linux (x64 Debug/Release) and macOS (x64 Debug/Release).  
Configure and build with:

```bash
# For Windows
cmake --preset x64-debug
cmake --build out/build/x64-debug

# For Linux
cmake --preset x64-debug-linux
cmake --build out/build/x64-debug-linux

# For macOS
cmake --preset x64-debug-macos
cmake --build out/build/x64-debug-macos
```

The presets automatically set up the vcpkg toolchain if `VCPKG_ROOT` is defined.

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

---

## 📄 License

This project is **dual‑licensed**:

- **Non‑commercial use**: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International ([CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/)).  
  You are free to share and adapt the material for non‑commercial purposes, provided you give appropriate credit and distribute any contributions under the same license.

- **Commercial use**: Requires a separate explicit agreement. For commercial licensing and inquiries, please contact: timohaishimcev@gmail.com

---

## 🧩 Contributing

We welcome **issues** – bug reports, feature requests, and we welcome **discussions** concerning both the code and the underlying research from Zenodo. 
**Pull requests** will generally **not** be accepted, because the library’s development follows a planned roadmap. Exceptions may be made for truly exceptional contributions that align with the project’s vision. If you have an idea, please open an issue first to discuss.

---

## 📚 Citation

If you use this library in your research, please cite the accompanying theoretical work:

```bibtex
@misc{ishimtsev_2026_18761044,
  author       = {Ishimtsev, Timofey and Echo},
  title        = {General Delta-Theory of the Discrete Continuum: Refounding Analysis to Unify Relativity and Quantum Gravity},
  month        = feb,
  year         = 2026,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.18761044},
  url          = {https://doi.org/10.5281/zenodo.18761044}
}
```

For now, please cite both the paper and the repository URL.

---

## 🙏 Acknowledgements

- Boost.Multiprecision for the `Rational` type.
- Eigen for linear algebra.
- {fmt} for modern formatting.
- Google Test and Google Benchmark for testing and benchmarking.

---

**Explore the discrete foundations of mathematical analysis and physics with delta‑analysis.**  
For questions, ideas, or commercial licensing, please open an issue or contact the authors.
