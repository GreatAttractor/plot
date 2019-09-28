//
// Plot
// Copyright (c) 2019 Filip Szczerek <ga.software@yahoo.com>
//
// This project is licensed under the terms of the MIT license
// (see the LICENSE file for details).
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE PlotExplicit2D

#include "plot_explicit_2d.hpp"

#include <boost/test/unit_test.hpp>
#include <memory>

using plot::ExplicitSingleValueCurve2D;

static std::shared_ptr<std::vector<double>> MakeDoubleVectorPtr(const std::initializer_list<double>& values)
{
    return std::make_shared<std::vector<double>>(values);
}

static std::shared_ptr<std::vector<std::optional<double>>> MakeOptionalDoubleVectorPtr(const std::initializer_list<std::optional<double>>& values)
{
    return std::make_shared<std::vector<std::optional<double>>>(values);
}

// ---------------------------- Test cases -------------------------------------------

BOOST_AUTO_TEST_CASE(Interpolation)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //             |       |
    //  2          |       |   o
    //  1.5        |       *
    //  1          |   o   |
    //  0.5        *       |
    //  0      o   |       |
    //
    // x:      0  0.5  1  1.5  2
    //
    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(0.5, 1.5);

    BOOST_CHECK_EQUAL(0.5, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(1.5, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(InterpolationAtEnd)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //                     |       |
    //  2                  |   o   |
    //  1.5                *       |
    //  1              o   |       |
    //                     |       |
    //  0      o           |       |
    //
    // x:      0       1  1.5  2  2.5
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(1.5, 2.5);

    BOOST_CHECK_EQUAL(1.5, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(2.0, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(InterpolationAtStart)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //          |        |
    //  2       |        |           o
    //          |        |
    //  1       |        |    o
    //  0.5     |        *
    //  0       |    o   |
    //          |        |
    // x:     -0.5   0   0.5  1      2
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(-0.5, 0.5);

    BOOST_CHECK_EQUAL(0.0, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(0.5, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(QueryAboveRange)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //                                 |     |
    //  2                        o     |     |
    //                                 |     |
    //  1                  o           |     |
    //                                 |     |
    //  0            o                 |     |
    //                                 |     |
    // x:            0     1     2     3     4
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(3, 4);
    BOOST_CHECK(!min_max.has_value());
}

BOOST_AUTO_TEST_CASE(QueryBelowRange)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //             |     |
    //  2          |     |                o
    //             |     |
    //  1          |     |          o
    //             |     |
    //  0          |     |    o
    //             |     |
    // x:         -4    -3    0     1     2
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(-4, -3);
    BOOST_CHECK(!min_max.has_value());
}

BOOST_AUTO_TEST_CASE(InterpolatedValuesIgnored)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //             |               |
    //  4          |           o   |
    //             |               *
    //  3          |               |   o
    //             |               |
    //             |               |
    //             |               |
    //  1          |               |
    //             |               |
    //  0      o   |               |
    //             *               |
    // -1          |   o           |
    //
    // x:      0  0.5  1       2  2.5  3
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2, 3});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, -1, 4, 3});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(0.5, 2.5);

    BOOST_CHECK_EQUAL(-1, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(4, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(EmptyResultIfEmptyYValues)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    // ^ - empty Y value
    //
    //  y:
    //
    //              |               |
    //  3           |   ^      ^    |     o
    //              |   ^      ^    |
    //  2           |   ^      ^    |
    //              |   ^      ^    |
    //  1           |   ^      ^    |
    //              |   ^      ^    |
    //  0       o   |   ^      ^    |
    //              |   ^      ^    |
    // x:       0  0.5  1      2   2.5    3
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2, 3});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, std::nullopt, std::nullopt, 3});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(0.5, 2.5);

    BOOST_CHECK(!min_max.has_value());
}

BOOST_AUTO_TEST_CASE(NoInterpolationAcrossEmptyYValueUpper)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    // ^ - empty Y value
    //
    //  y:
    //
    //              |               |
    //  3           |           ^   |   o
    //              |           ^   |
    //  2           |           ^   |
    //              |           ^   |
    //  1           |   o       ^   |
    //  0.5         *           ^   |
    //  0       o   |           ^   |
    //              |           ^   |
    // x:       0  0.5  1       2  2.5  3
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2, 3});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, std::nullopt, 3});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(0.5, 2.5);

    BOOST_CHECK_EQUAL(0.5, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(1.0, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(NoInterpolationAcrossEmptyYValueLower)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    // ^ - empty Y value
    //
    //  y:
    //
    //              |               |
    //  3           |   ^           |   o
    //              |   ^           *
    //  2           |   ^       o   |
    //              |   ^           |
    //  1           |   ^           |
    //  0.5         |   ^           |
    //  0       o   |   ^           |
    //              |   ^           |
    // x:       0  0.5  1       2  2.5  3
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2, 3});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, std::nullopt, 2, 3});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(0.5, 2.5);

    BOOST_CHECK_EQUAL(2.0, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(2.5, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(NotManyValues)
{
    const auto x_values = std::make_shared<std::vector<double>>();
    const auto y_values = std::make_shared<std::vector<std::optional<double>>>();

    for (int i = 0; i < 16; ++i)
    {
        x_values->push_back(i);
        y_values->push_back(i);
    }

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(5.0, 13.0);

    BOOST_CHECK_EQUAL(5.0, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(13.0, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(ManyValues)
{
    const auto x_values = std::make_shared<std::vector<double>>();
    const auto y_values = std::make_shared<std::vector<std::optional<double>>>();

    for (int i = 0; i < 1024; ++i)
    {
        x_values->push_back(i);
        y_values->push_back(i);
    }

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(101.0, 653.0);

    BOOST_CHECK_EQUAL(101.0, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(653.0, std::get<1>(min_max.value()));
}

BOOST_AUTO_TEST_CASE(InterpolateWithEmptyInterval)
{
    // Vertical lines mark the arguments of `GetMinMaxOverDomainInterval()`.
    //
    // o - plot points
    // * - interpolated plot points
    //
    //  y:
    //                         |   |
    //  2                      |   | o
    //  1.75                   |   *
    //                         |   |
    //  1.25                   *   |
    //  1                    o |   |
    //                         |   |
    //                         |   |
    //                         |   |
    //  0            o         |   |
    //                         |   |
    // x:            0       1 |   |  2
    //                       1.25  1.75
    //

    const auto x_values = MakeDoubleVectorPtr({0, 1, 2});
    const auto y_values = MakeOptionalDoubleVectorPtr({0, 1, 2});

    ExplicitSingleValueCurve2D plot(x_values, y_values);
    const std::optional<std::tuple<double, double>> min_max = plot.GetMinMaxOverDomainInterval(1.25, 1.75);
    BOOST_CHECK_EQUAL(1.25, std::get<0>(min_max.value()));
    BOOST_CHECK_EQUAL(1.75, std::get<1>(min_max.value()));
}
