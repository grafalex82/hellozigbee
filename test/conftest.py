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
    parser.addini('target_board',       'Name of the target board (options: E75-2G4M10S, QBKG11LM, QBKG12LM)')


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


# Ensure we are running tests for the proper board
@pytest.fixture(scope="session", autouse=True)
def target_board(pytestconfig, device_name, bridge):
    assert bridge.get_device_model(device_name) == pytestconfig.getini('target_board')
    yield pytestconfig.getini('target_board')


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
def server_channels(target_board):
    if target_board == "E75-2G4M10S" or target_board == "QBKG12LM":
        return [
            {"id": 2, "name": "left", "allowInterlock": True}, 
            {"id": 3, "name": "right", "allowInterlock": True}
        ]
    if target_board == "QBKG11LM":
        return [{"id": 2, "name": "button", "allowInterlock": False}]


def client_channels(target_board):
    if target_board == "E75-2G4M10S" or target_board == "QBKG12LM":
        return [
            {"id": 2, "name": "left"}, 
            {"id": 3, "name": "right"}, 
            {"id": 4, "name": "both"}
        ]
    if target_board == "QBKG11LM":
        return [{"id": 2, "name": "button"}]


def is_one_button_device(target_board):
    return target_board == "QBKG11LM"


def both_channel(target_board):
    assert not is_one_button_device(target_board
                                    )
    if target_board == "E75-2G4M10S" or target_board == "QBKG12LM":
        return {"id": 4, "name": "both"}
    return None


# Make each test or fixture parameterized with server or client channel list
def pytest_generate_tests(metafunc):
    if "server_channel" in metafunc.fixturenames:
        target_board = metafunc.config.getini("target_board")
        metafunc.parametrize("server_channel", server_channels(target_board), ids=lambda x: x["name"])
    if "client_channel" in metafunc.fixturenames:
        target_board = metafunc.config.getini("target_board")
        metafunc.parametrize("client_channel", client_channels(target_board), ids=lambda x: x["name"])


# Skip tests that are not applicable to the current board
def pytest_collection_modifyitems(config, items):
    target_board = config.getini("target_board")
    for item in items:
        if "skip_on_one_button_devices" in item.keywords and is_one_button_device(target_board):
            item.add_marker(pytest.mark.skip(reason="Not supported on one-button devices"))


# sswitch stands a Switch object in a server mode, interlock mode is off
@pytest.fixture(scope = 'function')
def sswitch(device, zigbee, server_channel, device_name):
    switch = SmartSwitch(device, zigbee, server_channel["id"], server_channel["name"], device_name)
    switch.set_attribute('operation_mode', 'server')

    if server_channel["allowInterlock"]:
        switch.set_attribute('interlock_mode', 'none')
        switch.wait_zigbee_attribute_change('interlock_mode') == 'none' # Setting `interlock_mode` attribute reads also buddy endpoint

    return switch

# cswitch stands a Switch object in a client mode
@pytest.fixture(scope = 'function')
def cswitch(device, zigbee, client_channel, device_name):
    switch = SmartSwitch(device, zigbee, client_channel["id"], client_channel["name"], device_name)
    switch.set_attribute('operation_mode', 'client')
    return switch

# A fixture that creates SmartSwitch object for both buttons endpoint
@pytest.fixture(scope = 'function')
def bswitch(device, zigbee, device_name, target_board):
    ch = both_channel(target_board)
    assert ch != None

    switch = SmartSwitch(device, zigbee, ch["id"], ch["name"], device_name)
    return switch


# Some tests needs to operate with two switch endpoints simultaneoulsy
@pytest.fixture(scope = 'function')
def switch_pair(device, zigbee, server_channel, device_name, target_board):
    # Create a switch
    switch1 = SmartSwitch(device, zigbee, server_channel["id"], server_channel["name"], device_name)
    switch1.set_attribute('operation_mode', 'server')

    # Search for another switch, not the same as one requested
    another_switch = None
    for button in server_channels(target_board):
        if button != server_channel:
            another_switch = button

    # Create the requested switch
    switch2 = SmartSwitch(device, zigbee, another_switch["id"], another_switch["name"], device_name)
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
