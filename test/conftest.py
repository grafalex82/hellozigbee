import pytest
import json

def pytest_addoption(parser):
    parser.addini('port',               'COM port where the Zigbee Device is connected to')
    parser.addini('mqtt_server',        'IP or network name of the MQTT server')
    parser.addini('mqtt_port',          'MQTT server port')
    parser.addini('device_mqtt_topic',  'Base MQTT topic of the tested device')
    parser.addini('bridge_mqtt_topic',  'Base MQTT topic for the zigbee2mqtt instance')


def set_device_attribute(device, zigbee, attribute, state, expected_response):
    # Make json payload like {"state_button_3", "ON"}
    payload = {attribute: state}

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Publish the 'set state' command
    zigbee.publish('set', payload)

    # Verify that the device has received the state change command
    device.wait_str(expected_response)

    # Wait the reply from zigbee2mqtt with the device state
    return zigbee.wait_msg()[attribute]


def get_device_attribute(device, zigbee, attribute, expected_response):
    # Make json payload like {"state_button_3", ""}
    payload = {attribute: ""}

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Request the device state
    zigbee.publish('get', payload)                      

    # Verify the device received read attribute command
    device.wait_str(expected_response)

    # Wait the reply from zigbee2mqtt with the device state
    return zigbee.wait_msg()[attribute]


def send_bind_request(zigbee, clusters, src, dst):
    # clusters attribute must be a list
    if isinstance(clusters, str):
        clusters = [clusters]

    # Send the bind request
    payload = {"clusters": clusters, "from": src, "to": dst}
    zigbee.publish('request/device/bind', payload, bridge=True)


def send_unbind_request(zigbee, clusters, src, dst):
    # clusters attribute must be a list
    if isinstance(clusters, str):
        clusters = [clusters]

    # Send the bind request
    payload = {"clusters": clusters, "from": src, "to": dst}
    zigbee.publish('request/device/unbind', payload, bridge=True)

