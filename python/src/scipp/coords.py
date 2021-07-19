# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Simon Heybrock, Jan-Lukas Wynen

import inspect
from typing import Union
from . import Variable, DataArray, Dataset


def _add_coord(*, name, obj, graph, rename):
    if name in obj.meta:
        return _produce_coord(obj, name)
    if isinstance(graph[name], str):
        out = _get_coord(graph[name], obj, graph, rename)
        if obj.bins is not None:
            # Calls to _get_coord for dense coord handling take care of
            # recursion and add also event coords, here and below we thus
            # simply consume the event coord.
            out_bins = _consume_coord(obj.bins, graph[name])
        dim = (graph[name], )
    else:
        func = graph[name]
        argnames = inspect.getfullargspec(func).kwonlyargs
        args = {arg: _get_coord(arg, obj, graph, rename) for arg in argnames}
        out = func(**args)
        if obj.bins is not None:
            args.update({
                arg: _consume_coord(obj.bins, arg)
                for arg in argnames if arg in obj.bins.coords
            })
            out_bins = func(**args)
        dim = tuple(argnames)
    if isinstance(out, Variable):
        out = {name: out}
    rename.setdefault(dim, []).extend(out.keys())
    for key, coord in out.items():
        obj.coords[key] = coord
    if obj.bins is not None:
        if isinstance(out_bins, Variable):
            out_bins = {name: out_bins}
        for key, coord in out_bins.items():
            # Non-binned coord should be duplicate of dense handling above, if
            # present => ignored.
            if coord.bins is not None:
                obj.bins.coords[key] = coord


def _consume_coord(obj, name):
    if name in obj.coords:
        obj.attrs[name] = obj.coords[name]
        del obj.coords[name]
    return obj.attrs[name]


def _produce_coord(obj, name):
    if name in obj.attrs:
        obj.coords[name] = obj.attrs[name]
        del obj.attrs[name]
    return obj.coords[name]


def _get_coord(name, obj, graph, rename):
    if name in obj.meta:
        return _consume_coord(obj, name)
    else:
        _add_coord(name=name, obj=obj, graph=graph, rename=rename)
        return _get_coord(name, obj, graph, rename=rename)


def _get_splitting_nodes(graph):
    nodes = {}
    for key in graph:
        for start in key:
            nodes[start] = nodes.get(start, 0) + 1
    return [node for node in nodes if nodes[node] > 1]


def transform_coords(obj: Union[DataArray, Dataset], coords,
                     graph: dict) -> Union[DataArray, Dataset]:
    """
    """
    # Keys in graph may be tuple to define multiple outputs
    simple_graph = {}
    for key in graph:
        for k in [key] if isinstance(key, str) else key:
            simple_graph[k] = graph[key]
    obj = obj.copy(deep=False)
    rename = {}
    for name in [coords] if isinstance(coords, str) else coords:
        _add_coord(name=name, obj=obj, graph=simple_graph, rename=rename)
    blacklist = _get_splitting_nodes(rename)
    for key, val in rename.items():
        found = [k for k in key if k in obj.dims]
        # rename if exactly one input is dimension-coord
        if len(val) == 1 and len(found) == 1 and found[0] not in blacklist:
            obj.rename_dims({found[0]: val[0]})
    return obj
