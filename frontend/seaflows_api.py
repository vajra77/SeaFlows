from seaflows.api import api as api_blueprint
from flask import Flask


app = Flask(__name__)
app.register_blueprint(api_blueprint, url_prefix='/api/v2/')