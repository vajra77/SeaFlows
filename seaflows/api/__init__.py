from flask import Blueprint # noqa


api = Blueprint('api', __name__)
from .data import data