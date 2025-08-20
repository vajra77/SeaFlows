from datetime import datetime, timedelta
import os
import rrdtool # noqa
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

    def get_flow_data(self, schedule, src, dst):

        f_path = self._base_path + f"/flows/{src}"

        f_rrdfile = f"{f_path}/flow_{src}_to_{dst}.rrd"

        if os.path.isfile(f_rrdfile):

            match schedule:
                case 'daily':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    f_max_data = f_avg_data

                case 'weekly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')

                case 'monthly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')

                case 'yearly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')

                case _:
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    f_max_data = f_avg_data

            f_clean_avg_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), f_avg_data[2]))
            f_clean_avg_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), f_avg_data[2]))

            f_clean_max_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), f_max_data[2]))
            f_clean_max_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), f_max_data[2]))

            # prepare date list for x-axis
            base = datetime.now()
            date_list = []

            match schedule:
                case 'yearly':
                    date_list.extend([base - timedelta(days=x) for x in range(len(f_clean_avg_data_v4))])
                case 'monthly':
                    date_list.extend([base - timedelta(hours=(2 * x)) for x in range(len(f_clean_avg_data_v4))])
                case 'weekly':
                    date_list.extend([base - timedelta(minutes=(30 * x)) for x in range(len(f_clean_avg_data_v4))])
                case 'daily':
                    date_list.extend([base - timedelta(minutes=(5 * x)) for x in range(len(f_clean_avg_data_v4))])

            date_list.reverse()

            result = {
                'time': date_list,
                'avg_v4': f_clean_avg_data_v4,
                'avg_v6': f_clean_avg_data_v6,
                'max_v4': f_clean_max_data_v4,
                'max_v6': f_clean_max_data_v6,
            }

            return True, result

        else:
            return False, { 'error': 'source or destination not found.' }


    def get_peer_data(self, schedule, proto, macs):
        pass

