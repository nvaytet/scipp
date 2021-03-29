# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

from .._scipp import core as sc


def to_bin_centers(x, dim):
    """
    Convert bin edges to centers by computing a simple arithmetic mean.

    :param [x]: Variable that contains bin edges.
    :type [x]: Variable
    :param [dim]: Dimension along which to convert to bin centers.
    :type [dim]: str
    :return: A Variable containing the bin centers.
    :rtype: Variable
    """
    return 0.5 * (x[dim, 1:] + x[dim, :-1])


def to_bin_edges(x, dim):
    """
    Convert bin centers to bin edges.
    The start and end edges are calculated so that the first two bins, and the
    last two bins, have the same width, respectively.

    :param [x]: Variable that contains bin edges.
    :type [x]: Variable
    :param [dim]: Dimension along which to convert to bin edges.
    :type [dim]: str
    :return: A Variable containing the bin edges.
    :rtype: Variable
    """
    idim = x.dims.index(dim)
    if x.shape[idim] < 2:
        one = 1.0 * x.unit
        return sc.concatenate(x[dim, 0:1] - one, x[dim, 0:1] + one, dim)
    else:
        center = to_bin_centers(x, dim)
        # Note: use range of 0:1 to keep dimension dim in the slice to avoid
        # switching round dimension order in concatenate step.
        left = center[dim, 0:1] - (x[dim, 1] - x[dim, 0])
        right = center[dim, -1] + (x[dim, -1] - x[dim, -2])
        return sc.concatenate(sc.concatenate(left, center, dim), right, dim)
