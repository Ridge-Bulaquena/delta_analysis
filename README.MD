# Внутренний статус: Delta Analysis

Текущее состояние проекта на этапе "Заливка фундамента". Настроено окружение C++.

## 🛠 Окружение и Тулчейн
- **Компилятор:** MSVC (Visual Studio 2022) + C++20.
- **Сборка:** CMake + Ninja (через `CMakePresets.json`).
- **Управление пакетами:** `vcpkg` в режиме **Manifest Mode**.
  - Корень vcpkg: Системная переменная `$env:VCPKG_ROOT`.
  - Версии либ: Закреплены через `builtin-baseline` в `vcpkg.json`.

## 📦 Подключенные библиотеки
- **Eigen3:** Линейная алгебра, матрицы, решение систем уравнений (уже в `src.cpp`).
- **fmt:** Форматированный вывод в консоль (замена `std::cout`).

## 📁 Структура репозитория
- `.gitignore` - Тут всё понятно
- `README.md` - Бортовой журнал
- `vcpkg.json` — Список зависимостей (сейчас для примера Eigen, fmt).
- `CMakeLists.txt` — Верхний уровень (подключает `/src`).
- `CMakePresets.json` — Настройки билда (x64-Debug/Release), путь к тулчейну vcpkg.
- `src/` — Исходники:
  - `src/CMakeLists.txt` — Описание таргета `analysis_app`.
  - `src/src.h` / `src/src.cpp` — Тестовая реализация решения СЛАУ через Eigen.

## ✅ Что уже работает
1. Автоматическая установка либ при открытии проекта в VS.
2. Решение матричных уравнений $A * x = b$ через QR-разложение Eigen.(чисто тест)
3. Приватный пуш в репозиторий (GitHub настроен).

---
*Примечание: Если CMake тупит, первым делом удалять папку `out`.
Важно: НИКАКИХ БИЛДОВ ЧЕРЕЗ КОНСОЛЬ, НИКАКИХ УСТАНОВОК ПАКЕТОВ ЧЕРЕЗ КОНСОЛЬ. Иначе нахрен всё сломается.*

---

## 🗺 План работ: Пошаговая реализация

Мы движемся от ядра наружу. Сначала реализуем чистую математику из первой главы (Δ-Analysis Formalism) с минимальными зависимостями. Потом надстраиваем Advanced, потом Reverse.

### Этап 1: Ядро (Core) — Главы 0-7.5 (без внешних зависимостей, только STL + boost::rational)

**Цель:** Реализовать все фундаментальные математические конструкции.

#### Шаг 1.1: Рациональные числа и базовая арифметика
- ✅ **Подключить:** `boost::rational` (добавить в `vcpkg.json`)
- ✅ **Создать папку:** `include/delta/core/`
- [ ] **Создать файлы:**
  - `include/delta/core/rational.h` — алиас для `boost::rational<int64_t>` + кастомные литералы
  ✅ `include/delta/core/regulative_idea.h` — структуры `Betweenness`, `Metric`
  ✅`include/delta/core/grid.h` — шаблонный класс `Grid<T>` (упорядоченный вектор)
  ✅include/delta/core/delta_path.h` — класс `DeltaPath` с операторами
- [ ] **Тесты:** 
  - `tests/test_rational_arithmetic.cpp` — проверка базовых операций
  - `tests/test_grid_construction.cpp` — создание сетки, вставка элементов

#### Шаг 1.2: Операторные функции и непрерывность
- [ ] **Создать папку:** `include/delta/calculus/`
- [ ] **Создать файлы:**
  - `include/delta/calculus/operational_function.h` — класс `OperationalFunction` (map на каждом уровне)
  - `include/delta/calculus/continuity.h` — функция проверки непрерывности (определение 2.7)
- [ ] **Тесты:**
  - `tests/test_identity_continuity.cpp` — проверка что `f(x)=x` непрерывна
  - `tests/test_constant_continuity.cpp` — константа непрерывна

#### Шаг 1.3: Интеграл (Римановы суммы)
- [ ] **Создать файл:** `include/delta/calculus/riemann_sum.h`
  - Функция `riemann_sum` для левых/правых/любых тегов
- [ ] **Тесты:**
  - `tests/test_dyadic_integral.cpp` — интеграл x dx от 0 до 1 сходится к 1/2
  - `tests/test_quadratic_integral.cpp` — x² dx сходится к 1/3

#### Шаг 1.4: Производная
- [ ] **Создать файл:** `include/delta/calculus/differentiability.h`
  - Функции `left_difference_quotient`, `right_difference_quotient`
  - Проверка дифференцируемости (определение 3.4)
- [ ] **Тесты:**
  - `tests/test_quadratic_derivative.cpp` — производная x² в точке
  - `tests/test_abs_derivative.cpp` — |x| в 0 НЕ дифференцируема

#### Шаг 1.5: Построение ℝ как инварианта
- [ ] **Создать файл:** `include/delta/core/completion.h`
  - Класс `FundamentalSequence` (с экспоненциальной скоростью сходимости)
  - Класс `RealNumber` (обёртка над классом эквивалентности)
- [ ] **Тесты:**
  - `tests/test_rational_embedding.cpp` (пример 6.13) — вложение ℚ в ℝ
  - `tests/test_sqrt2_construction.cpp` — построение √2 через бинарный поиск

#### Шаг 1.6: Обобщённые модули непрерывности (7.5)
- [ ] **Создать файл:** `include/delta/calculus/modulus.h`
  - Шаблонный класс `Modulus` с виртуальным `operator()(double delta)`
  - Конкретные реализации: `ExponentialModulus`, `HolderModulus`, `LogarithmicModulus`
- [ ] **Тесты:**
  - `tests/test_holder_continuity.cpp` — проверка √x с α=0.5

### Этап 2: Продвинутый формализм — Часть 1 (Главы A1-A11)

**Цель:** Реализовать дискретный анализ на структурированных и неструктурированных сетках.

#### Шаг 2.1: Тензорные поля и матричные поля
- [ ] **Создать папку:** `include/delta/geometry/`
- [ ] **Создать файлы:**
  - `include/delta/geometry/tensor_field.h` — класс `TensorField<Rank, Dim>`
  - `include/delta/geometry/matrix_field.h` — специализация для (1,1) тензоров
- [ ] **Подключить:** Eigen3 (уже есть) для работы с матрицами
- [ ] **Тесты:** `tests/test_tensor_arithmetic.cpp`

#### Шаг 2.2: Конечные разности на структурированных сетках
- [ ] **Создать папку:** `include/delta/numerical/`
- [ ] **Создать файлы:**
  - `include/delta/numerical/finite_differences.h` — функции `forward_diff`, `backward_diff`, `central_diff`
  - `include/delta/numerical/gradient.h` — дискретный градиент
  - `include/delta/numerical/laplacian.h` — дискретный лапласиан (5-point stencil)
- [ ] **Тесты:**
  - `tests/test_laplacian_quadratic.cpp` — Δ(x²+y²) = 4 на равномерной сетке

#### Шаг 2.3: Симплициальные комплексы и DEC
- [ ] **Создать файлы:**
  - `include/delta/geometry/simplicial_complex.h` — класс `SimplicialComplex` (вершины + симплексы)
  - `include/delta/geometry/barycentric_subdivision.h` — функция `subdivide`
  - `include/delta/geometry/discrete_forms.h` — классы `DiscreteForm<k>`
- [ ] **Тесты:**
  - `tests/test_barycentric_subdivision.cpp` — проверка что после 1 subdivision число треугольников ×4

#### Шаг 2.4: Дискретный лапласиан на симплициальных комплексах
- [ ] **Создать файл:** `include/delta/numerical/cotangent_laplacian.h`
  - Функция `build_cotangent_laplacian` возвращает разреженную матрицу Eigen
- [ ] **Тесты:**
  - `tests/test_cotangent_laplacian_flat.cpp` — на плоском треугольнике даёт правильные веса

### Этап 3: Продвинутый формализм — Часть 2 (Главы A12-A24)

**Цель:** Уравнения матфизики, вариационные принципы, калибровочные теории.

#### Шаг 3.1: Связности и ковариантная производная
- [ ] **Создать файл:** `include/delta/physics/connection.h`
  - Класс `Connection` (матрицы на рёбрах)
- [ ] **Тесты:** `tests/test_parallel_transport.cpp`

#### Шаг 3.2: Решатели для PDE
- [ ] **Создать папку:** `include/delta/numerical/solvers/`
- [ ] **Создать файлы:**
  - `solvers/poisson.h` — класс `PoissonSolver` (использует Eigen для решения)
  - `solvers/heat.h` — класс `HeatSolver` (схема Кранка-Николсон)
  - `solvers/wave.h` — класс `WaveSolver` (leapfrog)
- [ ] **Тесты:**
  - `tests/test_poisson_1d.cpp` — сходимость O(h²)
  - `tests/test_heat_equation.cpp` — сходимость к аналитическому решению

#### Шаг 3.3: Вариационные принципы
- [ ] **Создать файл:** `include/delta/calculus/action.h`
  - Класс `Action` для построения дискретного действия
  - Функция `euler_lagrange` для вывода дискретных уравнений
- [ ] **Тесты:** `tests/test_free_particle_action.cpp` — уравнения Эйлера-Лагранжа дают равномерное движение

#### Шаг 3.4: Решёточная калибровочная теория
- [ ] **Создать файл:** `include/delta/physics/lattice_gauge.h`
  - Класс `GaugeField` (U(1) или SU(N) матрицы на рёбрах)
  - Функция `wilson_action`
- [ ] **Тесты:** `tests/test_u1_plaquette.cpp` — голономия вокруг квадрата

### Этап 4: Обратный анализ (Главы R1-R7)

**Цель:** Инструменты для решения обратных задач и вероятностного вывода.

#### Шаг 4.1: Δ⁻¹ оператор
- [ ] **Создать папку:** `include/delta/inverse/`
- [ ] **Создать файлы:**
  - `inverse/delta_inverse.h` — класс `DeltaInverse` для построения распределений
  - `inverse/bayesian_inference.h` — байесовский вывод по конечным данным
- [ ] **Тесты:** `tests/test_delta_inverse_constant.cpp`

#### Шаг 4.2: Контекст и вероятностные регулятивные идеи
- [ ] **Создать файл:** `inverse/probabilistic_regulative_idea.h`
- [ ] **Тесты:** `tests/test_probabilistic_mathematicians.cpp` — парабола про бесконечных математиков

### Этап 5: Python-обвязка и визуализация

**Цель:** Сделать библиотеку доступной для исследований и визуализации.

- [ ] **Подключить:** `pybind11` в `vcpkg.json`
- [ ] **Создать папку:** `python_bindings/`
- [ ] **Создать файлы:**
  - `python_bindings/CMakeLists.txt`
  - `python_bindings/delta_core.cpp` — экспорт `Grid`, `DeltaPath`, `OperationalFunction`
  - `python_bindings/visualization_helpers.py` — функции для построения графиков через matplotlib
- [ ] **Пример:** `examples/python/plot_convergence.py` — демо визуализации

## 📊 Диаграмма зависимостей

```
Этап 1 (Core) → boost::rational
         ↓
Этап 2 (Advanced 1) → Eigen3 (уже есть)
         ↓
Этап 3 (Advanced 2) → Eigen3
         ↓
Этап 4 (Reverse) → (пока без новых зависимостей)
         ↓
Этап 5 (Python) → pybind11
```

## 🚦 Следующие шаги (ближайшие 3 дня)

1. ✅ Настроено окружение (сделано)
2. ✅ Eigen + fmt работают (сделано)
3. ⬜ Добавить `boost::rational` в `vcpkg.json`
4. ⬜ Создать `include/delta/core/rational.h` с алиасом
5. ⬜ Создать `include/delta/core/grid.h` с шаблонным классом
6. ⬜ Написать тест `test_grid_construction.cpp`
7. ⬜ Создать первый исполняемый пример `examples/01_basic_dyadic_path`
## Целевая структура проекта

delta-analysis/
├── README.md                       # This file
├── LICENSE                         # CC BY-NC-SA 4.0
├── CMakeLists.txt                   # Root CMake configuration
├── CMakePresets.json                 # CMake presets (Debug/Release/etc)
├── vcpkg.json                        # Dependency manifest
├── .gitignore
├── .clang-format                    # Code style
│
├── include/                          # Public interface (header-only core)
│   └── delta/
│       ├── core/                      # Foundational blocks (0-1.5)
│       │   ├── regulative_idea.h
│       │   ├── grid.h
│       │   ├── delta_path.h
│       │   ├── delta_strategy.h
│       │   └── operational_function.h
│       ├── calculus/                   # Analysis blocks (2-7.5)
│       │   ├── continuity.h
│       │   ├── differentiability.h
│       │   ├── riemann_sum.h
│       │   └── modulus.h
│       ├── numerical/                   # Numerical methods (A5-A8, A15-A18)
│       │   ├── finite_differences.h
│       │   ├── finite_elements.h
│       │   └── solvers/
│       │       ├── poisson.h
│       │       ├── heat.h
│       │       └── wave.h
│       ├── geometry/                     # Geometric structures (A2, A9-A11)
│       │   ├── simplicial_complex.h
│       │   ├── barycentric_subdivision.h
│       │   └── discrete_exterior_calculus.h
│       └── physics/                       # Physical applications (A13-A14, A24)
│           ├── tensor_field.h
│           ├── connection.h
│           └── lattice_gauge.h
│
├── src/                               # Private implementation (when not header-only)
│   ├── CMakeLists.txt
│   └── core/
│       └── delta_path.cpp              # Non-templated parts if any
│
├── tests/                              # Unit tests (MANDATORY)
│   ├── CMakeLists.txt
│   ├── test_rational_embedding.cpp     # Example 6.13: ℚ → ℝ embedding
│   ├── test_dyadic_integral.cpp        # ∫x dx from 0 to 1 via Riemann sums
│   ├── test_non_commutativity.cpp      # Theorem 9.3: λ=1/3 vs λ=2/3
│   ├── test_continuity.cpp             # Definition 2.7 verification
│   ├── test_poisson_1d.cpp             # 1D Poisson convergence
│   ├── test_heat_equation.cpp          # Crank-Nicolson convergence
│   └── test_wave_equation.cpp          # Leapfrog convergence
│
├── examples/                            # Demonstrations and benchmarks
│   ├── CMakeLists.txt
│   ├── 01_basic_dyadic_path/
│   │   └── main.cpp                     # Generate dyadic grids, compute δ_n
│   ├── 02_riemann_sum_convergence/
│   │   └── main.cpp                     # Compute I_n, output CSV for plotting
│   ├── 03_differentiability/
│   │   └── main.cpp                     # Test Definition 3.4 on x², |x|
│   ├── 04_adaptive_strategy/
│   │   └── main.cpp                     # Example 8.6: adaptive refinement
│   ├── 05_poisson_2d_fem/
│   │   └── main.cpp                     # FEM on triangulated square
│   └── 06_lattice_gauge_2d/
│       └── main.cpp                     # U(1) lattice gauge theory in 2D
│
├── python_bindings/                      # Python interface (future)
│   ├── CMakeLists.txt
│   └── pybind11/
│       ├── delta_core.cpp
│       └── visualization_helpers.py      # Plot grids, convergence, etc.
│
├── scripts/                               # Build/dev helper scripts
│   ├── build.sh
│   ├── run_tests.sh
│   └── plot_convergence.py                # Plot CSV output from examples
│
├── docs/                                   # Documentation
│   ├── Doxyfile
│   ├── introduction.md
│   └── theory/                             # Mathematical background
│       ├── block0_regulative_idea.md
│       ├── block1_delta_structure.md
│       └── ...
│
└── third_party/                             # External dependencies (managed by vcpkg)
    └── README.md

## 🐛 Известные проблемы и их решения

| Проблема | Решение |
|----------|---------|
| CMake не видит vcpkg | Проверить `VCPKG_ROOT` и `CMakePresets.json` |
| Линковка Eigen падает | Убедиться что `target_link_libraries` включает `Eigen3::Eigen` |
| boost::rational не устанавливается | Добавить в `vcpkg.json` и запустить vcpkg install |
| Консольные билды ломают VS | Запрещены правилами проекта (см. выше) |

---

*Последнее обновление: 1 марта 2026*
```