// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
/// @file
/// @author Simon Heybrock
#include "scipp/dataset/bucket.h"
#include "scipp/dataset/dataset.h"
#include "scipp/dataset/string.h"
#include "scipp/variable/bucket_variable.tcc"
#include "scipp/variable/buckets.h"
#include "scipp/variable/string.h"

namespace scipp::variable {

INSTANTIATE_BUCKET_VARIABLE(DatasetView, bucket<Dataset>)
INSTANTIATE_BUCKET_VARIABLE(DataArrayView, bucket<DataArray>)

} // namespace scipp::variable

namespace scipp::dataset {
class BucketVariableMakerDataArray
    : public variable::BucketVariableMaker<DataArray> {
private:
  Variable make_buckets(const VariableConstView &parent,
                        const VariableConstView &indices, const Dim dim,
                        const DType type, const Dimensions &dims,
                        const units::Unit &unit,
                        const bool variances) const override {
    const auto &source = std::get<2>(parent.constituents<bucket<DataArray>>());
    if (parent.dims() !=
        indices
            .dims()) // would need to select and copy slices from source coords
      throw std::runtime_error(
          "Shape changing operations with bucket<DataArray> not supported yet");
    auto buffer = DataArray(
        variable::variableFactory().create(type, dims, unit, variances),
        source.aligned_coords(), source.masks(), source.unaligned_coords());
    return Variable{std::make_unique<variable::DataModel<bucket<DataArray>>>(
        indices, dim, std::move(buffer))};
  }
  VariableConstView data(const VariableConstView &var) const override {
    return std::get<2>(var.constituents<bucket<DataArray>>()).data();
  }
  VariableView data(const VariableView &var) const override {
    return std::get<2>(var.constituents<bucket<DataArray>>()).data();
  }
  core::element_array_view
  array_params(const VariableConstView &var) const override {
    const auto &[indices, dim, buffer] = var.constituents<bucket<DataArray>>();
    auto params = var.array_params();
    return {0, // no offset required in buffer since access via indices
            params.dims(),
            params.dataDims(),
            {dim, buffer.dims(),
             indices.values<std::pair<scipp::index, scipp::index>>().data()}};
  }
};

namespace {
auto register_dataset_types(
    (variable::formatterRegistry().emplace(
         dtype<bucket<Dataset>>,
         std::make_unique<variable::Formatter<bucket<Dataset>>>()),
     variable::formatterRegistry().emplace(
         dtype<bucket<DataArray>>,
         std::make_unique<variable::Formatter<bucket<DataArray>>>()),
     0));
auto register_variable_maker_bucket_DataArray(
    (variable::variableFactory().emplace(
         dtype<bucket<DataArray>>,
         std::make_unique<BucketVariableMakerDataArray>()),
     0));
} // namespace
} // namespace scipp::dataset
