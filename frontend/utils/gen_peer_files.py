import glob
import os
import getopt
import sys
import json
import numpy as np
from frontend.seaflows.middleware import RRDBackend


RRD_DIR = '/data/rrd'
SRC_DIR = f"{RRD_DIR}/flows"
TGT_DIR = '/data/json/peers'


def usage():
    print("Command arguments:")
    print(" -h, --help : print this message")
    print(" -p, --proto : IP protocol [4|6]")
    print(" -s, --schedule : schedule [d|w|m|y]")

def get_in_data(schedule, proto, src):

    rrdb = RRDBackend(RRD_DIR)

    avg_in = None
    max_in = None
    ts = None

    rrd_files = os.listdir(SRC_DIR + '/' + src)
    for rrd_f in rrd_files:
        path = f"{SRC_DIR}/{src}/{rrd_f}"
        if os.path.isfile(path):
            tmp_avg, tmp_max = rrdb.get_flow_data(schedule, proto, path)

            if avg_in is None:
                avg_in = np.array(tmp_avg)
            else:
                avg_in = np.add(avg_in, tmp_avg)

            if max_in is None:
                max_in = np.array(tmp_max)
            else:
                max_in = np.add(max_in, tmp_max)

    if avg_in is not None and max_in is not None:
        ts = rrdb.get_timestamps(schedule, avg_in)
        return list(avg_in), list(max_in), ts
    else:
        return [], [], []


def get_out_data(schedule, proto, src):

    rrdb = RRDBackend(RRD_DIR)

    avg_out = None
    max_out = None
    ts = None

    search_term = SRC_DIR + f"*/flow_*_to_{src}.rrd"
    targets = glob.glob(search_term)

    for tgt_f in targets:
        if os.path.isfile(tgt_f):
            tmp_avg, tmp_max = rrdb.get_flow_data(schedule, proto, tgt_f)

            if avg_out is None:
                avg_out = np.array(tmp_avg)
            else:
                avg_out = np.add(avg_out, tmp_avg)

            if max_out is None:
                max_out = np.array(tmp_max)
            else:
                max_out = np.add(max_out, tmp_max)

    if avg_out is not None and max_out is not None:
        ts = rrdb.get_timestamps(schedule, avg_out)
        return list(avg_out), list(max_out), ts
    else:
        return [], [],[]


if __name__ == '__main__':

    my_proto = 4
    my_schedule = 'd'

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hp:s:", ["help", "proto=", "schedule="])

    except getopt.GetoptError as err:
        print(f"error: {err}")
        usage()
        sys.exit(1)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit(0)
        elif opt in ("-p", "--proto"):
            my_proto = arg
        elif opt in ("-s", "--schedule"):
            my_schedule = arg

    sources = [os.path.basename(x[0]) for x in os.walk(SRC_DIR)]
    sources.pop(0)

    for source in sources:
        src_avg_in, src_max_in, dates_in = get_in_data(my_schedule, my_proto, source)
        src_avg_out, src_max_out, dates_out = get_out_data(my_schedule, my_proto, source)

        data = {
            'avg_in': src_avg_in,
            'avg_out': src_avg_out,
            'max_in': src_max_in,
            'max_out': src_max_out,
            'time': dates_in
        }

        tgt_file = TGT_DIR + f"/peer_{source}_v{my_proto}.json"

        json_str = json.dumps(data, indent=4, default=str)
        with open(tgt_file, "w") as f:
            f.write(json_str)

    sys.exit(0)

