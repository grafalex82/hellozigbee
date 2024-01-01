import pytest

from device import *
from zigbee import *

def do_zigbee_request(device, zigbee, request_topic, payload, response_topic, expected_response):
    # Prepare for waiting a zigbee2mqtt response
    zigbee.subscribe(response_topic)

    # Publish the request
    zigbee.publish(request_topic, payload)

    # Verify that the device has received the request, and properly process it
    device.wait_str(expected_response)

    # Wait the response from zigbee2mqtt
    return zigbee.wait_msg(response_topic)


def do_bridge_request(zigbee, request_topic, payload, response_topic):
    # Prepare for waiting a zigbee2mqtt response
    zigbee.subscribe(response_topic, True)

    # Publish the request
    zigbee.publish(request_topic, payload, True)

    # Wait the response from zigbee2mqtt
    return zigbee.wait_msg(response_topic, True)


def set_device_attribute(device, zigbee, attribute, value, expected_response):
    # Send payload like {"state_button_3", "ON"} to the <device>/set topic
    # Wait for the new device state response
    payload = {attribute: value}
    response = do_zigbee_request(device, zigbee, 'set', payload, "", expected_response)
    return response[attribute]


def get_device_attribute(device, zigbee, attribute, expected_response):
    # Send payload like {"state_button_3", ""} to the <device>/get topic
    # Wait for the new device state response
    payload = {attribute: ""}
    response = do_zigbee_request(device, zigbee, 'get', payload, "", expected_response)
    return response[attribute]


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
    payload = {"friendly_name": name, "id": id}
    return do_bridge_request(zigbee, 'request/group/add', payload, 'response/group/add')


def delete_group(zigbee, name, id):
    payload = {"friendly_name": name, "id": id}
    return do_bridge_request(zigbee, 'request/group/remove', payload, 'response/group/remove')


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
        payload = {
            "device":f"{self.z2m_name}/{self.ep}", 
            "group": group,
            "skip_disable_reporting": "true"
            }
        return do_bridge_request(self.zigbee, 'request/group/members/add', payload, 'response/group/members/add')


    def remove_from_group(self, group):
        payload = {
            "device":f"{self.z2m_name}/{self.ep}", 
            "group": group,
            "skip_disable_reporting": "true"
            }
        return do_bridge_request(self.zigbee, 'request/group/members/remove', payload, 'response/group/members/remove')


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
