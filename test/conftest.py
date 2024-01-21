import serial
import pytest

from smartswitch import *
from zigbee import *
from device import *
from bridge import *
from group import *

def pytest_addoption(parser):
    parser.addini('port',               'COM port where the Zigbee Device is connected to')
    parser.addini('mqtt_server',        'IP or network name of the MQTT server')
    parser.addini('mqtt_port',          'MQTT server port')
    parser.addini('device_name',        'Name of the device in zigbee2mqtt')
    parser.addini('base_topic',         'Base MQTT topic of the zigbee2mqtt instance')


@pytest.fixture(scope="session")
def device_name(pytestconfig):
    yield pytestconfig.getini('device_name')


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
    net = ZigbeeNetwork(pytestconfig.getini('mqtt_server'), pytestconfig.getini('mqtt_port'), pytestconfig.getini('base_topic'))
    yield net
    net.disconnect()


@pytest.fixture(scope="session")
def bridge(zigbee, pytestconfig):
    bridge = Bridge(zigbee)
    yield bridge

# A fixture that creates a test_group, and cleans it up after all tests completed
@pytest.fixture(scope="session")
def group(zigbee, bridge):
    grp = Group(zigbee, bridge, 'test_group', 1234)

    # Remove previously created group (if any). Ignore errors if there was no group before.
    resp = grp.delete()

    # Create a brand new group
    resp = grp.create()
    assert resp['status'] == 'ok'

    # Use the group
    yield grp

    # Cleanup our group
    resp = grp.delete()
    assert resp['status'] == 'ok'


# List of smart switch channels (endpoint number and z2m name)
button_channels = [(2, "button_1"), (3, "button_2")]

# Make each test that uses switch fixture to run twice for both buttons. 
# Using the ids parameter the button name will be displayed as a test parameter
@pytest.fixture(scope = 'function', params = button_channels, ids=lambda x: x[1])
def switch(device, zigbee, request, pytestconfig):
    return SmartSwitch(device, zigbee, request.param[0], request.param[1], pytestconfig.getini('device_name'))


# Iterate on all bindings that device currently has, and cleanup all On/Off and LevelCtrl bindings to the Coordinator
# These bindings change the device behavior, and must be cleared on start to avoid undesired behavior
def clear_coordinator_bindings(bridge, device_name):
    coordinator_addr = bridge.get_coordinator_address()

    for binding in bridge.get_device_bindings(device_name):
        if binding['cluster'] == "genOnOff" or binding['cluster'] == "genLevelCtrl":
            if binding['target_addr'] == coordinator_addr:
                send_unbind_request(bridge, binding['cluster'], f"{device_name}/{binding['endpoint']}", 'Coordinator')


# Make sure that no bindings that could possibly change test behavior is active. 
# Cleanup bindings at exit. Use autouse=True to implicitly apply it to all tests
@pytest.fixture(scope="session", autouse = True)
def cleanup_bindings(bridge, device_name):
    clear_coordinator_bindings(bridge, device_name)

    yield

    clear_coordinator_bindings(bridge, device_name)


# A handy fixture that dumps the test name before test starts, and after it ends
@pytest.fixture(scope="function", autouse=True)
def dump_test_name(bridge, request, pytestconfig):
    # Print test name before test start
    # zigbee2mqtt ignores unknown bridge topics, so the message to test_name topic does no harm
    payload = {"test_name": request.node.name, "phase": "begin"}
    bridge.publish("test_name", payload)
    
    # Do the test
    yield

    # Dump the test name after it is done
    payload = {"test_name": request.node.name, "phase": "end"}
    bridge.publish("test_name", payload)
