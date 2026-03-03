// include/delta/core/adaptive_delta_path.h
#pragma once

#include <set>
#include <queue>
#include <vector>
#include <functional>
#include <cstddef>
#include <cassert>
#include "rational.h"
#include "value_metric.h"
#include "regulative_idea.h"

namespace delta {

    /**
     * @class AdaptiveDeltaPath
     * @brief Адаптивный Δ-путь, вставляющий точки по одной, выбирая интервал с наибольшим приоритетом.
     *
     * @tparam Addr тип адреса (должен поддерживать сравнение и арифметику)
     * @tparam Value тип значения функции
     * @tparam Distance тип расстояния (скалярный)
     * @tparam Betweenness тип отношения междусловности (из регулятивной идеи)
     * @tparam Metric тип метрики на адресах
     * @tparam ValueMetric тип метрики на значениях
     * @tparam Compare компаратор для адресов (по умолчанию std::less<Addr>)
     *
     * Использование:
     * 1. Задать начальные точки, функцию f, оператор вставки, порог.
     * 2. Вызывать advance() до тех пор, пока не будет достигнуто желаемое число точек
     *    или пока очередь не опустеет.
     * 3. Получить текущее множество точек через points().
     *
     * Оператор вставки (тип Operator) – это любая вызываемая сущность с сигнатурой
     * Addr(const Addr& left, const Addr& right, const Value& f_left, const Value& f_right),
     * возвращающая новую точку строго между left и right.
     */
    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric,
        typename Compare = std::less<Addr>>
        class AdaptiveDeltaPath {
        public:
            // Типы
            using Operator = std::function<Addr(const Addr&, const Addr&,
                const Value&, const Value&)>;
            using Func = std::function<Value(const Addr&)>;

            /**
             * @brief Конструктор
             * @param initial_points начальные адреса (должны быть упорядочены)
             * @param func функция, вычисляющая значение в любой точке
             * @param op оператор вставки
             * @param threshold порог: интервалы с приоритетом ≤ threshold не уточняются
             * @param betweenness отношение междусловности
             * @param metric метрика на адресах (не используется внутри, но сохраняется для совместимости)
             * @param value_metric метрика на значениях
             */
            AdaptiveDeltaPath(const std::vector<Addr>& initial_points,
                Func func,
                Operator op,
                Distance threshold = Distance(0),
                Betweenness betweenness = Betweenness{},
                Metric metric = Metric{},
                ValueMetric value_metric = ValueMetric{})
                : func_(std::move(func))
                , op_(std::move(op))
                , threshold_(threshold)
                , betweenness_(std::move(betweenness))
                , metric_(std::move(metric))
                , value_metric_(std::move(value_metric))
            {
                // Вставляем начальные точки в set
                for (const auto& p : initial_points) {
                    points_.insert(p);
                }

                // Создаём интервалы для всех соседних пар
                auto it = points_.begin();
                auto next = std::next(it);
                while (next != points_.end()) {
                    Addr left = *it;
                    Addr right = *next;
                    // Вычисляем значения через функцию (не сохраняем отдельно)
                    Value f_left = func_(left);
                    Value f_right = func_(right);
                    Distance prio = value_metric_(f_right, f_left);
                    if (prio > threshold_) {
                        queue_.push(Interval{ left, right, f_left, f_right, prio });
                    }
                    ++it;
                    ++next;
                }
            }

            /**
             * @brief Выполнить один шаг уточнения: обработать один интервал с наивысшим приоритетом.
             * @return true, если интервал был обработан; false, если очередь пуста.
             */
            bool advance() {
                if (queue_.empty()) return false;

                // Извлекаем интервал с максимальным приоритетом
                Interval intv = queue_.top();
                queue_.pop();

                // Вычисляем новую точку с помощью оператора
                Addr mid = op_(intv.left, intv.right, intv.f_left, intv.f_right);

                // Проверяем строгую междусловность (только в отладочном режиме)
                assert(betweenness_(intv.left, mid, intv.right) && "Operator returned point not between endpoints");

                // Вставляем точку в множество
                points_.insert(mid);

                // Вычисляем значение в новой точке
                Value f_mid = func_(mid);

                // Создаём два новых интервала, если их приоритет выше порога
                Distance prio1 = value_metric_(intv.f_left, f_mid);
                if (prio1 > threshold_) {
                    queue_.push(Interval{ intv.left, mid, intv.f_left, f_mid, prio1 });
                }
                Distance prio2 = value_metric_(f_mid, intv.f_right);
                if (prio2 > threshold_) {
                    queue_.push(Interval{ mid, intv.right, f_mid, intv.f_right, prio2 });
                }

                return true;
            }

            /// Возвращает константную ссылку на множество всех текущих точек (упорядоченное).
            const std::set<Addr, Compare>& points() const { return points_; }

            /// Возвращает количество текущих точек.
            std::size_t size() const { return points_.size(); }

            /// Вычисляет значение в произвольной точке (полезно для внешнего использования).
            Value value_at(const Addr& x) const { return func_(x); }

        private:
            // Внутренняя структура для хранения интервала с приоритетом
            struct Interval {
                Addr left;
                Addr right;
                Value f_left;
                Value f_right;
                Distance priority;

                // Компаратор для max-кучи: чем больше priority, тем выше в очереди
                bool operator<(const Interval& other) const {
                    return priority < other.priority;
                }
            };

            std::set<Addr, Compare> points_;            // все точки
            std::priority_queue<Interval> queue_;       // очередь интервалов (max по priority)
            Func func_;                                  // исходная функция
            Operator op_;                                // оператор вставки
            Distance threshold_;                         // порог отбрасывания
            Betweenness betweenness_;                    // отношение междусловности (для отладки)
            [[maybe_unused]] Metric metric_;             // метрика на адресах (не используется, но сохраняем)
            ValueMetric value_metric_;                   // метрика на значениях
    };

} // namespace delta