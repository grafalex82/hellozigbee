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
    # Prepare the group object
    grp = Group(zigbee, bridge, 'test_group', 1234)

    # No need to (re)create a group if it exists already
    for item in bridge.get_groups():
        if item['friendly_name'] == 'test_group':
            assert item['id'] == 1234
            return grp

    # Create a brand new group
    resp = grp.create()
    assert resp['status'] == 'ok'

    return grp


# List of smart switch channels (endpoint number and z2m name)
button_channels = [(2, "left"), (3, "right")]

# Make each test that uses sswitch fixture to run twice for both buttons. 
# sswitch stands a Switch object in a server mode
# Using the ids parameter the button name will be displayed as a test parameter
@pytest.fixture(scope = 'function', params = button_channels, ids=lambda x: x[1])
def sswitch(device, zigbee, request, pytestconfig):
    switch = SmartSwitch(device, zigbee, request.param[0], request.param[1], pytestconfig.getini('device_name'))
    switch.set_attribute('operation_mode', 'server')
    return switch


# List of logical client switch channels (endpoint number and z2m name)
# TODO: Rename endpoint that is associated with both buttons actions to 'both' when z2m supports this. It is named 'center' for now.
client_channels = [(2, "left"), (3, "right"), (4, "center")]

# Make each test that uses cswitch fixture to run for all logical channels (buttons + virtual channels)
# cswitch stands a Switch object in a client mode
# Using the ids parameter the button name will be displayed as a test parameter
@pytest.fixture(scope = 'function', params = client_channels, ids=lambda x: x[1])
def cswitch(device, zigbee, request, pytestconfig):
    switch = SmartSwitch(device, zigbee, request.param[0], request.param[1], pytestconfig.getini('device_name'))
    switch.set_attribute('operation_mode', 'client')
    return switch

# A fixture that creates SmartSwitch object for both buttons endpoint
@pytest.fixture(scope = 'function')
def bswitch(device, zigbee, pytestconfig):
    switch = SmartSwitch(device, zigbee, 4, 'center', pytestconfig.getini('device_name'))
    return switch

# Some tests needs to operate with two switch endpoints simultaneoulsy
@pytest.fixture(scope = 'function', params = button_channels, ids=lambda x: x[1])
def switch_pair(device, zigbee, request, pytestconfig):
    # Create a switch
    switch1 = SmartSwitch(device, zigbee, request.param[0], request.param[1], pytestconfig.getini('device_name'))
    switch1.set_attribute('operation_mode', 'server')

    # Search for another switch, not the same as one requested
    another_switch_name = None
    for button in button_channels:
        if button != request.param:
            another_switch_name = button

    # Create the requested switch
    switch2 = SmartSwitch(device, zigbee, another_switch_name[0], another_switch_name[1], pytestconfig.getini('device_name'))
    switch2.set_attribute('operation_mode', 'server')

    return (switch1, switch2)


# Iterate on all bindings that device currently has, and cleanup all On/Off and LevelCtrl bindings to the Coordinator
# These bindings change the device behavior, and must be cleared on start to avoid undesired behavior
def clear_coordinator_bindings(bridge, device_name):
    coordinator_addr = bridge.get_coordinator_address()

    for binding in bridge.get_device_bindings(device_name):
        if binding['cluster'] == "genOnOff" or binding['cluster'] == "genLevelCtrl":
            if binding['target_addr'] == coordinator_addr:
                bridge.send_unbind_request(binding['cluster'], f"{device_name}/{binding['endpoint']}", 'Coordinator')


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
