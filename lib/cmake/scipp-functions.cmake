# ~~~
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# ~~~
include(scipp-util)

set(convert_to_rad
    "#include \"scipp/variable/to_unit.h\"
namespace {
decltype(auto) preprocess(const scipp::variable::Variable &var) {
  return to_unit(var, scipp::units::rad)\;
}
}"
)

scipp_unary(math abs OUT)
scipp_unary(math exp OUT)
scipp_unary(math log OUT)
scipp_unary(math log10 OUT)
scipp_unary(math reciprocal OUT)
scipp_unary(math sqrt OUT)
scipp_unary(math norm)
scipp_unary(math floor OUT)
scipp_unary(math ceil OUT)
scipp_unary(math rint OUT)
scipp_unary(math erf)
scipp_unary(math erfc)
scipp_binary(math pow SKIP_VARIABLE OUT)
scipp_binary(math dot)
scipp_binary(math cross)
setup_scipp_category(math)

scipp_unary(util values)
scipp_unary(util variances)
scipp_unary(util stddevs)
setup_scipp_category(util)

scipp_unary(trigonometry sin PREPROCESS_VARIABLE "${convert_to_rad}" OUT)
scipp_unary(trigonometry cos PREPROCESS_VARIABLE "${convert_to_rad}" OUT)
scipp_unary(trigonometry tan PREPROCESS_VARIABLE "${convert_to_rad}" OUT)
scipp_unary(trigonometry asin OUT)
scipp_unary(trigonometry acos OUT)
scipp_unary(trigonometry atan OUT)
scipp_binary(trigonometry atan2 OUT)
setup_scipp_category(trigonometry OUT)

scipp_unary(special_values isnan)
scipp_unary(special_values isinf)
scipp_unary(special_values isfinite)
scipp_unary(special_values isposinf)
scipp_unary(special_values isneginf)
setup_scipp_category(special_values)

scipp_binary(comparison equal)
scipp_binary(comparison greater)
scipp_binary(comparison greater_equal)
scipp_binary(comparison less)
scipp_binary(comparison less_equal)
scipp_binary(comparison not_equal)
setup_scipp_category(comparison)

scipp_function("unary" arithmetic operator- OP unary_minus)
scipp_function("binary" arithmetic operator+ OP add)
scipp_function("binary" arithmetic operator- OP subtract)
scipp_function("binary" arithmetic operator* OP multiply)
scipp_function("binary" arithmetic operator/ OP divide)
scipp_function("binary" arithmetic floor_divide)
scipp_function("binary" arithmetic operator% OP mod)
scipp_function("inplace" arithmetic operator+= OP add_equals)
scipp_function("inplace" arithmetic operator-= OP subtract_equals)
scipp_function("inplace" arithmetic operator*= OP multiply_equals)
scipp_function("inplace" arithmetic operator/= OP divide_equals)
scipp_function("inplace" arithmetic operator%= OP mod_equals)
setup_scipp_category(arithmetic)

scipp_function("unary" logical operator~ OP logical_not)
scipp_function("binary" logical operator| OP logical_or)
scipp_function("binary" logical operator& OP logical_and)
scipp_function("binary" logical operator^ OP logical_xor)
scipp_function("inplace" logical operator|= OP logical_or_equals)
scipp_function("inplace" logical operator&= OP logical_and_equals)
scipp_function("inplace" logical operator^= OP logical_xor_equals)
setup_scipp_category(logical)

scipp_function("unary" bins bin_sizes SKIP_VARIABLE BASE_INCLUDE dataset/bins.h)
scipp_function("unary" bins bins_sum SKIP_VARIABLE BASE_INCLUDE dataset/bins.h)
scipp_function("unary" bins bins_mean SKIP_VARIABLE BASE_INCLUDE dataset/bins.h)
setup_scipp_category(bins)
