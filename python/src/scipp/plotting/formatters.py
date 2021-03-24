from .._utils import value_to_string
from .._scipp.core import to_unit, Unit
import numpy as np


class LabelFormatter:
    """
    Format ticks by replacing coord values with label values.
    """
    def __init__(self, label_values, coord_values):
        self.label_values = label_values
        self.coord_values = coord_values

    def formatter(self, val, pos):
        return value_to_string(self.label_values[np.abs(self.coord_values -
                                                        val).argmin()])


class VectorFormatter:
    """
    Format vector output for ticks: return 3 components as a string.
    """
    def __init__(self, array_values, size):
        self.array_values = array_values
        self.size = size

    def format(self, val, pos):
        return "(" + ",".join([
            value_to_string(item, precision=2)
            for item in self.array_values[int(val)]
        ]) + ")" if (int(val) >= 0 and int(val) < self.size) else ""


class StringFormatter:
    """
    Format string ticks: find closest string in coordinate array.
    """
    def __init__(self, array_values, size):
        self.array_values = array_values
        self.size = size

    def formatter(self, val, pos):
        return self.array_values[int(val)] if (
            int(val) >= 0 and int(val) < self.size) else ""


class DateFormatter:
    """
    Format datetime ticks: adjust the time precision and update
    offset according to the currently displayed range.
    """
    def __init__(self, offset, dim, interface):
        self.offset = offset
        self.dim = dim
        self.interface = interface
        self.indicators = []

    def formatter(self, val, pos):
        d = (self.offset + (int(val) * self.offset.unit)).value
        dt = str(d)
        trim = 0
        bounds = self.interface["get_view_axis_bounds"](self.dim)
        diff = (bounds[1] - bounds[0]) * self.offset.unit
        label = self.dim
        if pos == 0:
            self.indicators.clear()

        date_min = str((int(bounds[0]) * self.offset.unit + self.offset).value)
        date_max = str((int(bounds[1]) * self.offset.unit + self.offset).value)

        check_transition = True
        check_time = True
        check_ms = False

        if (diff < to_unit(4 * Unit('us'), diff.unit)).value:
            # label: 2017-01-13T12:15:45.123, tick: 456.789 us
            label += r" [$\mu$s]"
            string = str(float("{}.{}".format(dt[23:26], dt[26:])))
            check_ms = True
        elif (diff < to_unit(4 * Unit('ms'), diff.unit)).value:
            # label: 2017-01-13T12:15:45, tick: 123.456 ms
            label += " [ms]"
            string = str(float("{}.{}".format(dt[20:23], dt[23:26])))
        elif (diff < to_unit(4 * Unit('s'), diff.unit)).value:
            # label: 2017-01-13T12:15:45, tick: 123 ms
            label += " [ms]"
            string = str(float(dt[20:23]))
        elif (diff < to_unit(4 * Unit('min'), diff.unit)).value:
            # label: 2017-01-13, tick: 12:15:45.1
            string = dt[11:21]
            check_time = False
        elif (diff < to_unit(4 * Unit('h'), diff.unit)).value:
            # label: 2017-01-13, tick: 12:15:45
            string = dt[11:19]
            check_time = False
        elif (diff < to_unit(4 * Unit('d'), diff.unit)).value:
            # label: 2017-01-13, tick: 12:15
            string = dt[11:16]
            check_time = False
        elif (diff < to_unit(6 * Unit('month'), diff.unit)).value:
            # label: 2017-01-13, tick: 12h
            string = dt[11:13] + 'h'
            check_time = False
        elif (diff < to_unit(4 * Unit('year'), diff.unit)).value:
            # label: 2017, tick: 01-13
            string = dt[:10]
            check_transition = False
        elif (diff < to_unit(100 * Unit('year'), diff.unit)).value:
            # tick: 2017-01
            string = dt[:7]
            check_transition = False
        else:
            # tick: 2017
            string = dt[:4]
            check_transition = False

        if check_transition:
            string, trim = self.check_for_transition(pos, string, date_min,
                                                     date_max, dt, check_time,
                                                     check_ms)

        if pos == 1:
            if trim > 0:
                label += f" ({dt[:trim]})"
            self.interface["set_view_axis_label"](self.dim, label)
        return string

    def check_for_transition(self, pos, string, date_min, date_max, dt,
                             check_time, check_ms):
        """
        Function that checks for transitions between years, months, days etc..
        adds an additional row of information below the tick labels if
        required, to indicate the year on each side of the transition.
        """
        different_date = False
        different_time = False

        trim = 0
        date_start = 0
        time_start = 11
        date_end = 10
        time_end = 0

        different_date = date_min[:date_end] != date_max[:date_end]

        if check_time:
            if check_ms:
                time_end = 23
            else:
                time_end = 19
            different_time = date_min[time_start:time_end] != date_max[
                time_start:time_end]

        if (not different_date) and (not different_time):
            trim = max(date_end, time_end)
        else:
            if different_date and different_time:
                start = date_start
                end = time_end
            elif different_date:
                start = date_start
                end = date_end
            elif different_time:
                start = time_start
                end = time_end
                trim = time_start - 1
            if pos != 0 and (dt[start:end] not in self.indicators):
                string += '\n' + dt[start:end]
                self.indicators.append(dt[start:end])

        return string, trim
