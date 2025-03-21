from nmflows.frontend.api import api
from nmflows.utils.mac_directory import MACDirectory
from nmflows.backend.rrd import RRDBackend
from flask import Flask, request, make_response, render_template, jsonify # noqa
from config import CONFIG
import numpy as np  # noqa
import pygal # noqa
from pygal.style import LightGreenStyle # noqa
from datetime import datetime, timedelta


#
# API: GET /data/directory
#
@api.route('/data/directory', methods=['GET'])
def get_directory():
    directory = MACDirectory(CONFIG['ixf_url'])
    entries = []
    for entry in directory:
        entries.append({
            'name': entry.name,
            'asnum': entry.asnum,
            'mac': entry.mac,
            'ipv4_addr': entry.ipv4,
            'ipv6_addr': entry.ipv6
        })
    response = make_response(jsonify(entries), 200)
    return response


#
# API: GET /data/long-tail
#
@api.route('/data/long-tail', methods=['GET'])
def get_long_tail_data():

    schedule = request.args.get('period')
    source = request.args.get('source')

    try:

        if schedule is None:
            schedule = 'daily'

        else:
            assert schedule in ['daily', 'weekly', 'monthly', 'yearly'], "unknown period/schedule"

        if source is None:
            source = 'all'
        else:
            assert source in ['all', 'amazon', 'akamai', 'mainstreaming', 'fastly']

        aggregate_data_v4 = np.load(f"/tmp/longtail-to-{source}_{schedule}_v4.npy")
        aggregate_data_v6 = np.load(f"/tmp/longtail-to-{source}_{schedule}_v6.npy")

        # prepare date list for x-axis
        base = datetime.now()

        date_list = []

        match schedule:
            case 'yearly':
                date_list.extend([base - timedelta(days=x) for x in range(aggregate_data_v4.size)])
            case 'monthly':
                date_list.extend([base - timedelta(hours=(2 * x)) for x in range(aggregate_data_v4.size)])
            case 'weekly':
                date_list.extend([base - timedelta(minutes=(30 * x)) for x in range(aggregate_data_v4.size)])
            case 'daily':
                date_list.extend([base - timedelta(minutes=(5 * x)) for x in range(aggregate_data_v4.size)])

        date_list.reverse()

        samples = []

        for idx, tstamp in enumerate(date_list):
            samples.append({
                'ts': tstamp,
                'ipv4': aggregate_data_v4[idx],
                'ipv6': aggregate_data_v6[idx]
            })

        return jsonify({
            'datasets': [{
                'data': samples,
            }]
        })

    except Exception as e:
        return make_response(jsonify({"error": str(e) }), 400)


#
# API: GET /data/peak-prediction
#
@api.route('/data/peak-prediction', methods=['GET'])
def get_peak_prediction_data():

    try:
        period = request.args.get('period')
        if period is None:
            period = 'daily'

        assert period in ['daily', 'weekly', 'monthly', 'yearly'], "unknown period"
        assert 'sources' in request.args.keys(), "undefined sources"
        assert 'destinations' in request.args.keys(), "undefined destinations"

        sources = request.args.get('sources').split(',')
        destinations = request.args.get('destinations').split(',')

        backend = RRDBackend('/data/rrd', 2)
        aggregate_data_v4, aggregate_data_v6 = backend.extract_aggregate_flow(period, sources, destinations)

        # prepare date list for x-axis
        base = datetime.now()

        date_list = []

        match period:
            case 'yearly':
                date_list.extend([base - timedelta(days=x) for x in range(aggregate_data_v4.size)])
            case 'monthly':
                date_list.extend([base - timedelta(hours=(2 * x)) for x in range(aggregate_data_v4.size)])
            case 'weekly':
                date_list.extend([base - timedelta(minutes=(30 * x)) for x in range(aggregate_data_v4.size)])
            case 'daily':
                date_list.extend([base - timedelta(minutes=(5 * x)) for x in range(aggregate_data_v4.size)])

        date_list.reverse()

        samples = []

        for idx, tstamp in enumerate(date_list):
            samples.append({
                'ts': tstamp,
                'ipv4': aggregate_data_v4[idx],
                'ipv6': aggregate_data_v6[idx]
            })

        return jsonify({
            'datasets': [{
                'data': samples,
            }]
        })


    except Exception as e:
        return make_response(jsonify({"error": str(e) }), 400)
