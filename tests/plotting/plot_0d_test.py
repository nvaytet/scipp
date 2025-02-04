# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @file
# @author Owen Arnold

import scipp as sc
import pytest
from .plot_helper import plot
import matplotlib
matplotlib.use('Agg')


def test_plot_0d_variable():
    with pytest.raises(ValueError):
        plot(sc.scalar(1))


def test_plot_0d_dataarray():
    da = sc.DataArray(data=sc.scalar(1))
    with pytest.raises(ValueError):
        plot(da)


def test_plot_0d_dataset():
    ds = sc.Dataset(data={'a': sc.scalar(1)})
    with pytest.raises(ValueError):
        plot(ds)


def test_plot_not_all_0d_dataset():
    ds = sc.Dataset(data={
        'a': sc.scalar(1),
        'b': sc.Variable(dims=['x'], values=[1, 2])
    })
    plot(ds)  # Throw nothing


def test_dict_of_0d_variables():
    with pytest.raises(ValueError):
        plot({'a': sc.scalar(1)})


def test_dict_of_0d_data_array():
    with pytest.raises(ValueError):
        plot({'a': sc.DataArray(data=sc.scalar(1))})


def test_dict_of_0d_dataset():
    with pytest.raises(ValueError):
        plot({'a': sc.Dataset(data={'x': sc.scalar(1)})})


def test_dict_of_nd_variables():
    # 0D and 1D is OK
    plot({'a': sc.scalar(1), 'b': sc.Variable(dims=['x'], values=[1, 2])})


def test_dict_of_nd_data_array():
    # 0D and 1D is OK
    plot({
        'a': sc.DataArray(data=sc.scalar(1)),
        'b': sc.DataArray(data=sc.Variable(dims=['x'], values=[1, 2]))
    })


def test_dict_of_nd_dataset():
    # 0D and 1D is OK
    ds1 = sc.Dataset(data={'a': sc.Variable(dims=['x'], values=[1, 2])})
    ds2 = sc.Dataset(data={
        'a': sc.scalar(1),
    })
    plot({'x': ds1, 'y': ds2})
