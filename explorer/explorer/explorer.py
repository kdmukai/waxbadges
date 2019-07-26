import json
import os

from datetime import datetime
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
    achievement = ecosystem.get('categories')[int(cat_key)].get('achievements')[int(key)]
    achievement['key'] = int(key)
    users = []
    for usergrant in achievement.get('usergrants'):
        user = ecosystem.get('users')[usergrant.get('user_id')]
        if not user.get('avatarurl').startswith('http'):
            user['avatarurl'] = ecosystem.get('baseasseturl') + "/" + user.get('avatarurl')

        user['user_id'] = usergrant.get('user_id')
        print(user)
        users.append({
            "user": user,
            "timestamp": datetime.utcfromtimestamp(usergrant.get('timestamp')),
        })

    return render_template(
        'ach.html',
        ecosystem=ecosystem,
        category_id=cat_key,
        achievement=achievement,
        usergrants=users
    )




@app.route('/poa/<ecosys_key>/<cat_key>/<ach_key>/<user_key>', methods=['GET'])
def get_proofofachievement(ecosys_key, cat_key, ach_key, user_key):
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
    achievement = ecosystem.get('categories')[int(cat_key)].get('achievements')[int(ach_key)]
    achievement['key'] = int(ach_key)
    user = ecosystem.get('users')[int(user_key)]
    user['key'] = int(user_key)
    if not user.get('avatarurl').startswith('http'):
        user['avatarurl'] = ecosystem.get('baseasseturl') + "/" + user.get('avatarurl')

    for userachievement in user.get('userachievements'):
        if userachievement.get('category_id') == int(cat_key) and userachievement.get('achievement_id') == int(ach_key):
            timestamp = datetime.utcfromtimestamp(userachievement.get('timestamp'))
            break

    return render_template(
        'poa.html',
        ecosystem=ecosystem,
        category_id=cat_key,
        achievement=achievement,
        user=user,
        timestamp=timestamp
    )


@app.route('/user/<ecosys_key>/<key>', methods=['GET'])
def get_user(ecosys_key, key):
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
    user = ecosystem.get('users')[int(key)]

    # Get all of this User's granted Achievements
    achievements = []
    grants_categories = user.bycategory
    for (category_id, achievements_list) in grants_categories.iteritems():
        userachievements = achievements_list.userachievements
        for userachievement in userachievements:
            achievements.append((ecosystem.get('categories')[category_id].get('achievements')[userachievement.get('achievement_id')], userachievement.get('timestamp')))

    return render_template(
        'user.html',
        ecosystem=ecosystem,
        achievements=achievements
    )



@app.route('/find/user/<ecosys_key>/<userid>', methods=['GET'])
def find_user(ecosys_key, userid):
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
    for (key, user) in enumerate(ecosystem.users):
        if user.userid == userid:
            return key

    return None


