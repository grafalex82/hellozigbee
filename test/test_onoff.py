# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import time

from device import *
from zigbee import *
from conftest import *

EP3_ON = "SwitchEndpoint EP=3: do state change 1"
EP3_OFF = "SwitchEndpoint EP=3: do state change 0"
EP3_GET_STATE = "ZCL Read Attribute: EP=3 Cluster=0006 Command=00 Attr=0000"
EP3_SET_MODE = "ZCL Write Attribute: Cluster 0007 Attrib ff00"
EP3_GET_MODE = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=ff00"
EP3_SET_RELAY_MODE = "ZCL Write Attribute: Cluster 0007 Attrib ff01"
EP3_SET_LONG_PRESS_MODE = "ZCL Write Attribute: Cluster 0007 Attrib ff04"


def test_on_off(device, zigbee):    
    assert set_device_attribute(device, zigbee, 'state_button_2', 'ON', EP3_ON) == "ON"
    assert set_device_attribute(device, zigbee, 'state_button_2', 'OFF', EP3_OFF) == "OFF"


def test_toggle(device, zigbee):
    assert set_device_attribute(device, zigbee, 'state_button_2', 'OFF', EP3_OFF) == "OFF"
    assert get_device_attribute(device, zigbee, 'state_button_2', EP3_GET_STATE) == "OFF"

    assert set_device_attribute(device, zigbee, 'state_button_2', 'TOGGLE', EP3_ON) == "ON"
    assert get_device_attribute(device, zigbee, 'state_button_2', EP3_GET_STATE) == "ON"

    assert set_device_attribute(device, zigbee, 'state_button_2', 'TOGGLE', EP3_OFF) == "OFF"
    assert get_device_attribute(device, zigbee, 'state_button_2', EP3_GET_STATE) == "OFF"


def test_oosc_attributes(device, zigbee):
    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "toggle", EP3_SET_MODE) == "toggle"
    assert get_device_attribute(device, zigbee, 'switch_mode_button_2', EP3_GET_MODE) == "toggle"

    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "momentary", EP3_SET_MODE) == "momentary"
    assert get_device_attribute(device, zigbee, 'switch_mode_button_2', EP3_GET_MODE) == "momentary"

    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "multifunction", EP3_SET_MODE) == "multifunction"
    assert get_device_attribute(device, zigbee, 'switch_mode_button_2', EP3_GET_MODE) == "multifunction"


def test_btn_press(device, zigbee):
    # Ensure the switch is off on start, and the mode is 'toggle'
    assert set_device_attribute(device, zigbee, 'state_button_2', 'OFF', EP3_OFF) == "OFF"
    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "toggle", EP3_SET_MODE) == "toggle"

    zigbee.subscribe()

    # Emulate short button press
    device.send_str("BTN2_PRESS")
    device.wait_str("Switching button 3 state to PRESSED1")

    # In the toggle mode the switch is triggered immediately on button press
    device.wait_str(EP3_ON)

    # Release the button
    time.sleep(0.1)
    device.send_str("BTN2_RELEASE")
    device.wait_str("Switching button 3 state to IDLE")

    # Check the device state changed, and the action is generated (in this particular order)
    assert zigbee.wait_msg()['action'] == "single_button_2"
    assert zigbee.wait_msg()['state_button_2'] == "ON"


def test_double_click(device, zigbee):
    # Ensure the switch is off on start, the mode is 'multifunction', and relay mode is 'double'
    assert set_device_attribute(device, zigbee, 'state_button_2', 'OFF', EP3_OFF) == "OFF"
    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "multifunction", EP3_SET_MODE) == "multifunction"
    assert set_device_attribute(device, zigbee, 'relay_mode_button_2', "double", EP3_SET_RELAY_MODE) == "double"

    zigbee.subscribe()

    # Emulate the first click
    device.send_str("BTN2_PRESS")
    device.wait_str("Switching button 3 state to PRESSED1")
    time.sleep(0.1)
    device.send_str("BTN2_RELEASE")
    device.wait_str("Switching button 3 state to PAUSE1")

    # Emulate the second click
    device.send_str("BTN2_PRESS")
    device.wait_str("Switching button 3 state to PRESSED2")
    time.sleep(0.1)
    device.send_str("BTN2_RELEASE")
    device.wait_str("Switching button 3 state to PAUSE2")

    # We expect the LED to toggle after the second button click
    device.wait_str(EP3_ON)

    # Check the device state changed, and the double click action is generated
    assert zigbee.wait_msg()['action'] == "double_button_2"
    assert zigbee.wait_msg()['state_button_2'] == "ON"


def test_level_control(device, zigbee):
    # Bind the endpoint with the coordinator
    send_bind_request(zigbee, "genLevelCtrl", "my_test_switch/3", "Coordinator")
    
    # Ensure the switch will generate levelCtrlDown messages on long press
    assert set_device_attribute(device, zigbee, 'switch_mode_button_2', "multifunction", EP3_SET_MODE) == "multifunction"
    assert set_device_attribute(device, zigbee, 'relay_mode_button_2', "unlinked", EP3_SET_RELAY_MODE) == "unlinked"
    assert set_device_attribute(device, zigbee, 'long_press_mode_button_2', "levelCtrlDown", EP3_SET_LONG_PRESS_MODE) == "levelCtrlDown"

    zigbee.subscribe()

    # Emulate the long button press, wait until the switch transits to the long press state
    device.send_str("BTN2_PRESS")
    device.wait_str("Switching button 3 state to PRESSED1")
    device.wait_str("Switching button 3 state to LONG_PRESS")
    device.wait_str("Reporting multistate action EP=3 value=255... status: 00")
    device.wait_str("Sending Level Control Move command status: 00")

    # Verify the Level Control Move command has been received by the coordinator
    assert zigbee.wait_msg()['action'] == "hold_button_2"
    assert zigbee.wait_msg()['level_ctrl'] == {'command': 'commandMove', 'payload': {'movemode': 1, 'rate': 80}}

    # Do not forget to release the button
    device.send_str("BTN2_RELEASE")
    device.wait_str("Switching button 3 state to IDLE")
    device.wait_str("Reporting multistate action EP=3 value=0... status: 00")
    device.wait_str("Sending Level Control Stop command status: 00")

    # Verify the Level Control Move command has been received by the coordinator
    assert zigbee.wait_msg()['action'] == "release_button_2"
    assert zigbee.wait_msg()['level_ctrl']['command'] == 'commandStop'
