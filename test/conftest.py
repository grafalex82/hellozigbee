import pytest
import json

def pytest_addoption(parser):
    parser.addini('port', 'COM port where the Zigbee Device is connected to')
    parser.addini('mqtt_server',    'IP or network name of the MQTT server')
    parser.addini('mqtt_port',      'MQTT server port')
    parser.addini('mqtt_topic',     'Base MQTT topic of the tested device')


def set_device_attribute(device, zigbee, attribute, state, expected_response):
    # Make json payload like {"state_button_3", "ON"}
    payload = '{{"{}":"{}"}}'.format(attribute, state)

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Publish the 'set state' command
    zigbee.publish('set', payload)

    # Verify that the device has received the state change command
    device.wait_str(expected_response)

    # Wait the reply from zigbee2mqtt with the device state
    state = json.loads(zigbee.wait_msg())
    return state[attribute]


def get_device_attribute(device, zigbee, attribute, expected_response):
    # Make json payload like {"state_button_3", ""}
    payload = f'{{"{attribute}":""}}'

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Request the device state
    zigbee.publish('get', payload)                      

    # Verify the device received read attribute command
    device.wait_str(expected_response)

    # Wait the reply from zigbee2mqtt with the device state
    state = json.loads(zigbee.wait_msg())
    return state[attribute]


def wait_attribute_report(zigbee, attribute):
    state = json.loads(zigbee.wait_msg())
    return state[attribute]
