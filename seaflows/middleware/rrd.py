from datetime import datetime, timedelta
import os
import rrdtool # noqa
import glob
import numpy as np


def _octets2bits(x, gamma):
    if isinstance(x, (int, float)) and x >= 0:
        return x * 8 * gamma / 1000000000
    else:
        return 0.0


class RRDBackend:

    def __init__(self, base_path, base_gamma=1):
        self._base_path = base_path
        self._base_gamma = base_gamma

    @property
    def base_path(self):
        return self._base_path

    @property
    def base_gamma(self):
        return self._base_gamma

    def render_flow(self, schedule, src, dst):
        f_path = self._base_path + f"/flows/{src}"
        f_rrdfile = f"{f_path}/flow_{src}_to_{dst}.rrd"

        if os.path.isfile(f_rrdfile):
            avg_v4, max_v4 = self.get_flow_data(schedule, 4, f_rrdfile)
            avg_v6, max_v6 = self.get_flow_data(schedule, 6, f_rrdfile)

            date_list = self.get_timestamps(schedule, avg_v4)

            result = {
                'time': date_list,
                'avg_v4': avg_v4,
                'avg_v6': avg_v6,
                'max_v4': max_v4,
                'max_v6': max_v6,
            }

            return True, result

        else:
            return False, { 'error': 'unable to find RRD file' }


    def render_peer(self, schedule, proto, macs):

        avg_in = None
        max_in = None
        avg_out = None
        max_out = None
        ts = None

        for mac in macs:
            rrd_files = os.listdir(self._base_path + f"/flows/{mac}")
            for rrd_f in rrd_files:
                path = self._base_path + f"/flows/{mac}/{rrd_f}"
                if os.path.isfile(path):
                    tmp_avg, tmp_max = self.get_flow_data(schedule, proto, path)

                    if avg_in is None:
                        avg_in = np.array(tmp_avg)
                    else:
                        avg_in = np.add(avg_in, tmp_avg)

                    if max_in is None:
                        max_in = np.array(tmp_max)
                    else:
                        max_in = np.add(max_in, tmp_max)

            search_term = self._base_path + f"*/flow_*_to_{mac}.rrd"
            targets = glob.glob(search_term)

            for tgt_f in targets:
                if os.path.isfile(tgt_f):
                    tmp_avg, tmp_max = self.get_flow_data(schedule, proto, tgt_f)

                    if avg_out is None:
                        avg_out = np.array(tmp_avg)
                    else:
                        avg_out = np.add(avg_out, tmp_avg)

                    if max_out is None:
                        max_out = np.array(tmp_max)
                    else:
                        max_out = np.add(max_out, tmp_max)
            # end mac loop

        if avg_in is not None and \
            avg_out is not None and \
            max_in is not None and \
            max_out is not None:

            ts = self.get_timestamps(schedule, avg_in)

            data = {
                'avg_in': avg_in,
                'avg_out': avg_out,
                'max_in': max_in,
                'max_out': max_out,
                'time': ts
            }

            return True, data

        else:
            return False, { 'error': 'unable to recover all data' }


    def get_flow_data(self, schedule, proto, filename):

        match schedule:
            case 'weekly' | 'week' | 'w':
                avg_data = rrdtool.fetch(filename, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
                max_data = rrdtool.fetch(filename, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
            case 'monthly' | 'month' | 'm':
                avg_data = rrdtool.fetch(filename, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
                max_data = rrdtool.fetch(filename, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
            case 'yearly' | 'year' | 'y':
                avg_data = rrdtool.fetch(filename, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
                max_data = rrdtool.fetch(filename, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
            case _:
                avg_data = rrdtool.fetch(filename, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                max_data = avg_data

        if proto == 4:
            clean_avg_data = list(map(lambda x: _octets2bits(x[0], self._base_gamma), avg_data[2]))
            clean_max_data = list(map(lambda x: _octets2bits(x[0], self._base_gamma), max_data[2]))
        else:
            clean_avg_data = list(map(lambda x: _octets2bits(x[1], self._base_gamma), avg_data[2]))
            clean_max_data = list(map(lambda x: _octets2bits(x[1], self._base_gamma), max_data[2]))

        return clean_avg_data, clean_max_data


    @staticmethod
    def get_timestamps(schedule, data):

        # prepare date list for x-axis
        base = datetime.now()
        timestamps = []

        match schedule:
            case 'yearly' | 'year' | 'y':
                timestamps.extend([base - timedelta(days=x) for x in range(len(data))])
            case 'monthly' | 'month' | 'm':
                timestamps.extend([base - timedelta(hours=(2 * x)) for x in range(len(data))])
            case 'weekly' | 'week' | 'w':
                timestamps.extend([base - timedelta(minutes=(30 * x)) for x in range(len(data))])
            case _:
                timestamps.extend([base - timedelta(minutes=(5 * x)) for x in range(len(data))])

        timestamps.reverse()

        return timestamps

