// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>

#include "scipp/units/unit.h"

#include "scipp/core/dimensions.h"
#include "scipp/core/except.h"
#include "scipp/core/slice.h"
#include "scipp/core/string.h"
#include "scipp/core/time_point.h"

namespace scipp::core {

std::ostream &operator<<(std::ostream &os, const Dimensions &dims) {
  return os << to_string(dims);
}

std::string to_string(const Dimensions &dims) {
  if (dims.empty())
    return "()";
  std::string s = "(";
  for (int32_t i = 0; i < scipp::size(dims.shape()); ++i)
    s += to_string(dims.labels()[i]) + ": " + std::to_string(dims.shape()[i]) +
         ", ";
  s.resize(s.size() - 2);
  s += ")";
  return s;
}

const std::string &to_string(const std::string &s) { return s; }
std::string_view to_string(const std::string_view s) { return s; }
std::string to_string(const char *s) { return std::string(s); }

std::string to_string(const bool b) { return b ? "True" : "False"; }

std::string to_string(const DType dtype) {
  return dtypeNameRegistry().at(dtype);
}

std::string to_string(const Slice &slice) {
  std::string end = slice.end() >= 0 ? ", " + std::to_string(slice.end()) : "";
  return "Slice(" + to_string(slice.dim()) + ", " +
         std::to_string(slice.begin()) + end + ")\n";
}

std::string to_string(const scipp::index_pair &index) {
  return '(' + std::to_string(index.first) + ", " +
         std::to_string(index.second) + ')';
}

std::map<DType, std::string> &dtypeNameRegistry() {
  static std::map<DType, std::string> registry;
  return registry;
}

namespace {
template <class Ratio> constexpr int64_t num_digits() {
  static_assert(Ratio::num == 1 || Ratio::num % 10 == 0);
  static_assert(Ratio::den == 1 || Ratio::den % 10 == 0);
  static_assert(Ratio::den > Ratio::num);
  int64_t result = 0;
  for (std::size_t i = Ratio::num; i < Ratio::den; i *= 10) {
    ++result;
  }
  return result;
}

// For synchronizing access to gmtime because its return value is shared.
std::mutex gmtime_mutex;

void put_time(std::ostream &os, const std::time_t time_point,
              const bool include_time) {
  std::lock_guard guard_{gmtime_mutex};
  const std::tm *tm = std::gmtime(&time_point);
  if (include_time)
    os << std::put_time(tm, "%FT%T");
  else
    os << std::put_time(tm, "%F");
}

template <class Rep, class Period>
std::string to_string(const std::chrono::duration<Rep, Period> &duration) {
  using Clock = std::chrono::system_clock;

  std::ostringstream oss;
  // Cast to seconds to be independent of clock precision.
  // Sub-second digits are formatted manually.
  put_time(oss,
           Clock::to_time_t(Clock::time_point{
               std::chrono::duration_cast<std::chrono::seconds>(duration)}),
           true);
  if constexpr (std::ratio_less_v<Period, std::ratio<1, 1>>) {
    oss << '.' << std::setw(num_digits<Period>()) << std::setfill('0')
        << (duration.count() % (Period::den / Period::num));
  }
  return oss.str();
}

std::string to_string(const std::chrono::days &duration) {
  using Clock = std::chrono::system_clock;
  std::ostringstream oss;
  put_time(oss, Clock::to_time_t(Clock::time_point{duration}), false);
  return oss.str();
}

constexpr std::chrono::year_month_day epoch{
    std::chrono::year{1970}, std::chrono::month{1}, std::chrono::day{1}};

auto normalize(const int64_t years_since_epoch,
               const int64_t months_since_epoch) {
  const int64_t absolute_year =
      years_since_epoch + static_cast<int>(epoch.year());
  const int64_t absolute_month =
      months_since_epoch + static_cast<unsigned int>(epoch.month());
  if (absolute_month > 0)
    return std::pair{absolute_year, absolute_month};
  else
    return std::pair{absolute_year - 1, absolute_month + 12};
}

/*
 * Custom implementations for months and years because we cannot construct
 * a std::chrono::time_point with an exact number of months and years since
 * epoch because std::chrono::duration uses average months / years.
 */
std::string to_string(const std::chrono::months &duration) {
  const auto years_since_epoch = static_cast<int64_t>(duration.count()) / 12;
  const auto months_since_epoch =
      static_cast<int64_t>(duration.count()) - years_since_epoch * 12;
  const auto [year, month] = normalize(years_since_epoch, months_since_epoch);
  std::ostringstream oss;
  oss << std::setw(4) << std::setfill('0') << year << '-' << std::setw(2)
      << std::setfill('0') << month;
  return oss.str();
}

std::string to_string(const std::chrono::years &duration) {
  const auto years_since_epoch = duration.count();
  const auto year = years_since_epoch + static_cast<int>(epoch.year());
  std::ostringstream oss;
  oss << std::setw(4) << std::setfill('0') << year;
  return oss.str();
}
} // namespace

std::string to_iso_date(const scipp::core::time_point &item,
                        const units::Unit &unit) {
  if (unit == units::ns) {
    return to_string(std::chrono::nanoseconds{item.time_since_epoch()});
  } else if (unit == units::s) {
    return to_string(std::chrono::seconds{item.time_since_epoch()});
  } else if (unit == units::us) {
    return to_string(std::chrono::microseconds{item.time_since_epoch()});
  } else if (unit == units::Unit(llnl::units::precise::ms)) {
    return to_string(std::chrono::milliseconds{item.time_since_epoch()});
  } else if (unit == units::Unit(llnl::units::precise::min)) {
    return to_string(std::chrono::minutes{item.time_since_epoch()});
  } else if (unit == units::Unit(llnl::units::precise::hr)) {
    return to_string(std::chrono::hours{item.time_since_epoch()});
  } else if (unit == units::Unit(llnl::units::precise::day)) {
    return to_string(std::chrono::days{item.time_since_epoch()});
  } else if (unit == units::Unit("month")) {
    return to_string(std::chrono::months{item.time_since_epoch()});
  } else if (unit == units::Unit("year")) {
    return to_string(std::chrono::years{item.time_since_epoch()});
  }
  throw except::UnitError("Cannot display time point, unsupported unit: " +
                          to_string(unit));
}
} // namespace scipp::core
