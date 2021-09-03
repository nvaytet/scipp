// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
/// @author Neil Vaytet
#include <regex>
#include <stdexcept>

#include <units/units.hpp>
#include <units/units_util.hpp>

#include "scipp/units/except.h"
#include "scipp/units/unit.h"

namespace scipp::units {

namespace {
std::string map_unit_string(const std::string &unit) {
  // custom dimensionless name
  return unit == "dimensionless"
             ? ""
             // Use Gregorian months and years by default.
             : unit == "y" || unit == "Y" || unit == "year"
                   ? "a_g"
                   // Overwrite M to mean month instead of molarity for numpy
                   // interop.
                   : unit == "M" || unit == "month" ? "mog" : unit;
}
} // namespace

Unit::Unit(const std::string &unit)
    : Unit(llnl::units::unit_from_string(map_unit_string(unit))) {
  if (!is_valid(m_unit))
    throw except::UnitError("Failed to convert string `" + unit +
                            "` to valid unit.");
}

std::string Unit::name() const {
  if (*this == Unit{"month"}) {
    return "M";
  }
  auto repr = to_string(m_unit);
  repr = std::regex_replace(repr, std::regex("^u"), "µ");
  repr = std::regex_replace(repr, std::regex("item"), "count");
  repr = std::regex_replace(repr, std::regex("count(?!s)"), "counts");
  repr = std::regex_replace(repr, std::regex("day"), "D");
  repr = std::regex_replace(repr, std::regex("a_g"), "Y");
  return repr.empty() ? "dimensionless" : repr;
}

bool Unit::isCounts() const { return *this == counts; }

bool Unit::isCountDensity() const {
  return !isCounts() && m_unit.base_units().count() != 0;
}

bool Unit::has_same_base(const Unit &other) const {
  return m_unit.has_same_base(other.underlying());
}

bool Unit::operator==(const Unit &other) const {
  return m_unit == other.m_unit;
}
bool Unit::operator!=(const Unit &other) const { return !(*this == other); }

Unit &Unit::operator+=(const Unit &other) { return *this = *this + other; }

Unit &Unit::operator-=(const Unit &other) { return *this = *this - other; }

Unit &Unit::operator*=(const Unit &other) { return *this = *this * other; }

Unit &Unit::operator/=(const Unit &other) { return *this = *this / other; }

Unit &Unit::operator%=([[maybe_unused]] const Unit &other) { return *this; }

Unit operator+(const Unit &a, const Unit &b) {
  if (a == b)
    return a;
  throw except::UnitError("Cannot add " + a.name() + " and " + b.name() + ".");
}

Unit operator-(const Unit &a, const Unit &b) {
  if (a == b)
    return a;
  throw except::UnitError("Cannot subtract " + a.name() + " and " + b.name() +
                          ".");
}

Unit operator*(const Unit &a, const Unit &b) {
  if (llnl::units::times_overflows(a.underlying(), b.underlying()))
    throw except::UnitError("Unsupported unit as result of multiplication: (" +
                            a.name() + ") * (" + b.name() + ')');
  return Unit{a.underlying() * b.underlying()};
}

Unit operator/(const Unit &a, const Unit &b) {
  if (llnl::units::divides_overflows(a.underlying(), b.underlying()))
    throw except::UnitError("Unsupported unit as result of division: (" +
                            a.name() + ") / (" + b.name() + ')');
  return Unit{a.underlying() / b.underlying()};
}

Unit operator%(const Unit &a, [[maybe_unused]] const Unit &b) { return a; }

Unit operator-(const Unit &a) { return a; }

Unit abs(const Unit &a) { return a; }

Unit floor(const Unit &a) { return a; }

Unit ceil(const Unit &a) { return a; }

Unit rint(const Unit &a) { return a; }

Unit sqrt(const Unit &a) {
  if (llnl::units::is_error(sqrt(a.underlying())))
    throw except::UnitError("Unsupported unit as result of sqrt: sqrt(" +
                            a.name() + ").");
  return Unit{sqrt(a.underlying())};
}

Unit pow(const Unit &a, const int64_t power) {
  if (llnl::units::pow_overflows(a.underlying(), power))
    throw except::UnitError("Unsupported unit as result of pow: pow(" +
                            a.name() + ", " + std::to_string(power) + ").");
  return Unit{a.underlying().pow(power)};
}

Unit trigonometric(const Unit &a) {
  if (a == units::rad || a == units::deg)
    return units::dimensionless;
  throw except::UnitError(
      "Trigonometric function requires rad or deg unit, got " + a.name() + ".");
}

Unit inverse_trigonometric(const Unit &a) {
  if (a == units::dimensionless)
    return units::rad;
  throw except::UnitError(
      "Inverse trigonometric function requires dimensionless unit, got " +
      a.name() + ".");
}

Unit sin(const Unit &a) { return trigonometric(a); }
Unit cos(const Unit &a) { return trigonometric(a); }
Unit tan(const Unit &a) { return trigonometric(a); }
Unit asin(const Unit &a) { return inverse_trigonometric(a); }
Unit acos(const Unit &a) { return inverse_trigonometric(a); }
Unit atan(const Unit &a) { return inverse_trigonometric(a); }
Unit atan2(const Unit &y, const Unit &x) {
  if (x == y)
    return units::rad;
  throw except::UnitError(
      "atan2 function requires matching units for input, got a " + x.name() +
      " b " + y.name() + ".");
}

} // namespace scipp::units
