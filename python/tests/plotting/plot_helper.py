# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @file
# @author Neil Vaytet
# import matplotlib as mpl
# mpl.use('Agg')
import scipp as sc
# import matplotlib.pyplot as plt


def plot(*args, close=True, **kwargs):
    fig = sc.plot(*args, **kwargs)
    if close:
        fig.close()
    # fig, ax = plt.subplots()
    # # ax.plot([1, 2, 3], [4, 5, 6])
    # # plt.close(fig)
    # return
