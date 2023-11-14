import pytest

def pytest_addoption(parser):
    parser.addini('port', 'COM port where the Zigbee Device is connected to')
    parser.addini('mqtt_server',    'IP or network name of the MQTT server')
    parser.addini('mqtt_port',      'MQTT server port')
    parser.addini('mqtt_topic',     'Base MQTT topic of the tested device')
