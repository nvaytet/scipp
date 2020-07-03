# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

from .. import _utils as su
from .._scipp import core as sc


def to_dict(scipp_obj):
    """
    Convert a scipp object (Variable, DataArray or Dataset) to a Python dict.
    """
    if su.is_variable(scipp_obj):
        return _variable_to_dict(scipp_obj)
    elif su.is_data_array(scipp_obj):
        return _data_array_to_dict(scipp_obj)
    elif su.is_dataset(scipp_obj):
        # TODO: This currently duplicates all coordinates that would otherwise
        # be at the Dataset level onto the individual DataArrays. We are also
        # manually duplicating all attributes, since these are not carried when
        # accessing items of a Dataset.
        out = {}
        copy_attrs = len(scipp_obj.attrs.keys()) > 0
        for name, item in scipp_obj.items():
            out[name] = _data_array_to_dict(item)
            if copy_attrs:
                for key, attr in scipp_obj.attrs.items():
                    out[name]["attrs"][key] = _variable_to_dict(attr)
        return out


def _variable_to_dict(v):
    """
    Convert a scipp Variable to a Python dict.
    """
    return {
        "dims": _dims_to_strings(v.dims),
        "shape": v.shape,
        "values": v.values,
        "variances": v.variances,
        "unit": v.unit,
        "dtype": v.dtype
    }


def _data_array_to_dict(da):
    """
    Convert a scipp DataArray to a Python dict.
    """
    out = {"coords": {}, "masks": {}, "attrs": {}}
    for key in out.keys():
        for name, item in getattr(da, key).items():
            out[key][str(name)] = _variable_to_dict(item)
    out["data"] = _variable_to_dict(da.data)
    out["values"] = da.values
    out["variances"] = da.variances
    out["dims"] = _dims_to_strings(da.dims)
    out["shape"] = da.shape
    out["name"] = da.name
    out["unit"] = da.unit
    out["dtype"] = da.dtype
    return out


def _dims_to_strings(dims):
    """
    Convert dims that may or may not be strings to strings.
    """
    return [str(dim) for dim in dims]


def from_dict(dict_obj):
    """
    Convert a Python dict to a scipp Variable, DataArray or Dataset.
    """
    if {"coords", "data"}.issubset(set(dict_obj.keys())):
        # Case of a DataArray-like dict (most-likely)
        return _dict_to_data_array(dict_obj)
    elif {"dims", "values"}.issubset(set(dict_obj.keys())):
        # Case of a Variable-like dict (most-likely)
        return _dict_to_variable(dict_obj)
    else:
        # Case of a Dataset-like dict
        out = sc.Dataset()
        for key, item in dict_obj.items():
            out[key] = _dict_to_data_array(item)
        return out


def _dict_to_variable(d):
    """
    Convert a Python dict to a scipp Variable.
    """
    out = {}
    for key, item in d.items():
        if key != "shape":
            out[key] = item
    return sc.Variable(**out)


def _dict_to_data_array(d):
    """
    Convert a Python dict to a scipp DataArray.
    """
    out = {"coords": {}, "masks": {}, "attrs": {}}
    for key in out.keys():
        if key in d:
            for name, item in d[key].items():
                out[key][name] = _dict_to_variable(item)

    return sc.DataArray(data=_dict_to_variable(d["data"]),
                        coords=out["coords"],
                        masks=out["masks"],
                        attrs=out["attrs"])
