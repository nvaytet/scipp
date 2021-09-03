// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/core/element/creation.h"
#include "scipp/core/time_point.h"
#include "scipp/variable/creation.h"
#include "scipp/variable/shape.h"
#include "scipp/variable/transform.h"
#include "scipp/variable/variable_factory.h"

namespace scipp::variable {

Variable empty(const Dimensions &dims, const units::Unit &unit,
               const DType type, const bool with_variances) {
  return variableFactory().create(type, dims, unit, with_variances);
}

Variable ones(const Dimensions &dims, const units::Unit &unit, const DType type,
              const bool with_variances) {
  const auto make_prototype = [&](auto &&one) {
    return with_variances
               ? Variable{type, Dimensions{}, unit, Values{one}, Variances{one}}
               : Variable{type, Dimensions{}, unit, Values{one}};
  };
  if (type == dtype<core::time_point>) {
    return copy(broadcast(make_prototype(core::time_point{1}), dims));
  } else if (type == dtype<std::string>) {
    // This would result in a Variable containing (char)1.
    throw std::invalid_argument("Cannot construct 'ones' of strings.");
  } else {
    return copy(broadcast(make_prototype(1), dims));
  }
}

/// Create empty (uninitialized) variable with same parameters as prototype.
///
/// If specified, `shape` defines the shape of the output. If `prototype`
/// contains binned data `shape` may not be specified, instead `sizes` defines
/// the sizes of the desired bins.
Variable empty_like(const Variable &prototype,
                    const std::optional<Dimensions> &shape,
                    const Variable &sizes) {
  return variableFactory().empty_like(prototype, shape, sizes);
}

Variable special_like(const Variable &prototype, const FillValue &fill) {
  const char *name = "special_like";
  if (fill == FillValue::Default)
    return Variable(prototype, prototype.dims());
  if (fill == FillValue::ZeroNotBool)
    return transform(prototype, core::element::zeros_not_bool_like, name);
  if (fill == FillValue::True)
    return transform(prototype, core::element::values_like<bool, true>, name);
  if (fill == FillValue::False)
    return transform(prototype, core::element::values_like<bool, false>, name);
  if (fill == FillValue::Max)
    return transform(prototype, core::element::numeric_limits_max_like, name);
  if (fill == FillValue::Lowest)
    return transform(prototype, core::element::numeric_limits_lowest_like,
                     name);
  throw std::runtime_error("Unsupported fill value.");
}

/// Create scalar variable containing 0 with same parameters as prototype.
Variable zero_like(const Variable &prototype) {
  return {prototype, Dimensions{}};
}

} // namespace scipp::variable
