from seaflows.api import api    # noqa
from seaflows.middleware import RRDBackend
import numpy as np  # noqa
from flask import Flask, request, make_response, render_template, jsonify # noqa


@api.route('/data/flow', methods=['GET'])
def get_flow():
    src = request.args.get('src')
    dst = request.args.get('dst')
    schedule = request.args.get('period')
    proto = request.args.get('proto')

    rrd = RRDBackend('/data/rrd', 1)

    res, data = rrd.get_flow_data(schedule, proto, src, dst)

    if res:
        return make_response(jsonify(data)), 200
    else:
        return make_response(jsonify(data)), 404



