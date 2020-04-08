// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#pragma once

#include <map>
#include <memory>
#include <string>

#include "scipp-core_export.h"
#include "scipp/common/index.h"
#include "scipp/core/dimensions.h"
#include "scipp/core/dtype.h"
#include "scipp/core/variable.h"

namespace scipp::core {

template <class Id, class Key, class Value> class ConstView;
template <class T, class U> class MutableView;

SCIPP_CORE_EXPORT std::ostream &operator<<(std::ostream &os,
                                           const Dimensions &dims);
SCIPP_CORE_EXPORT std::ostream &operator<<(std::ostream &os,
                                           const VariableConstView &variable);
SCIPP_CORE_EXPORT std::ostream &operator<<(std::ostream &os,
                                           const VariableView &variable);
SCIPP_CORE_EXPORT std::ostream &operator<<(std::ostream &os,
                                           const Variable &variable);

SCIPP_CORE_EXPORT const std::string &to_string(const std::string &s);
SCIPP_CORE_EXPORT std::string_view to_string(const std::string_view s);
SCIPP_CORE_EXPORT std::string to_string(const char *s);
SCIPP_CORE_EXPORT std::string to_string(const bool b);
SCIPP_CORE_EXPORT std::string to_string(const DType dtype);
SCIPP_CORE_EXPORT std::string to_string(const Dimensions &dims);
SCIPP_CORE_EXPORT std::string to_string(const Slice &slice);
SCIPP_CORE_EXPORT std::string to_string(const Variable &variable);
SCIPP_CORE_EXPORT std::string to_string(const VariableConstView &variable);

template <class Id, class Key, class Value>
std::string to_string(const ConstView<Id, Key, Value> &view) {
  std::stringstream ss;

  for (const auto &[key, item] : view) {
    ss << "<scipp.ConstView> (" << key << "):\n" << to_string(item);
  }
  return ss.str();
}

template <class T, class U>
std::string to_string(const MutableView<T, U> &mutableView) {
  std::stringstream ss;

  for (const auto &[key, item] : mutableView) {
    ss << "<scipp.MutableView> (" << key << "):\n" << to_string(item);
  }
  return ss.str();
}

template <class T> std::string array_to_string(const T &arr);

template <class T> std::string element_to_string(const T &item) {
  using std::to_string;
  if constexpr (std::is_same_v<T, std::string>)
    return {'"' + item + "\", "};
  else if constexpr (std::is_same_v<T, bool>)
    return core::to_string(item) + ", ";
  else if constexpr (std::is_same_v<T, Eigen::Vector3d>)
    return {"(" + to_string(item[0]) + ", " + to_string(item[1]) + ", " +
            to_string(item[2]) + "), "};
  else if constexpr (std::is_same_v<T, Eigen::Quaterniond>)
    return {"(" + to_string(item.x()) + ", " + to_string(item.y()) + ", " +
            to_string(item.z()) + ", " + to_string(item.w()) + "), "};
  else if constexpr (is_sparse_v<T>)
    return array_to_string(item) + ", ";
  else
    return to_string(item) + ", ";
}

template <class T> std::string array_to_string(const T &arr) {
  const auto size = scipp::size(arr);
  if (size == 0)
    return std::string("[]");
  std::string s = "[";
  for (scipp::index i = 0; i < scipp::size(arr); ++i) {
    if (i == 2 && size > 4) {
      s += "..., ";
      i = size - 2;
    }
    s += element_to_string(arr[i]);
  }
  s.resize(s.size() - 2);
  s += "]";
  return s;
}

SCIPP_CORE_EXPORT std::string
format_variable(const std::string &key, const VariableConstView &variable,
                const Dimensions &datasetDims = Dimensions());

/// Abstract base class for formatters for variables with element types not in
/// scipp-core module.
class SCIPP_CORE_EXPORT AbstractFormatter {
public:
  virtual ~AbstractFormatter() = default;
  virtual std::string format(const VariableConstView &var) const = 0;
};

/// Concrete class for formatting variables with element types not in
/// scipp-core.
template <class T> class Formatter : public AbstractFormatter {
  std::string format(const VariableConstView &var) const override {
    return array_to_string(var.template values<T>());
  }
};

/// Registry of formatters.
///
/// Modules instantiating variables with custom dtype can call `emplace` to
/// register a formatter.
class SCIPP_CORE_EXPORT FormatterRegistry {
public:
  void emplace(const DType key, std::unique_ptr<AbstractFormatter> formatter);
  bool contains(const DType key) const noexcept;
  std::string format(const DType key, const VariableConstView &var) const;

private:
  std::map<DType, std::unique_ptr<AbstractFormatter>> m_formatters;
};

/// Return the global FormatterRegistry instrance
SCIPP_CORE_EXPORT FormatterRegistry &formatterRegistry();

} // namespace scipp::core
