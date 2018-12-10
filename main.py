#!/usr/bin/env python3

import sqlite3
import json
import datetime

from flask import Flask
from flask import request
from flask import g
from flask import jsonify

DATABASE='db/iot.db'

app = Flask(__name__)

def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
    return db

@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()

@app.route('/')
def hello():
    return "Hello IOT"

@app.route('/create_table', methods=['POST'])
def create_table():
    cur = get_db().cursor()
    cur.execute('''DROP TABLE IF EXISTS iot_data''')
    get_db().commit()
    cur.execute('''CREATE TABLE IF NOT EXISTS iot_data (
                                            sensor_type text,
                                            humidity real,
                                            temperature real,
                                            touched boolean,
                                            lumens real,
                                            created_at datetime)''')

    return "Table Created"

@app.route('/iot', methods=['POST'])
def iot():
    data = request.get_json()
    cur = get_db().cursor()
    sensor = data["sensorType"]

    timestamp = datetime.datetime.now()

    if sensor == 'HumTemp':
        hum = data['humidity']
        temp = data['temperature']
        cur.execute('''INSERT INTO iot_data (sensor_type, humidity, temperature, created_at)
                            VALUES (?, ?, ?, ?)''', (sensor, hum, temp, timestamp))
        get_db().commit()
    elif sensor == 'Touch':
        touched = data['touched']
        cur.execute('''INSERT INTO iot_data (sensor_type, touched, created_at) VALUES (?, ?, ?)''', (sensor, touched,
            timestamp))
        get_db().commit()
    elif sensor == 'Luminosity':
        lumens = data['lumens']
        cur.execute('''INSERT INTO iot_data (sensor_type, lumens, created_at) VALUES (?, ?, ?)''', (sensor, lumens,
            timestamp))
        get_db().commit()

    return '''data is {}'''.format(data)

@app.route('/get_data')
def get_data():
    data = query_db("SELECT * FROM iot_data")
    #for item in query_db("SELECT * FROM iot_data"):
    #    data.append(jsonify(sensor=item["sensor_type"],
    #                        temperature=item["temperature"],
    #                        humidity=item["humidity"],
    #                        touched=item["touched"],
    #                        lumens=item["lumens"]))

    print(data)
    return jsonify(data)

def query_db(query, args=(), one=False):
    cur = get_db().cursor().execute(query, args)
    rv = cur.fetchall()
    cur.close()
    return (rv[0] if rv else None) if one else rv

if __name__ == '__main__':
    app.run()


