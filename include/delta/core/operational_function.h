// include/delta/core/operational_function.h
#pragma once

#include <map>
#include <functional>
#include <cassert>
#include <type_traits>
#include "list_grid.h"
#include "uniform_grid.h"
#include "grid_concept.h"

namespace delta {

    namespace detail {
        // Вспомогательный трейт для определения равномерности сетки
        template<typename Grid>
        struct is_uniform_grid : std::false_type {};

        template<typename T, typename C>
        struct is_uniform_grid<UniformGrid<T, C>> : std::true_type {};

        // Получение индекса в равномерной сетке по адресу
        template<typename Addr, typename Grid>
        std::size_t uniform_index(const Addr& addr, const Grid& grid) {
            auto idx = (addr - grid.start()) / grid.step();
            // idx должно быть целым числом (знаменатель 1)
            if (denominator(idx) != 1) {
                throw std::runtime_error("Address does not belong to uniform grid (non-integer index)");
            }
            std::size_t uidx = static_cast<std::size_t>(numerator(idx));
            if (uidx >= grid.size()) {
                throw std::out_of_range("Index out of bounds");
            }
            return uidx;
        }
    }

    // -------------------------------------------------------------------------
    // Основной шаблон (для произвольных сеток) — использует std::map
    // -------------------------------------------------------------------------
    template<typename Addr, typename Value, typename Grid, typename Enable = void>
    class OperationalFunction {
    public:
        using Interpolator = std::function<Value(const Addr&, const Addr&,
            const Value&, const Value&)>;

        template<typename Func>
            requires GridConcept<Grid, Addr>
        OperationalFunction(const Grid& grid, Func&& initial) {
            for (const auto& addr : grid) {
                values_[addr] = initial(addr);
            }
        }

        template<typename OldGrid>
            requires GridConcept<OldGrid, Addr>
        void extend(const OldGrid& old_grid, const Grid& new_grid,
            Interpolator interpolate) {
            const std::size_t old_size = old_grid.size();
            const std::size_t new_size = new_grid.size();

            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_size; ++new_idx) {
                const Addr& addr = new_grid[new_idx];

                if (values_.find(addr) == values_.end()) {
                    assert(old_idx + 1 < old_size && "No interval for new address");
                    const Addr& left = old_grid[old_idx];
                    const Addr& right = old_grid[old_idx + 1];
                    Value val = interpolate(left, right,
                        values_.at(left), values_.at(right));
                    values_[addr] = std::move(val);
                }

                if (old_idx + 1 < old_size && addr == old_grid[old_idx + 1]) {
                    ++old_idx;
                }
            }

            assert(old_idx == old_size - 1 && "Did not consume all old addresses");
        }

        const Value& operator()(const Addr& addr) const {
            auto it = values_.find(addr);
            if (it == values_.end()) {
                throw std::out_of_range("Address not found in operational function");
            }
            return it->second;
        }

        bool contains(const Addr& addr) const {
            return values_.find(addr) != values_.end();
        }

    private:
        std::map<Addr, Value> values_;
    };

    // -------------------------------------------------------------------------
    // Специализация для UniformGrid (равномерные сетки)
    // -------------------------------------------------------------------------
    template<typename Addr, typename Value, typename Compare>
    class OperationalFunction<Addr, Value, UniformGrid<Addr, Compare>> {
    public:
        using Grid = UniformGrid<Addr, Compare>;
        using Interpolator = std::function<Value(const Addr&, const Addr&,
            const Value&, const Value&)>;

        template<typename Func>
        OperationalFunction(const Grid& grid, Func&& initial)
            : grid_(grid)
        {
            values_.reserve(grid.size());
            for (const auto& addr : grid) {
                values_.push_back(initial(addr));
            }
        }

        // Перегрузка extend для случая, когда новая сетка тоже UniformGrid
        void extend(const Grid& old_grid, const Grid& new_grid,
            Interpolator interpolate) {
            // Предполагаем, что новая сетка получается из старой равномерным уточнением
            // (например, dyadic refinement). Тогда new_grid.size() == 2*old_grid.size() - 1
            std::size_t old_n = old_grid.size();
            std::size_t new_n = new_grid.size();

            std::vector<Value> new_values;
            new_values.reserve(new_n);

            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_n; ++new_idx) {
                Addr addr = new_grid[new_idx];
                if (old_idx < old_n && addr == old_grid[old_idx]) {
                    // Старый адрес
                    new_values.push_back(values_[old_idx]);
                    ++old_idx;
                }
                else {
                    // Новый адрес – интерполируем между old_idx-1 и old_idx
                    assert(old_idx > 0 && old_idx < old_n && "Invalid interpolation interval");
                    const Addr& left = old_grid[old_idx - 1];
                    const Addr& right = old_grid[old_idx];
                    Value val = interpolate(left, right,
                        values_[old_idx - 1], values_[old_idx]);
                    new_values.push_back(std::move(val));
                }
            }
            assert(old_idx == old_n && "Did not consume all old addresses");

            values_ = std::move(new_values);
            grid_ = new_grid;
        }

        // Версия extend для случая, когда старая сетка UniformGrid, а новая – произвольная
        // (например, после адаптивного уточнения). Тогда мы вынуждены переключиться на map.
        template<typename OtherGrid>
            requires (!detail::is_uniform_grid<OtherGrid>::value) &&
        GridConcept<OtherGrid, Addr>
            void extend(const Grid& old_grid, const OtherGrid& new_grid,
                Interpolator interpolate) {
            // Перестраиваем внутреннее представление на map
            std::map<Addr, Value> map_values;
            for (std::size_t i = 0; i < old_grid.size(); ++i) {
                map_values[old_grid[i]] = values_[i];
            }

            // Далее работаем как с map (аналогично основному шаблону)
            const std::size_t old_size = old_grid.size();
            const std::size_t new_size = new_grid.size();

            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_size; ++new_idx) {
                const Addr& addr = new_grid[new_idx];

                if (map_values.find(addr) == map_values.end()) {
                    assert(old_idx + 1 < old_size && "No interval for new address");
                    const Addr& left = old_grid[old_idx];
                    const Addr& right = old_grid[old_idx + 1];
                    Value val = interpolate(left, right,
                        map_values.at(left), map_values.at(right));
                    map_values[addr] = std::move(val);
                }

                if (old_idx + 1 < old_size && addr == old_grid[old_idx + 1]) {
                    ++old_idx;
                }
            }
            assert(old_idx == old_size - 1 && "Did not consume all old addresses");

            // Заменяем внутреннее представление на map (теперь это будет общий случай)
            // Но мы не можем изменить тип *this. Вместо этого сохраняем map и отмечаем,
            // что теперь работаем в map-режиме. Для простоты можно хранить variant.
            // Однако для текущей задачи (равномерные сетки) такой переход не требуется,
            // поэтому пока оставим как есть и при необходимости вернёмся к этому позже.
            // Пока просто копируем map в значения, но это неэффективно.
            // Лучше запретить такой переход или обработать позже.
            // В данной реализации мы просто не поддерживаем переход от равномерной к неравномерной.
            static_assert(detail::is_uniform_grid<OtherGrid>::value,
                "Transition from UniformGrid to non-uniform grid not supported yet");
        }

        const Value& operator()(const Addr& addr) const {
            std::size_t idx = detail::uniform_index(addr, grid_);
            return values_[idx];
        }

        bool contains(const Addr& addr) const {
            try {
                detail::uniform_index(addr, grid_);
                return true;
            }
            catch (...) {
                return false;
            }
        }

        // Для доступа к внутренним данным (например, в тестах)
        const std::vector<Value>& values() const { return values_; }

    private:
        Grid grid_;
        std::vector<Value> values_;
    };

} // namespace delta