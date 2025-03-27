from flask import Blueprint # noqa


seaflows_api = Blueprint('api', __name__)
from .data import data