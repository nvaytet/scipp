# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Simon Heybrock
from ._scipp import core as _cpp
from ._cpp_wrapper_util import call_func as _call_cpp_func


def less(x, y):
    """Element-wise '<' (less).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a < b`.
    """
    return _call_cpp_func(_cpp.less, x, y)


def greater(x, y):
    """Element-wise '>' (greater).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a > b`.
    """
    return _call_cpp_func(_cpp.greater, x, y)


def less_equal(x, y):
    """Element-wise '<=' (less_equal).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a <= b`.
    """
    return _call_cpp_func(_cpp.less_equal, x, y)


def greater_equal(x, y):
    """Element-wise '>=' (greater_equal).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a >= b`.
    """
    return _call_cpp_func(_cpp.greater_equal, x, y)


def equal(x, y):
    """Element-wise '==' (equal).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a == b`.
    """
    return _call_cpp_func(_cpp.equal, x, y)


def not_equal(x, y):
    """Element-wise '!=' (not_equal).

    Warning: If one or both of the operators have variances (uncertainties)
    there are ignored silently, i.e., comparison is based exclusively on
    the values.

    :param x: Left input.
    :param y: Right input.
    :raises: If the units of inputs are not the same, or if the dtypes of
             inputs cannot be compared.
    :return: Booleans that are true if `a != b`.
    """
    return _call_cpp_func(_cpp.not_equal, x, y)


def identical(x, y):
    """Full comparison of x and y.

    :param x: Left input.
    :param y: Right input.
    :return: True if x and y have identical values, variances, dtypes, units,
             dims, shapes, coords, and masks. Else False.
    """
    return _call_cpp_func(_cpp.identical, x, y)


def isclose(x, y, rtol=None, atol=None, equal_nan=False):
    """Compares values (x, y) element by element against tolerance absolute
    and relative tolerances (non-symmetric).

    abs(x - y) <= atol + rtol * abs(y)

    If both x and y have variances, the variances are also compared
    between elements. In this case, both values and variances must
    be within the computed tolerance limits. That is:

    .. code-block:: python

        abs(x.value - y.value) <= atol + rtol * abs(y.value) and abs(
            sqrt(x.variance) - sqrt(y.variance)) \
                <= atol + rtol * abs(sqrt(y.variance))

    :param x: Left input.
    :param y: Right input.
    :param rtol: Tolerance value relative (to y).
                 Can be a scalar or non-scalar.
                 Defaults to scalar 1e-5 if unset.
    :param atol: Tolerance value absolute. Can be a scalar or non-scalar.
                 Defaults to scalar 1e-8 if unset and takes units from y arg.
    :param equal_nan: if true, non-finite values at the same index in (x, y)
                      are treated as equal.
                      Signbit must match for infs.
    :type x: Variable
    :type y: Variable
    :type rtol: Variable. May be a scalar or an array variable.
                Cannot have variances.
    :type atol: Variable. May be a scalar or an array variable.
                Cannot have variances.
    :type equal_nan: bool
    :return: Variable same size as input.
             Element True if absolute diff of value <= atol + rtol * abs(y),
             otherwise False.
    """
    if rtol is None:
        rtol = 1e-5 * _cpp.units.one
    if atol is None:
        atol = 1e-8 * y.unit
    return _call_cpp_func(_cpp.isclose, x, y, rtol, atol, equal_nan)


def isnear(x,
           y,
           rtol=None,
           atol=None,
           include_attrs=True,
           include_data=True,
           equal_nan=True):
    """
    Similar to scipp isclose, but intended to compare whole DataArrays.
    Coordinates compared element by element
    with abs(x - y) <= atol + rtol * abs(y)

    Compared coord and attr pairs are only considered equal if all
    element-wise comparisons are True.

    See scipp isclose for more details on how the comparions on each
    item will be conducted.

    :param x: lhs input
    :param y: rhs input
    :param rtol: relative tolerance (to y)
    :param atol: absolute tolerance
    :param include_data: Compare data element-wise between x, and y
    :param include_attrs: Compare all meta (coords and attrs) between x and y,
           otherwise only compare coordinates from meta
    :param equal_nan: If True, consider Nans or Infs to be equal
                       providing that they match in location and, for infs,
                       have the same sign
    :type x: DataArray
    :type y: DataArray
    :type tol :
    :raises: If x, y are not going to be logically comparable
             for reasons relating to shape, item naming or non-finite elements.
    :return True if near:
    """
    same_data = sc.all(
        sc.isclose(x.data, y.data, rtol=rtol, atol=atol,
                   equal_nan=equal_nan)).value if include_data else True
    same_len = len(x.meta) == len(y.meta) if include_attrs else len(
        x.coords) == len(y.coords)
    if not same_len:
        raise RuntimeError('Different number of items'
                           f' in meta {len(x.meta)} {len(y.meta)}')
    for key, val in x.meta.items() if include_attrs else x.coords.items():
        a = x.meta[key] if include_attrs else x.coords[key]
        b = y.meta[key] if include_attrs else y.coords[key]
        if a.shape != b.shape:
            raise sc.CoordError(
                f'Coord (or attr) with key {key} have different'
                f' shapes. For x, shape is {a.shape}. For y, shape = {b.shape}'
            )
        if val.dtype in [sc.dtype.float64, sc.dtype.float32]:
            if not sc.all(
                    sc.isclose(a, b, rtol=rtol, atol=atol,
                               equal_nan=equal_nan)).value:
                return False
    return same_data
