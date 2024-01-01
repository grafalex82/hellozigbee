import pytest

from device import *
from zigbee import *

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


def create_group(zigbee, name, id):
    zigbee.subscribe("response/group/add", True)

    payload = {"friendly_name": name, "id": id}
    zigbee.publish('request/group/add', payload, bridge=True)
    return zigbee.wait_msg("response/group/add", True)


def delete_group(zigbee, name, id):
    zigbee.subscribe("response/group/remove", True)

    payload = {"friendly_name": name, "id": id}
    zigbee.publish('request/group/remove', payload, bridge=True)
    return zigbee.wait_msg("response/group/remove", True)


class SmartSwitch:
    """ 
    Smart Switch Test Harness

    This is a helper object that simplifies operating with the device via both UART and MQTT, performs routine checks,
    and moves communication burden from the test to the test harness.
    """

    def __init__(self, device, zigbee, ep, ep_name, z2m_name):
        # Remember parameters for further use
        self.device = device
        self.zigbee = zigbee
        self.ep = ep
        self.button = ep-1
        self.ep_name = ep_name
        self.z2m_name = z2m_name

        # Most of the tests will require device state MQTT messages. Subscribe for them
        self.zigbee.subscribe()

        # Make sure the device is fresh and ready to operate
        self.reset()


    def reset(self):
        self.device.reset()
        self.wait_button_state('IDLE')


    def get_state_change_msg(self, expected_state):
        state = "1" if expected_state == "ON" else ("0" if expected_state == "OFF" else "") 
        return f"SwitchEndpoint EP={self.ep}: do state change {state}"


    def switch(self, cmd, expected_state = None):
        # Calculate expected state. Toggle command may not be fully checked, but this ok as it simplifies the test
        if expected_state == None and cmd != "TOGGLE": 
            expected_state = cmd   

        # Send the On/Off/Toggle command, verify device log has the state change message
        msg = self.get_state_change_msg(expected_state)
        set_device_attribute(self.device, self.zigbee, 'state_'+self.ep_name, cmd, msg)

        # Device will respond with On/Off state report
        state = self.zigbee.wait_msg()['state_'+self.ep_name]

        # Verify response from Z2M if possible
        if expected_state != None:
            assert state == expected_state

        return state


    def get_state(self):
        msg = f"ZCL Read Attribute: EP={self.ep} Cluster=0006 Command=00 Attr=0000"
        return get_device_attribute(self.device, self.zigbee, 'state_'+self.ep_name, msg)


    def wait_state_change_msg(self, expected_state):
        msg = self.get_state_change_msg(expected_state)
        self.device.wait_str(msg)


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
        assert set_device_attribute(self.device, self.zigbee, attr + '_' + self.ep_name, value, msg) == value
        self.wait_button_state('IDLE')


    def get_attribute(self, attr):
        msg = f"ZCL Read Attribute: EP={self.ep} Cluster=0007 Command=00 Attr={self.get_attr_id_by_name(attr)}"
        return get_device_attribute(self.device, self.zigbee, attr + '_' + self.ep_name, msg)


    def add_to_group(self, group):
        self.zigbee.subscribe("response/group/members/add", True)

        payload = {
            "device":f"{self.z2m_name}/{self.ep}", 
            "group": group,
            "skip_disable_reporting": "true"
            }
        self.zigbee.publish('request/group/members/add', payload, True)

        return self.zigbee.wait_msg("response/group/members/add", True)


    def remove_from_group(self, group):
        self.zigbee.subscribe("response/group/members/remove", True)

        payload = {
            "device":f"{self.z2m_name}/{self.ep}", 
            "group": group,
            "skip_disable_reporting": "true"
            }
        self.zigbee.publish('request/group/members/remove', payload, True)

        return self.zigbee.wait_msg("response/group/members/remove", True)


    def press_button(self):
        cmd = f"BTN{self.button}_PRESS"
        self.device.send_str(cmd)


    def release_button(self):
        cmd = f"BTN{self.button}_RELEASE"
        self.device.send_str(cmd)


    def wait_button_state(self, state):
        state_str = f"Switching button {self.ep} state to {state}"
        self.device.wait_str(state_str)


    def wait_report_multistate(self, value):
        state_str = f"Reporting multistate action EP={self.ep} value={value}... status: 00"
        self.device.wait_str(state_str)


    def wait_report_level_ctrl(self, cmd):
        state_str = f"Sending Level Control {cmd} command status: 00"
        self.device.wait_str(state_str)


    def wait_zigbee_state(self):
        return self.zigbee.wait_msg()
