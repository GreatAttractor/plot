//
// Plot
// Copyright (c) 2019 Filip Szczerek <ga.software@yahoo.com>
//
// This project is licensed under the terms of the MIT license
// (see the LICENSE file for details).
//

#pragma once

#ifndef PLOT_H
#define PLOT_H

#include <optional>
#include <tuple>
#include <vector>
#include <memory>

namespace plot {

/// Represents an explicit single-value 2D curve: y = f(x); finds min and max value over an interval in O(log n).
class ExplicitSingleValueCurve2D
{
public:
    /// Constructor.
    ///
    /// @param x_values X values; must be strictly increasing.
    /// @param y_values Y values corresponding to `x_values`.
    ///
    /// Using `shared_ptr`s to simplify working with caching (if any) of the values on the client side.
    ///
    ExplicitSingleValueCurve2D(
        std::shared_ptr<std::vector<double>> x_values,
        std::shared_ptr<std::vector<std::optional<double>>> y_values
    );

    /// Returns the min and max Y value in the interval [xmin, xmax];
    /// or `std::nullopt` if the interval contains no values.
    std::optional<std::tuple<double, double>> GetMinMaxOverDomainInterval(double xmin, double xmax) const;

    const std::vector<double>& GetXValues() const { return *x_values_; }
    const std::vector<std::optional<double>>& GetYValues() const { return *y_values_; }

private:
    void FillIntervals();

    /// Returns the min and max value of the `y_values_` interval between indices [lo_idx, hi_idx];
    /// or `std::nullopt` if the interval contains no values.
    ///
    /// @param interval_idx Index in `intervals_` of the initial interval to test.
    ///
    std::optional<std::tuple<double, double>> GetMinMaxOverIndexInterval(size_t lo_idx, size_t hi_idx, size_t interval_idx) const;

    std::shared_ptr<std::vector<double>> x_values_;
    std::shared_ptr<std::vector<std::optional<double>>> y_values_;

    struct Interval
    {
        uint32_t lo_idx; ///< Lower bound (inclusive) of the interval; index in `y_values_`.
        uint32_t hi_idx; ///< Upper bound (inclusive) of the interval; index in `y_values_`.
        std::optional<std::tuple<double, double>> min_max; ///< Min and max value (from `y_values_`) of the interval.
    };

    /// Stores a complete binary tree.
    ///
    /// Root element ([0]) encompasses indices (0, N-1), where N is `y_values_.size()`
    /// rounded to the nearest greater power of 2.
    /// Root's direct children represent intervals (0, N/2-1), (N/2, N-1), and their children
    /// similarly divide each interval in two.
    /// Element [i] has children at [2*i+1], [2*i+2].
    ///
    std::unique_ptr<Interval[]> intervals_;
};

} // namespace plot

#endif // PLOT_H
