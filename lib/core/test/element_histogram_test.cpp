// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "scipp/common/constants.h"
#include "scipp/core/element/histogram.h"
#include "scipp/units/unit.h"

using namespace scipp;
using namespace scipp::core;

TEST(ElementHistogramTest, variance_flags) {
  static_assert(
      std::is_base_of_v<transform_flags::expect_in_variance_if_out_variance_t,
                        decltype(element::histogram)>);
  static_assert(std::is_base_of_v<transform_flags::expect_no_variance_arg_t<1>,
                                  decltype(element::histogram)>);
  static_assert(std::is_base_of_v<transform_flags::expect_no_variance_arg_t<3>,
                                  decltype(element::histogram)>);
}

TEST(ElementHistogramTest, unit) {
  // Note that this is an operator for `transform_subspan`, so the overload for
  // units has one argument fewer than the one for data.
  EXPECT_EQ(element::histogram(units::m, units::counts, units::m),
            units::counts);
}

TEST(ElementHistogramTest, event_and_edge_unit_must_match) {
  EXPECT_NO_THROW(element::histogram(units::m, units::counts, units::m));
  EXPECT_NO_THROW(element::histogram(units::s, units::counts, units::s));
  EXPECT_THROW(element::histogram(units::m, units::counts, units::s),
               except::UnitError);
  EXPECT_THROW(element::histogram(units::s, units::counts, units::m),
               except::UnitError);
}

TEST(ElementHistogramTest, weight_unit_propagates) {
  for (const auto &unit : {units::m, units::counts, units::one})
    EXPECT_EQ(element::histogram(units::m, unit, units::m), unit);
}

TEST(ElementHistogramTest, values) {
  std::vector<double> edges{2, 4, 6};
  std::vector<double> events{1, 2, 3, 4, 5, 6, 7};
  std::vector<double> weight_vals{10, 20, 30, 40, 50, 60, 70};
  std::vector<double> weight_vars{100, 200, 300, 400, 500, 600, 700};
  std::vector<double> result_vals{0, 0};
  std::vector<double> result_vars{0, 0};
  element::histogram(
      ValueAndVariance(std::span(result_vals), std::span(result_vars)), events,
      ValueAndVariance(std::span(weight_vals), std::span(weight_vars)), edges);
  EXPECT_EQ(result_vals, std::vector<double>({20 + 30, 40 + 50}));
  EXPECT_EQ(result_vars, std::vector<double>({200 + 300, 400 + 500}));
}

TEST(ElementHistogramTest, no_variance) {
  std::vector<double> edges{2, 4, 6};
  std::vector<double> events{1, 2, 3, 4, 5, 6, 7};
  std::vector<double> weight_vals{10, 20, 30, 40, 50, 60, 70};
  std::vector<double> result_vals{0, 0};
  element::histogram(std::span(result_vals), events, std::span(weight_vals),
                     edges);
  EXPECT_EQ(result_vals, std::vector<double>({20 + 30, 40 + 50}));
}
