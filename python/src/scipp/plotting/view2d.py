# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

from .view import PlotView
from ..utils import make_random_color
from .. import dtype, zeros
import numpy as np
from matplotlib.collections import PathCollection


class PlotView2d(PlotView):
    """
    View object for 2 dimensional plots. Contains a `PlotFigure2d`.

    The difference between `PlotView2d` and `PlotFigure2d` is that
    `PlotView2d` also handles the communications with the `PlotController` that
    are to do with the `PlotProfile` plot displayed below the `PlotFigure2d`.

    In addition, `PlotView2d` provides a dynamic image resampling for large
    input data.
    """
    def __init__(self, figure):
        super().__init__(figure=figure)
        self._axes = ['x', 'y']

        self.xlim_updated = False
        self.ylim_updated = False
        self.current_lims = {"x": np.zeros(2), "y": np.zeros(2)}
        self.global_lims = {"x": np.zeros(2), "y": np.zeros(2)}

        # Connect changes in axes limits to resampling function
        self.figure.ax.callbacks.connect('xlim_changed',
                                         self.check_for_xlim_update)
        self.figure.ax.callbacks.connect('ylim_changed',
                                         self.check_for_ylim_update)

    def _make_data(self, new_values, mask_info):
        values = new_values.values
        # TODO
        # transpose = self.displayed_dims['x'] == new_values.dims[0]
        transpose = False
        if transpose:
            values = np.transpose(values)
        slice_values = {
            "values": values,
            "extent": np.array(list(self.current_lims.values())).flatten()
        }
        # TODO duplication with view1d.py _make_masks
        mask_info = next(iter(mask_info.values()))
        if len(mask_info) > 0:
            # Use automatic broadcasting in Scipp variables
            msk = zeros(sizes=new_values.sizes, dtype='int32')
            for m, val in mask_info.items():
                if val:
                    msk += new_values.masks[m].astype(dtype.int32)
            if transpose:
                slice_values["masks"] = np.transpose(msk.values)
            else:
                slice_values["masks"] = msk.values
        return slice_values

    def check_for_xlim_update(self, event_ax):
        """
        When we use the zoom tool, the event listener on the displayed axes
        limits detects two separate events: one for the x axis and another for
        the y axis. We use a small locking mechanism here to trigger only a
        single resampling update by waiting for the y limits to also change.
        """
        self.xlim_updated = True
        if self.ylim_updated:
            self.update_bins_from_axes_limits()

    def check_for_ylim_update(self, event_ax):
        """
        When we use the zoom tool, the event listener on the displayed axes
        limits detects two separate events: one for the x axis and another for
        the y axis. We use a small locking mechanism here to trigger only a
        single resampling update by waiting for the x limits to also change.
        """
        self.ylim_updated = True
        if self.xlim_updated:
            self.update_bins_from_axes_limits()

    def update_bins_from_axes_limits(self):
        """
        Update the axis limits and resample the image according to new
        viewport.
        """
        self.xlim_updated = False
        self.ylim_updated = False

        # Make sure we don't overrun the original array bounds
        xylims = {
            "x": np.clip(self.figure.ax.get_xlim(),
                         *sorted(self.global_lims["x"])),
            "y": np.clip(self.figure.ax.get_ylim(),
                         *sorted(self.global_lims["y"]))
        }

        dx = np.abs(self.current_lims["x"][1] - self.current_lims["x"][0])
        dy = np.abs(self.current_lims["y"][1] - self.current_lims["y"][0])
        diffx = np.abs(self.current_lims["x"] - xylims["x"]) / dx
        diffy = np.abs(self.current_lims["y"] - xylims["y"]) / dy
        diff = diffx.sum() + diffy.sum()

        # Only resample image if the changes in axes limits are large enough to
        # avoid too many updates while panning.
        if diff > 0.1:
            self.current_lims = xylims
            self.interface["update_viewport"](xylims)

        # If we are zooming, rescale to data?
        if self.figure.rescale_on_zoom():
            self.interface["rescale_to_data"]()

    def update_axes(self, axparams):
        """
        Update the current and global axes limits, before updating the figure
        axes.
        """
        self.current_lims['x'] = axparams["x"]["lims"]
        self.current_lims['y'] = axparams["y"]["lims"]
        self.global_lims["x"] = axparams["x"]["lims"]
        self.global_lims["y"] = axparams["y"]["lims"]
        self.figure.update_axes(axparams)
        self.reset_profile()

    def reset_profile(self):
        """
        Reset all scatter markers when a profile is reset.
        """
        if self.profile_scatter is not None:
            self.profile_scatter = None
            self.figure.ax.collections = []
            self.figure.draw()

    def update_profile(self, event):
        """
        If mouse is hovering inside the axes, show and update profile.
        Otherwise, hide profile.

        TODO: optimize visibility update to that it only calls the function on
        a state change and not on every mouse movement.
        """
        if event.inaxes == self.figure.ax:
            # TODO if we handle tranposition by transposing plot input
            # then we can just assume data dims are [y,x]
            # we do this here, even though the mechanism was not
            # refactored yet
            dimx = self._data.dims[-1]
            dimy = self._data.dims[-2]
            x = self._data.meta[dimx]
            y = self._data.meta[dimy]
            xdata = event.xdata - self.current_lims["x"][0]
            ydata = event.ydata - self.current_lims["y"][0]
            # Note that xdata and ydata already have the left edge subtracted
            ix = int(xdata / (x.values[1] - x.values[0]))
            iy = int(ydata / (y.values[1] - y.values[0]))
            slices = {
                dimx: (x[dimx, ix], x[dimx, ix + 1]),
                dimy: (y[dimy, iy], y[dimy, iy + 1])
            }
            self.interface["update_profile"](slices)
            self.interface["toggle_hover_visibility"](True)
        else:
            self.interface["toggle_hover_visibility"](False)

    def keep_or_remove_profile(self, event):
        """
        Forward the keep or remove event to the correct function.
        """
        if isinstance(event.artist, PathCollection):
            self.remove_profile(event)
            # We need a profile lock to catch the second time the function is
            # called because the pick event is registed by both the scatter
            # points and the image
            self.profile_update_lock = True
        elif self.profile_update_lock:
            self.profile_update_lock = False
        else:
            self.keep_profile(event)
        self.figure.draw()

    def keep_profile(self, event):
        """
        Add a colored scatter point to mark the location of the saved profile.
        """
        xdata = event.mouseevent.xdata
        ydata = event.mouseevent.ydata
        col = make_random_color(fmt='rgba')
        self.profile_counter += 1
        line_id = self.profile_counter
        self.profile_ids.append(line_id)
        if self.profile_scatter is None:
            self.profile_scatter = self.figure.ax.scatter([xdata], [ydata],
                                                          c=[col],
                                                          edgecolors="w",
                                                          picker=5,
                                                          zorder=10)
        else:
            new_offsets = np.concatenate(
                (self.profile_scatter.get_offsets(), [[xdata, ydata]]), axis=0)
            new_colors = np.concatenate(
                (self.profile_scatter.get_facecolors(), [col]), axis=0)
            self.profile_scatter.set_offsets(new_offsets)
            self.profile_scatter.set_facecolors(new_colors)

        self.interface["keep_line"](target="profile",
                                    color=col,
                                    line_id=line_id)

    def remove_profile(self, event):
        """
        Remove a scatter point corresponding to a saved profile.
        """
        ind = event.ind[0]
        xy = np.delete(self.profile_scatter.get_offsets(), ind, axis=0)
        c = np.delete(self.profile_scatter.get_facecolors(), ind, axis=0)
        self.profile_scatter.set_offsets(xy)
        self.profile_scatter.set_facecolors(c)
        # Also remove the line from the 1d plot
        self.interface["remove_line"](target="profile",
                                      line_id=self.profile_ids[ind])
        self.profile_ids.pop(ind)
