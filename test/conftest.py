import serial
import pytest

from smartswitch import *
from zigbee import *
from device import *

def pytest_addoption(parser):
    parser.addini('port',               'COM port where the Zigbee Device is connected to')
    parser.addini('mqtt_server',        'IP or network name of the MQTT server')
    parser.addini('mqtt_port',          'MQTT server port')
    parser.addini('device_mqtt_topic',  'Base MQTT topic of the tested device')
    parser.addini('bridge_mqtt_topic',  'Base MQTT topic for the zigbee2mqtt instance')


@pytest.fixture(scope="session")
def port(pytestconfig):
    ser = serial.Serial(pytestconfig.getini("port"), baudrate=115200, timeout=1)
    ser.dtr = False # Release reset signal so that the device can boot
    yield ser


@pytest.fixture(scope="session")
def device(port):
    dev = ZigbeeDevice(port)
    yield dev


@pytest.fixture(scope="session")
def zigbee(pytestconfig):
    net = ZigbeeNetwork(pytestconfig)
    yield net
    net.disconnect()


# List of smart switch channels (endpoint number and z2m name)
button_channels = [(2, "button_1"), (3, "button_2")]

# Make each test that uses switch fixture to run twice for both buttons. 
# Using the ids parameter the button name will be displayed as a test parameter
@pytest.fixture(scope = 'function', params = button_channels, ids=lambda x: x[1])
def switch(device, zigbee, request):
    return SmartSwitch(device, zigbee, request.param[0], request.param[1])

# Make sure that no bindings that could possibly change test behavior is active. 
# Cleanup bindings at exit. Use autouse=True to implicitly apply it to all tests
@pytest.fixture(scope="session", autouse = True)
def cleanup_bindings(zigbee):
    for ep, _ in button_channels:
        send_unbind_request(zigbee, "genLevelCtrl", f"my_test_switch/{ep}", "Coordinator")

    yield

    for ep, _ in button_channels:
        send_unbind_request(zigbee, "genLevelCtrl", f"my_test_switch/{ep}", "Coordinator")
