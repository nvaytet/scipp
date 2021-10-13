from scipp.utils import name_with_unit
import numpy as np


def check_axis_range(axrange, var):
    values = var.values
    delta = 0.05 * (values.max() - values.min())
    assert np.allclose(axrange, [values.min() - delta, values.max() + delta])


# def check_axis_range(axrange, coordinate):
#     values = coordinate.values
#     delta = 0.05 * (values.max() - values.min())
#     assert np.allclose(axrange, [values.min() - delta, values.max() + delta])

# def check_axis(plot, coordinate, x_or_y):
#     assert getattr(plot.ax, 'get_{}label'.format(x_or_y))() == coordinate.dims[0]
#     values = coordinate.values
#     delta = 0.05 * (values.max() - values.min())
#     assert np.allclose(
#         getattr(plot.ax, 'get_{}lim'.format(x_or_y))(),
#         [values.min() - delta, values.max() + delta])

# def check_x_axis(*args, **kwargs):
#     check_axis(*args, x_or_y='x', **kwargs)

# def check_data_1d(plot, data):

#     if not isinstance(data, dict):
#         data = {'': data}

#     key = list(data.keys())[0]
#     print(plot.ax.get_ylabel(), name_with_unit(var=data[key]))
#     assert plot.ax.get_ylabel() == name_with_unit(var=data[key])

#     lines = plot.ax.lines
#     vmin = np.Inf
#     vmax = np.Ninf
#     for i, var in enumerate(data.values()):
#         assert np.allclose(lines[i].get_ydata(), var.values)
#         vmin = min(vmin, var.values.min())
#         vmax = min(vmax, var.values.max())

#     # Check axis limits
#     delta = 0.05 * (vmax - vmin)
#     assert np.allclose(plot.ax, get_ylim(), [vmin - delta, vmax + delta])
