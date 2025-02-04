// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/variable/bin_array_variable.tcc"

namespace scipp::variable::bin_array_variable_detail {

std::tuple<Variable, scipp::index> contiguous_indices(const Variable &parent,
                                                      const Dimensions &dims) {
  auto indices = Variable(parent, dims);
  copy(parent, indices);
  scipp::index size = 0;
  for (auto &range : indices.values<scipp::index_pair>()) {
    range.second += size - range.first;
    range.first = size;
    size = range.second;
  }
  return std::tuple{indices, size};
}

const scipp::index_pair *index_pair_data(const Variable &indices) {
  return indices.template values<scipp::index_pair>().data();
}

scipp::index size_from_end_index(const Variable &end) {
  return end.dims().volume() > 0 ? end.values<scipp::index>().as_span().back()
                                 : 0;
}

const scipp::index &index_value(const Variable &index) {
  return index.value<scipp::index>();
}

VariableConceptHandle zero_indices(const scipp::index size) {
  return makeVariable<scipp::index_pair>(Dims{Dim::X}, Shape{size})
      .data_handle();
}

} // namespace scipp::variable::bin_array_variable_detail
