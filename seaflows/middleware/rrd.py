from datetime import datetime, timedelta
import os
import rrdtool # noqa


def _octets2bits(x, gamma):
    if isinstance(x, (int, float)) and x >= 0:
        return x * 8 * gamma / 1000000000
    else:
        return 0.2


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

    def get_flow_data(self, schedule, proto, src, dst):

        f_path = self._base_path + f"/flows/{src}"
        r_path = self._base_path + f"/flows/{dst}"

        f_rrdfile = f"{f_path}/flow_{src}_to_{dst}.rrd"
        r_rrdfile = f"{r_path}/flow_{dst}_to_{src}.rrd"

        if os.path.isfile(f_rrdfile) and os.path.isfile(r_rrdfile):

            match schedule:
                case 'daily':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    r_avg_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    f_max_data = f_avg_data
                    r_max_data = r_avg_data

                case 'weekly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
                    r_avg_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')
                    r_max_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '1800', '-s', "end-1w", '-e', 'midnight today')

                case 'monthly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
                    r_avg_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')
                    r_max_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '7200', '-s', "end-1m", '-e', 'midnight today')

                case 'yearly':
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
                    r_avg_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
                    f_max_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')
                    r_max_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '86400', '-s', "end-1y", '-e', 'midnight today')

                case _:
                    f_avg_data = rrdtool.fetch(f_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    r_avg_data = rrdtool.fetch(r_rrdfile, "AVERAGE", '-r', '300', '-s', "now-1d", '-e', 'now')
                    f_max_data = f_avg_data
                    r_max_data = r_avg_data

            f_clean_avg_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), f_avg_data[2]))
            f_clean_avg_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), f_avg_data[2]))
            r_clean_avg_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), r_avg_data[2]))
            r_clean_avg_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), r_avg_data[2]))

            f_clean_max_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), f_max_data[2]))
            f_clean_max_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), f_max_data[2]))
            r_clean_max_data_v4 = list(map(lambda x: _octets2bits(x[0], self._base_gamma), r_max_data[2]))
            r_clean_max_data_v6 = list(map(lambda x: _octets2bits(x[1], self._base_gamma), r_max_data[2]))

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

            if proto == 4:
                result = {
                    'time': date_list,
                    'avg_in': f_clean_avg_data_v4,
                    'avg_out': r_clean_avg_data_v4,
                    'max_in': f_clean_max_data_v4,
                    'max_out': r_clean_max_data_v4,
                }

            else:
                result = {
                    'time': date_list,
                    'avg_in': f_clean_avg_data_v6,
                    'avg_out': r_clean_avg_data_v6,
                    'max_in': f_clean_max_data_v6,
                    'max_out': r_clean_max_data_v6,
                }

            return True, result

        else:
            return False, { 'error': 'source or destination not found.' }
