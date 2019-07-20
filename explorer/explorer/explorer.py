import json
import os

from eospy.cleos import Cleos
from flask import Flask, render_template, url_for, redirect
from flask_cdn import CDN


app = Flask(__name__)
app.config['CDN_DOMAIN'] = 'static.waxbadges.com'
app.config['CDN_TIMESTAMP'] = False
CDN(app)


CONTRACT_ACCOUNT = 'waxbadgesftw'
CONTRACT_SCOPE = CONTRACT_ACCOUNT

ECOSYSTEMS_TABLE = 'ecosystems'
MYWAXBADGES_TABLE = 'mywaxbadges'


"""
    Cleos quick reference

    get_table(self, code, scope, table, index_position='',key_type='', lower_bound='', upper_bound='', limit=10, timeout=30)
"""

@app.route('/favicon.ico')
def favicon():
    return redirect("https://explorer.waxbadges.com/static/image/favicon/favicon.ico", code=302)


@app.route('/', methods=['GET'])
def index():
    cleos = Cleos(url='https://chain.wax.io')
    ecosystems = cleos.get_table(
        code=CONTRACT_ACCOUNT,
        scope=CONTRACT_SCOPE,
        table=ECOSYSTEMS_TABLE,
        limit=100,
        timeout=30
    )

    return render_template(
        'index.html',
        ecosystems=ecosystems.get('rows', [])
    )


@app.route('/ecosys/<key>', methods=['GET'])
def get_ecosystem(key):
    cleos = Cleos(url='https://chain.wax.io')
    ecosystems = cleos.get_table(
        code=CONTRACT_ACCOUNT,
        scope=CONTRACT_SCOPE,
        table=ECOSYSTEMS_TABLE,
        lower_bound=key,
        upper_bound=key,
        limit=1,
        timeout=30
    )

    ecosystem = ecosystems.get('rows', [])[0]
    print(ecosystem)

    return render_template(
        'ecosys.html',
        ecosystem=ecosystem
    )


@app.route('/ach/<ecosys_key>/<cat_key>/<key>', methods=['GET'])
def get_achievement(ecosys_key, cat_key, key):
    cleos = Cleos(url='https://chain.wax.io')
    ecosystems = cleos.get_table(
        code=CONTRACT_ACCOUNT,
        scope=CONTRACT_SCOPE,
        table=ECOSYSTEMS_TABLE,
        lower_bound=ecosys_key,
        upper_bound=ecosys_key,
        limit=1,
        timeout=30
    )

    ecosystem = ecosystems.get('rows', [])[0]
    ach = ecosystem.get('categories')[int(cat_key)].get('achievements')[int(key)]

    return render_template(
        'ach.html',
        ecosystem=ecosystem,
        ach=ach
    )


