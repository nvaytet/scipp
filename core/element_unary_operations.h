// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#ifndef SCIPP_CORE_ELEMENT_UNARY_OPERATIONS_H
#define SCIPP_CORE_ELEMENT_UNARY_OPERATIONS_H

#include <cmath>

#include "scipp/common/overloaded.h"
#include "scipp/core/transform.h"

namespace scipp::core {

/// Operators to be used with transform and transform_in_place to implement
/// operations for Variable.
namespace element {

/// Helper to define lists of supported arguments for transform_in_place.
template <class... Ts> struct arg_list_t {
  constexpr void operator()() const noexcept;
  using types = std::tuple<Ts...>;
};
template <class... Ts> constexpr arg_list_t<Ts...> arg_list{};

constexpr auto abs = [](const auto x) noexcept {
  using std::abs;
  return abs(x);
};

constexpr auto abs_out_arg =
    overloaded{arg_list<double, float>, [](auto &x, const auto y) {
                 using std::abs;
                 x = abs(y);
               }};

constexpr auto sqrt = [](const auto x) noexcept {
  using std::sqrt;
  return sqrt(x);
};

constexpr auto sqrt_out_arg =
    overloaded{arg_list<double, float>, [](auto &x, const auto y) {
                 using std::sqrt;
                 x = sqrt(y);
               }};

constexpr auto sin_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::sin;
      x = sin(y);
    }};

constexpr auto cos_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::cos;
      x = cos(y);
    }};

constexpr auto tan_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::tan;
      x = tan(y);
    }};

constexpr auto asin =
    overloaded{transform_flags::expect_no_variance_arg<0>, [](const auto x) {
                 using std::asin;
                 return asin(x);
               }};

constexpr auto asin_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::asin;
      x = asin(y);
    }};

constexpr auto acos =
    overloaded{transform_flags::expect_no_variance_arg<0>, [](const auto x) {
                 using std::acos;
                 return acos(x);
               }};

constexpr auto acos_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::acos;
      x = acos(y);
    }};

constexpr auto atan =
    overloaded{transform_flags::expect_no_variance_arg<0>, [](const auto x) {
                 using std::atan;
                 return atan(x);
               }};

constexpr auto atan_out_arg = overloaded{
    arg_list<double, float>, transform_flags::expect_no_variance_arg<0>,
    transform_flags::expect_no_variance_arg<1>, [](auto &x, const auto y) {
      using std::atan;
      x = atan(y);
    }};

constexpr auto nan_to_num = [](const auto x, const auto &repl) noexcept {
  using std::isnan;
  return isnan(x) ? repl : x;
};

constexpr auto nan_to_num_out_arg = [](auto &x, const auto y,
                                       const auto &repl) noexcept {
  using std::isnan;
  x = isnan(y) ? repl : y;
};

constexpr auto positive_inf_to_num = [](const auto &x, const auto &repl) {
  if constexpr (is_ValueAndVariance_v<std::decay_t<decltype(x)>>)
    return isinf(x) && x.value > 0 ? repl : x;
  else
    return std::isinf(x) && x > 0 ? repl : x;
};

constexpr auto positive_inf_to_num_out_arg = [](auto &x, const auto &y,
                                                const auto &repl) {
  if constexpr (is_ValueAndVariance_v<std::decay_t<decltype(y)>>)
    x = isinf(y) && y.value > 0 ? repl : y;
  else
    x = std::isinf(y) && y > 0 ? repl : y;
};

constexpr auto negative_inf_to_num = [](const auto &x, const auto &repl) {
  if constexpr (is_ValueAndVariance_v<std::decay_t<decltype(x)>>)
    return isinf(x) && x.value < 0 ? repl : x;
  else
    return std::isinf(x) && x < 0 ? repl : x;
};

constexpr auto negative_inf_to_num_out_arg = [](auto &x, const auto &y,
                                                const auto &repl) {
  if constexpr (is_ValueAndVariance_v<std::decay_t<decltype(y)>>)
    x = isinf(y) && y.value < 0 ? repl : y;
  else
    x = std::isinf(y) && y < 0 ? repl : y;
};

} // namespace element

} // namespace scipp::core

#endif // SCIPP_CORE_ELEMENT_UNARY_OPERATIONS_H
