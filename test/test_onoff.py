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
EP3_SET_SWITCH_ACTIONS = "ZCL Write Attribute: Cluster 0007 Attrib 0010"
EP3_GET_SWITCH_ACTIONS = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=0010"
EP3_SET_RELAY_MODE = "ZCL Write Attribute: Cluster 0007 Attrib ff01"
EP3_GET_RELAY_MODE = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=ff01"
EP3_SET_MAX_PAUSE = "ZCL Write Attribute: Cluster 0007 Attrib ff02"
EP3_GET_MAX_PAUSE = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=ff02"
EP3_SET_MIN_LONG_PRESS = "ZCL Write Attribute: Cluster 0007 Attrib ff03"
EP3_GET_MIN_LONG_PRESS = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=ff03"
EP3_SET_LONG_PRESS_MODE = "ZCL Write Attribute: Cluster 0007 Attrib ff04"
EP3_GET_LONG_PRESS_MODE = "ZCL Read Attribute: EP=3 Cluster=0007 Command=00 Attr=ff04"


class SmartSwitch:
    def __init__(self, device, zigbee, ep, z2m_name):
        self.device = device
        self.zigbee = zigbee
        self.ep = ep
        self.z2m_name = z2m_name

        self.ON_MSG                 = f"SwitchEndpoint EP={ep}: do state change 1"
        self.OFF_MSG                = f"SwitchEndpoint EP={ep}: do state change 0"
        self.GET_STATE_MSG          = f"ZCL Read Attribute: EP={ep} Cluster=0006 Command=00 Attr=0000"
        self.SET_MODE_MSG           = f"ZCL Write Attribute: Cluster 0007 Attrib ff00"
        self.GET_MODE_MSG           = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=ff00"
        self.SET_SWITCH_ACTIONS_MSG = f"ZCL Write Attribute: Cluster 0007 Attrib 0010"
        self.GET_SWITCH_ACTIONS_MSG = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=0010"
        self.SET_RELAY_MODE_MSG     = f"ZCL Write Attribute: Cluster 0007 Attrib ff01"
        self.GET_RELAY_MODE_MSG     = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=ff01"
        self.SET_MAX_PAUSE_MSG      = f"ZCL Write Attribute: Cluster 0007 Attrib ff02"
        self.GET_MAX_PAUSE_MSG      = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=ff02"
        self.SET_MIN_LONG_PRESS_MSG = f"ZCL Write Attribute: Cluster 0007 Attrib ff03"
        self.GET_MIN_LONG_PRESS_MSG = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=ff03"
        self.SET_LONG_PRESS_MODE_MSG = f"ZCL Write Attribute: Cluster 0007 Attrib ff04"
        self.GET_LONG_PRESS_MODE_MSG = f"ZCL Read Attribute: EP={ep} Cluster=0007 Command=00 Attr=ff04"


    def switch(self, cmd, expected_state):
        msg = self.ON_MSG if expected_state else self.OFF_MSG
        return set_device_attribute(self.device, self.zigbee, 'state_'+self.z2m_name, cmd, msg)


    def get_state(self):
        return get_device_attribute(self.device, self.zigbee, 'state_'+self.z2m_name, self.GET_STATE_MSG)


    def get_attr_id_by_name(self, attr):
        match attr:
            case 'switch_mode':
                return 'ff00'
            case 'switch_actions':
                return '0010'
            case 'relay_mode':
                return 'ff01'
            case 'max_pause':
                return 'ff02'
            case 'min_long_press':
                return 'ff03'
            case 'long_press_mode':
                return 'ff04'
            case _:
                raise RuntimeError("Unknown attribute name")


    def set_attribute(self, attr, value):
        msg = f"ZCL Write Attribute: Cluster 0007 Attrib {self.get_attr_id_by_name(attr)}"
        return set_device_attribute(self.device, self.zigbee, attr + '_' + self.z2m_name, value, msg)


    def get_attribute(self, attr):
        msg = f"ZCL Read Attribute: EP={self.ep} Cluster=0007 Command=00 Attr={self.get_attr_id_by_name(attr)}"
        return get_device_attribute(self.device, self.zigbee, attr + '_' + self.z2m_name, msg)


def test_on_off(device, zigbee):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    assert switch.switch('ON', True) == 'ON'
    assert switch.switch('OFF', False) == 'OFF'


def test_toggle(device, zigbee):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    assert switch.switch('OFF', False) == 'OFF'
    assert switch.get_state() == 'OFF'

    assert switch.switch('TOGGLE', True) == 'ON'
    assert switch.get_state() == 'ON'

    assert switch.switch('TOGGLE', False) == 'OFF'
    assert switch.get_state() == 'OFF'


@pytest.mark.parametrize("switch_mode", ["toggle", "momentary", "multifunction"])
def test_oosc_attribute_switch_mode(device, zigbee, switch_mode):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    assert switch.set_attribute('switch_mode', switch_mode) == switch_mode
    assert switch.get_attribute('switch_mode') == switch_mode


@pytest.mark.parametrize("switch_actions", ["onOff", "offOn", "toggle"])
def test_oosc_attribute_switch_action(device, zigbee, switch_actions):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    assert switch.set_attribute('switch_actions', switch_actions) == switch_actions
    assert switch.get_attribute('switch_actions') == switch_actions


@pytest.mark.parametrize("relay_mode", ["unlinked", "front", "single", "double", "tripple", "long"])
def test_oosc_attribute_relay_mode(device, zigbee, relay_mode):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    assert switch.set_attribute('relay_mode', relay_mode) == relay_mode
    assert switch.get_attribute('relay_mode') == relay_mode


def test_oosc_attributes_survive_reboot(device, zigbee):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    # Set a specific OOSC options
    assert switch.set_attribute('switch_mode', 'multifunction') == 'multifunction'
    assert switch.set_attribute('relay_mode', 'double') == 'double'
    assert switch.set_attribute('long_press_mode', 'levelCtrlUp') == 'levelCtrlUp'
    assert switch.set_attribute('max_pause', '152') == '152'
    assert switch.set_attribute('min_long_press', '602') == '602'

    # Reset the device
    device.reset()

    # Expect the OOSC settings survive the reboot
    assert switch.get_attribute('switch_mode') == 'multifunction'
    assert switch.get_attribute('relay_mode') == 'double'
    assert switch.get_attribute('long_press_mode') == 'levelCtrlUp'
    assert switch.get_attribute('max_pause') == 152
    assert switch.get_attribute('min_long_press') == 602


def test_btn_press(device, zigbee):
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    # Ensure the switch is off on start, and the mode is 'toggle'
    assert switch.switch('OFF', False) == 'OFF'
    assert switch.set_attribute('switch_mode', 'toggle') == 'toggle'

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
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    # Ensure the switch is off on start, the mode is 'multifunction', and relay mode is 'double'
    assert switch.switch('OFF', False) == 'OFF'
    assert switch.set_attribute('switch_mode', 'multifunction') == 'multifunction'
    assert switch.set_attribute('relay_mode', 'double') == 'double'

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
    switch = SmartSwitch(device, zigbee, 3, "button_2")

    # Bind the endpoint with the coordinator
    send_bind_request(zigbee, "genLevelCtrl", "my_test_switch/3", "Coordinator")
    
    # Ensure the switch will generate levelCtrlDown messages on long press
    assert switch.set_attribute('switch_mode', 'multifunction') == 'multifunction'
    assert switch.set_attribute('relay_mode', 'unlinked') == 'unlinked'
    assert switch.set_attribute('long_press_mode', 'levelCtrlDown') == 'levelCtrlDown'

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
