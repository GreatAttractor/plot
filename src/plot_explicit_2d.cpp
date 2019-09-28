//
// Plot
// Copyright (c) 2019 Filip Szczerek <ga.software@yahoo.com>
//
// This project is licensed under the terms of the MIT license
// (see the LICENSE file for details).
//

#include "plot_explicit_2d.hpp"

#include <algorithm>
#include <iostream>

#define PLOT_ASSERT(condition)                                           \
{                                                                           \
    if (!(condition))                                                       \
    {                                                                       \
        std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__  \
                  << " inside " << __FUNCTION__ << "\n"                     \
                  << "Condition: " << #condition << "\n";                   \
        std::abort();                                                       \
    }                                                                       \
}


namespace plot {

/// Returns ceil(log2(i)).
static int CeilingLog2(size_t i)
{
    int k{0};
    bool is_power_of_2{true};

    while (i > 0)
    {
        if (i > 1 && (i & 1)) { is_power_of_2 = false; }

        i >>= 1;
        ++k;
    }

    return is_power_of_2 ? k - 1 : k;
}

ExplicitSingleValueCurve2D::ExplicitSingleValueCurve2D(
    std::shared_ptr<std::vector<double>> x_values,
    std::shared_ptr<std::vector<std::optional<double>>> y_values
): x_values_(x_values), y_values_(y_values)
{
    PLOT_ASSERT(x_values_->size() == y_values_->size());
    PLOT_ASSERT(!x_values_->empty());

    for (size_t i = 1; i < x_values->size(); ++i)
    {
        PLOT_ASSERT((*x_values)[i] > (*x_values)[i-1]);
    }

    const int k = CeilingLog2(y_values_->size());
    const size_t num_intervals = (1 << k) - 1;

    intervals_ = std::make_unique<Interval[]>(num_intervals);

    if (y_values_->size() > 1)
    {
        FillIntervals();
    }
}

static std::optional<double> GetOneOf(const std::optional<double>& a, const std::optional<double>& b, double comp_func(double, double))
{
    if (a.has_value() && b.has_value())
    {
        return comp_func(*a, *b);
    }
    else if (a.has_value())
    {
        return *a;
    }
    else if (b.has_value())
    {
        return *b;
    }
    else
    {
        return std::nullopt;
    }
}

static std::optional<std::tuple<double, double>> GetMinMax(const std::optional<double>& a, const std::optional<double>& b)
{
    if (a.has_value() && b.has_value())
    {
        return std::make_tuple(std::min(*a, *b), std::max(*a, *b));
    }
    else if (a.has_value())
    {
        return std::make_tuple(*a, *a);
    }
    else if (b.has_value())
    {
        return std::make_tuple(*b, *b);
    }
    else
    {
        return std::nullopt;
    }
}

void ExplicitSingleValueCurve2D::FillIntervals()
{
    // Consider `y_values_` having 16 elements (N = 16 = 2^k, k = 4).
    // The complete binary tree of intervals is stored in `intervals_` as follows:
    //
    // layer 0, index 0:       (0,15),
    // layer 1, indices 1-2:   (0,7), (8,15),
    // layer 2, indices 3-6:   (0,3), (4,7), (8,11), (12,15),
    // layer 3, indices 7-14:  (0,1), (2,3), (4,5), (6,7), (8,9), (10,11), (12,13), (14,15)
    //
    // The lowest layer starts at index 2^(k-1)-1 and contains 2^(k-1) elements,
    // the next at 2^(k-2)-1 with 2^(k-2) elements, the next at 2^(k-3)-1 with 2^(k-3) elements and so on.
    // Finally, the top layer contains just one element at index 0.
    //
    // Element of `intervals_` at index `i` has children at 2*i+1, 2*i+2.

    const int k = CeilingLog2(y_values_->size());

    // layers are filled from the lowest one

    // handle the lowest layer separately, as it depends directly on `y_values_`
    {
        const int layer = k - 1;
        int layer_start = (1 << layer) - 1;
        int layer_end = layer_start + (1 << layer);

        size_t interval_start = 0;
        for (int i = layer_start; i < layer_end; ++i)
        {
            auto& interval = intervals_[i];
            interval.lo_idx = interval_start;
            interval.hi_idx = interval_start + (1 << (k - layer)) - 1;

            interval.min_max = GetMinMax((*y_values_)[interval.lo_idx], (*y_values_)[interval.hi_idx]);

            interval_start = interval.hi_idx + 1;
        }
    }

    // handle the higher layers
    for (int layer = k - 2; layer >= 0; --layer)
    {
        int layer_start = (1 << layer) - 1;
        int layer_end = layer_start + (1 << layer);

        size_t interval_start = 0;
        for (int i = layer_start; i < layer_end; ++i)
        {
            auto& interval = intervals_[i];
            const int child_1 = 2 * i + 1;
            const int child_2 = 2 * i + 2;
            interval.lo_idx = interval_start;
            interval.hi_idx = interval_start + (1 << (k - layer)) - 1;
            interval_start = interval.hi_idx + 1;

            if (intervals_[child_1].min_max.has_value() && intervals_[child_2].min_max.has_value())
            {
                interval.min_max = {
                    std::min(
                        std::get<0>(*intervals_[child_1].min_max),
                        std::get<0>(*intervals_[child_2].min_max)
                    ),
                    std::max(
                        std::get<1>(*intervals_[child_1].min_max),
                        std::get<1>(*intervals_[child_2].min_max)
                    )
                };
            }
            else if (intervals_[child_1].min_max.has_value())
            {
                interval.min_max = intervals_[child_1].min_max;
            }
            else if (intervals_[child_2].min_max.has_value())
            {
                interval.min_max = intervals_[child_2].min_max;
            }
            else
            {
                interval.min_max = std::nullopt;
            }
        }
    }
}

std::optional<std::tuple<double, double>> ExplicitSingleValueCurve2D::GetMinMaxOverDomainInterval(double xmin, double xmax) const
{
    const auto lo_bound = std::lower_bound(x_values_->begin(), x_values_->end(), xmin);
    if (lo_bound == x_values_->end()) { return std::nullopt; }

    const auto hi_bound = std::upper_bound(x_values_->begin(), x_values_->end(), xmax);
    if (hi_bound == x_values_->begin()) { return std::nullopt; }

    size_t lo_idx = lo_bound - x_values_->begin();
    size_t hi_idx = hi_bound - x_values_->begin() - 1;

    // interpolated Y value at `xmin` if `xmin` falls between two `x_values_` having non-empty `y_values_`.
    std::optional<double> lo_interp;
    // interpolated Y value at `xmax` if `xmax` falls between two `x_values_` having non-empty `y_values_`.
    std::optional<double> hi_interp;

    if ((*x_values_)[lo_idx] > xmin &&
        lo_idx > 0 &&
        (*y_values_)[lo_idx].has_value() &&
        (*y_values_)[lo_idx - 1].has_value() &&
        (*x_values_)[lo_idx - 1] != (*x_values_)[lo_idx])
    {
        lo_interp = *(*y_values_)[lo_idx - 1] +
            (xmin - (*x_values_)[lo_idx - 1]) / ((*x_values_)[lo_idx] - (*x_values_)[lo_idx - 1]) *
            (*(*y_values_)[lo_idx] - *(*y_values_)[lo_idx - 1]);
    }

    if (hi_idx < x_values_->size() - 1 &&
        (*x_values_)[hi_idx] != xmax &&
        (*x_values_)[hi_idx + 1] > xmax &&
        (*y_values_)[hi_idx].has_value() &&
        (*x_values_)[hi_idx + 1] != (*x_values_)[hi_idx] &&
        (*y_values_)[hi_idx + 1].has_value())
    {
        hi_interp = *(*y_values_)[hi_idx] +
            (xmax - (*x_values_)[hi_idx]) / ((*x_values_)[hi_idx + 1] - (*x_values_)[hi_idx]) *
            (*(*y_values_)[hi_idx + 1] - *(*y_values_)[hi_idx]);
    }

    std::optional<std::tuple<double, double>> min_max_inside_interval;

    if (hi_idx >= lo_idx) // if there are any `x_values_` between (xmin, xmax)
    {
        min_max_inside_interval = GetMinMaxOverIndexInterval(lo_idx, hi_idx, 0);
    }

    if (min_max_inside_interval.has_value())
    {
        const auto min_interp = GetOneOf(lo_interp, hi_interp, [](double a, double b) { return std::min(a, b); });
        const auto max_interp = GetOneOf(lo_interp, hi_interp, [](double a, double b) { return std::max(a, b); });

        const auto actual_min = GetOneOf(min_interp, std::get<0>(*min_max_inside_interval), [](double a, double b) { return std::min(a, b); });
        const auto actual_max = GetOneOf(max_interp, std::get<1>(*min_max_inside_interval), [](double a, double b) { return std::max(a, b); });

        if (actual_min.has_value() && actual_max.has_value())
        {
            return std::make_tuple(*actual_min, *actual_max);
        }
        else if (actual_min.has_value())
        {
            return std::make_tuple(*actual_min, *actual_min);
        }
        else if (actual_max.has_value())
        {
            return std::make_tuple(*actual_max, *actual_max);
        }
        else
        {
            return std::nullopt;
        }
    }
    else
    {
        return GetMinMax(lo_interp, hi_interp);
    }
}

std::optional<std::tuple<double, double>> ExplicitSingleValueCurve2D::GetMinMaxOverIndexInterval(size_t lo_idx, size_t hi_idx, size_t interval_idx) const
{
    if (intervals_[interval_idx].lo_idx == lo_idx &&
        intervals_[interval_idx].hi_idx == hi_idx)
    {
        return intervals_[interval_idx].min_max;
    }
    else if (lo_idx == hi_idx)
    {
        if ((*y_values_)[lo_idx].has_value())
        {
            return std::make_tuple(*(*y_values_)[lo_idx], *(*y_values_)[lo_idx]);
        }
        else
        {
            return std::nullopt;
        }
    }
    else
    {
        const size_t child1_idx = 2 * interval_idx + 1;
        const size_t child2_idx = 2 * interval_idx + 2;
        const Interval& child1 = intervals_[child1_idx];
        const Interval& child2 = intervals_[child2_idx];

        if (lo_idx >= child1.lo_idx && hi_idx <= child1.hi_idx)
        {
            return GetMinMaxOverIndexInterval(lo_idx, hi_idx, child1_idx);
        }
        else if (lo_idx >= child2.lo_idx && hi_idx <= child2.hi_idx)
        {
            return GetMinMaxOverIndexInterval(lo_idx, hi_idx, child2_idx);
        }
        else
        {
            const auto& partial1 = GetMinMaxOverIndexInterval(lo_idx,        child1.hi_idx, child1_idx);
            const auto& partial2 = GetMinMaxOverIndexInterval(child2.lo_idx, hi_idx,        child2_idx);

            if (partial1.has_value() && partial2.has_value())
            {
                return std::make_tuple(
                    std::min(std::get<0>(*partial1), std::get<0>(*partial2)),
                    std::max(std::get<1>(*partial1), std::get<1>(*partial2))
                );
            }
            else if (partial1.has_value())
            {
                return partial1;
            }
            else if (partial2.has_value())
            {
                return partial2;
            }
            else
            {
                return std::nullopt;
            }
        }
    }
}

} // namespace plot
