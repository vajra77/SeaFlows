from seaflows.config import APP_CONFIG
from seaflows.api import api
from seaflows.middleware import RRDBackend
from flask import Flask, request, make_response, render_template, jsonify # noqa


@api.route('/data/test', methods=['GET'])
def test():
    return make_response(jsonify({ 'message': 'ok' })), 200


@api.route('/data/flow', methods=['GET'])
def get_flow():
    src = request.args.get('src')
    dst = request.args.get('dst')
    schedule = request.args.get('period') or 'daily'

    rrd = RRDBackend(APP_CONFIG['rrd_dir'], 1)

    res, data = rrd.render_flow(schedule, src, dst)

    if res:
        return make_response(jsonify(data)), 200
    else:
        return make_response(jsonify(data)), 404


@api.route('/data/peer', methods=['GET'])
def get_peer():
    macs = request.args.get('macs').split(',')
    schedule = request.args.get('period') or 'daily'
    proto = request.args.get('proto') or 4

    rrd = RRDBackend(APP_CONFIG['rrd_dir'], 1)

    res, data = rrd.render_peer(schedule, proto, macs)

    if res:
        return make_response(jsonify(data)), 200
    else:
        return make_response(jsonify(data)), 404

