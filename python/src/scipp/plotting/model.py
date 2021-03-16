# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

from .helpers import PlotArray
from .tools import to_bin_edges, to_bin_centers, make_fake_coord, \
                   vars_to_err, find_limits
from .._utils import name_with_unit, value_to_string
from .._scipp import core as sc
import numpy as np


class PlotModel:
    """
    Base class for `model`.

    Upon creation, it:
    - makes a copy of the input data array (including masks)
    - units of the original data are saved, and units of the copy are set to
        counts, because `rebin` is used for resampling and only accepts counts.
    - it replaces all coordinates with corresponding bin-edges, which allows
        for much more generic plotting code
    - coordinates that contain strings or vectors are converted to fake
        integer coordinates, and axes formatters are updated with lambda
        function formatters.

    The model is where all operations on the data (slicing and resampling) are
    performed.
    """
    def __init__(self,
                 scipp_obj_dict=None,
                 name=None,
                 axes=None,
                 dim_to_shape=None,
                 dim_label_map=None):

        self.interface = {}

        # The main container of DataArrays
        self.data_arrays = {}
        self.coord_info = {}
        self.dim_to_shape = dim_to_shape
        self.coord_offset = {}

        self.axformatter = {}

        axes_dims = list(axes.values())

        # Create dict of DataArrays using information from controller
        for name, array in scipp_obj_dict.items():

            # Store axis tick formatters
            self.axformatter[name] = {}
            self.coord_info[name] = {}
            self.coord_offset[name] = {}
            coord_list = {}

            # Iterate through axes and collect coordinates
            for dim in axes_dims:

                coord, formatter, label, unit, offset = self._axis_coord_and_formatter(
                    dim, array, self.dim_to_shape[name], dim_label_map)

                self.axformatter[name][dim] = formatter
                self.coord_info[name][dim] = {"label": label, "unit": unit}
                self.coord_offset[name][dim] = offset

                is_histogram = False
                for i, d in enumerate(coord.dims):
                    if d == dim:
                        is_histogram = self.dim_to_shape[name][
                            d] == coord.shape[i] - 1

                if is_histogram:
                    coord_list[dim] = coord
                else:
                    coord_list[dim] = to_bin_edges(coord, dim)

            # Create a PlotArray helper object that supports slicing where new
            # bin-edge coordinates can be attached to the data
            self.data_arrays[name] = PlotArray(data=array.data,
                                               meta=coord_list)

            # Include masks
            for m in array.masks:
                self.data_arrays[name].masks[m] = array.masks[m]

        # Store dim of multi-dimensional coordinate if present
        self.multid_coord = None
        for array in self.data_arrays.values():
            for dim, coord in array.meta.items():
                if len(coord.dims) > 1:
                    self.multid_coord = dim

        # The main currently displayed data slice
        self.dslice = None
        # Save a copy of the name for simpler access
        self.name = name

    def _axis_coord_and_formatter(self, dim, data_array, dim_to_shape,
                                  dim_label_map):
        """
        Get dimensions from requested axis.
        Also retun axes tick formatters and locators.
        """

        # Create some default axis tick formatter, depending on linear or log
        # scaling.
        formatter = {"linear": None, "log": None, "custom_locator": None}

        contains_strings = False
        contains_vectors = False
        contains_datetime = False
        offset = 0.0
        coord_label = None
        form = None

        has_no_coord = dim not in data_array.meta
        if not has_no_coord:
            if data_array.meta[dim].dtype == sc.dtype.vector_3_float64:
                contains_vectors = True
            elif data_array.meta[dim].dtype == sc.dtype.string:
                contains_strings = True
            elif data_array.meta[dim].dtype == sc.dtype.datetime64:
                contains_datetime = True

        # Get the coordinate from the DataArray or generate a fake one
        if has_no_coord or contains_vectors or contains_strings:
            coord = make_fake_coord(dim, dim_to_shape[dim] + 1)
            if not has_no_coord:
                coord.unit = data_array.meta[dim].unit
        elif contains_datetime:
            nanoseconds = data_array.meta[dim].values.astype(
                'datetime64[ns]').astype(np.int64)
            offset = np.amin(nanoseconds)
            nanoseconds -= offset
            coord = sc.Variable(dims=data_array.meta[dim].dims,
                                unit=data_array.meta[dim].unit,
                                values=nanoseconds,
                                dtype=sc.dtype.int64)
        else:
            coord = data_array.meta[dim]
            if (coord.dtype != sc.dtype.float32) and (coord.dtype !=
                                                      sc.dtype.float64):
                coord = coord.astype(sc.dtype.float64)

        # Set up tick formatters
        if dim in dim_label_map:
            key = dim_label_map[dim]
        else:
            key = dim

        if data_array.meta[key].dtype == sc.dtype.vector_3_float64:
            # If the non-dimension coordinate contains vectors
            form = self._vector_tick_formatter(
                data_array.meta[key].values,
                dim_to_shape[dim])
            formatter["custom_locator"] = True
        elif data_array.meta[key].dtype == sc.dtype.string:
            # If the non-dimension coordinate contains strings
            form = self._string_tick_formatter(
                data_array.meta[key].values,
                dim_to_shape[dim])
            formatter["custom_locator"] = True
        elif data_array.meta[key].dtype == sc.dtype.datetime64:
            form = self._date_tick_formatter(offset, key)
        elif dim in dim_label_map:
            coord_values = coord.values
            if has_no_coord:
                # In this case we always have a bin-edge coord
                coord_values = to_bin_centers(coord, dim).values
            else:
                if data_array.meta[dim].shape[-1] == dim_to_shape[dim]:
                    coord_values = to_bin_centers(coord, dim).values
            form = lambda val, pos: value_to_string(  # noqa: E731
                data_array.meta[dim_label_map[dim]].values[np.abs(
                    coord_values - val).argmin()])

        if form is not None:
            formatter.update({"linear": form, "log": form})

        if dim in dim_label_map:
            coord_label = name_with_unit(
                var=data_array.meta[dim_label_map[dim]],
                name=dim_label_map[dim])
            coord_unit = name_with_unit(
                var=data_array.meta[dim_label_map[dim]], name="")
        else:
            coord_label = name_with_unit(var=coord)
            coord_unit = name_with_unit(var=coord, name="")

        return coord, formatter, coord_label, coord_unit, offset

    def _vector_tick_formatter(self, array_values, size):
        """
        Format vector output for ticks: return 3 components as a string.
        """
        return lambda val, pos: "(" + ",".join([
            value_to_string(item, precision=2)
            for item in array_values[int(val)]
        ]) + ")" if (int(val) >= 0 and int(val) < size) else ""

    def _string_tick_formatter(self, array_values, size):
        """
        Format string ticks: find closest string in coordinate array.
        """
        return lambda val, pos: array_values[int(val)] if (int(
            val) >= 0 and int(val) < size) else ""

    def _date_tick_formatter(self, offset, dim):
        """
        Format datetime ticks: adjust the time precision and update
        offset according to the currently displayed range.
        """
        def formatter(val, pos):
            dt = str(np.datetime64(int(val) + int(offset), 'ns'))
            start = 0
            end = len(dt)
            u = ""
            suffix = ""
            bounds = self.interface["get_view_axis_bounds"](dim)
            diff = bounds[1] - bounds[0]
            if diff < 2e3:  # offset: 2017-01-13T12:15:45.123456, tick: 789 ns
                start = 26
                u = "ns"
            elif diff < 2e6:  # offset: 2017-01-13T12:15:45.123, tick: 456 us
                start = 23
                end = 26
                u = "us"
            elif diff < 2e9:  # offset: 2017-01-13T12:15, tick: 45.123 s
                start = 17
                end = 23
                u = "s"
            elif diff < 60 * 2e9:  # offset: 2017-01-13, tick: 12:15:45
                start = 11
                end = 19
            elif diff < 3600 * 2e9:  # offset: 2017-01-13, tick: 12:15
                start = 11
                end = 16
            elif diff < 86400 * 2e9:  # offset: 2017-01, tick: 13 12h
                start = 8
                end = 13
                suffix = "h"
            elif diff < 86400 * 6e9 * 30:  # tick: 2017-01-13
                end = 10
            else:  # tick: 2017-01
                end = 7

            if pos == 0:
                self.interface["set_view_axis_offset"](
                    dim, dt[:start].strip('T').strip(':').strip('-'))
                label = dim
                if len(u) > 0:
                    label += " [{}]".format(u)
                self.interface["set_view_axis_label"](dim, label)

            return (dt[start:end] + suffix).replace("T", " ")

        return formatter

    def _make_masks(self, array, mask_info, transpose=False):
        if not mask_info:
            return {}
        masks = {}
        data = array.data
        base_mask = sc.Variable(dims=data.dims,
                                values=np.ones(data.shape, dtype=np.int32))
        for m in mask_info:
            if m in array.masks:
                msk = base_mask * sc.Variable(dims=array.masks[m].dims,
                                              values=array.masks[m].values)
                masks[m] = msk.values
                if transpose:
                    masks[m] = np.transpose(masks[m])
            else:
                masks[m] = None
        return masks

    def _make_profile(self, profile, dim, mask_info):
        values = {"values": {}, "variances": {}, "masks": {}}
        values["values"]["x"] = profile.meta[dim].values.ravel()
        values["values"]["y"] = profile.data.values.ravel()
        if profile.data.variances is not None:
            values["variances"]["e"] = vars_to_err(
                profile.data.variances.ravel())
        values["masks"] = self._make_masks(profile, mask_info=mask_info)
        return values

    def get_axformatter(self, name, dim):
        """
        Get an axformatter for a given data name and dimension.
        """
        return self.axformatter[name][dim]

    def get_slice_coord_bounds(self, name, dim, bounds):
        """
        Return the left, center, and right coordinates for a bin index.
        """
        return self.data_arrays[name].meta[dim][
            dim,
            bounds[0]].value, self.data_arrays[name].meta[dim][dim,
                                                               bounds[1]].value

    def get_data_names(self):
        """
        List all names in dict of data arrays.
        This is usually only a single name, but can be more than one for 1d
        plots.
        """
        return list(self.data_arrays.keys())

    def get_data_coord(self, name, dim):
        """
        Get a coordinate along a requested dimension.
        """
        return self.data_arrays[name].meta[dim], self.coord_info[name][dim][
            "label"], self.coord_info[name][dim]["unit"]

    def rescale_to_data(self, scale=None):
        """
        Get the min and max values of the currently displayed slice.
        """
        if self.dslice is not None:
            return find_limits(self.dslice.data, scale=scale)[scale]
        else:
            return [None, None]

    def slice_data(self, array, slices, keep_dims=False):
        """
        Slice the data array according to the dimensions and extents listed
        in slices.
        """
        for dim, [lower, upper] in slices.items():
            # TODO: Could this be optimized for performance?
            # Note: we use the range 1 [dim, i:i+1] slicing here instead of
            # index slicing [dim, i] so that we hit the correct branch in
            # rebin, because in the case of slicing an outer dim, rebin-inner
            # cannot deal with non-continuous data as an input.
            array = array[dim, lower:upper]
            if (upper - lower) > 1:
                array.data = sc.rebin(
                    array.data, dim, array.meta[dim],
                    sc.concatenate(array.meta[dim][dim, 0],
                                   array.meta[dim][dim, -1], dim))
            if not keep_dims:
                array = array[dim, 0]
        return array

    def get_multid_coord(self):
        """
        Return the multi-dimensional coordinate.
        """
        return self.multid_coord

    def update_profile_model(self, *args, **kwargs):
        return

    def connect(self, callbacks):
        for name, func in callbacks.items():
            self.interface[name] = func
