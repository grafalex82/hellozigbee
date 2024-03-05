import pytest

from device import *
from zigbee import *

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
        self.zigbee.subscribe(self.z2m_name)

        # Make sure the device is fresh and ready to operate
        self.reset()


    def reset(self):
        self.device.reset()
        self.wait_button_state('IDLE')


    def get_full_name(self):
        return f"{self.z2m_name}/{self.ep}"
    

    def get_action_name(self, action):
        return action + '_' + self.ep_name


    def do_zigbee_request(self, request_topic, payload, response_topic, expected_response):
        # Prepare for waiting a zigbee2mqtt response
        self.zigbee.subscribe(response_topic)

        # Publish the request
        self.zigbee.publish(request_topic, payload)

        # Verify that the device has received the request, and properly process it
        self.device.wait_str(expected_response)

        # Wait the response from zigbee2mqtt
        return self.zigbee.wait_msg(response_topic)


    def do_set_request(self, attribute, value, expected_response):
        # Send payload like {"state_button_3", "ON"} to the <device>/set topic
        # Wait for the new device state response
        payload = {attribute: value}
        response = self.do_zigbee_request(self.z2m_name + '/set', payload, self.z2m_name, expected_response)
        return response[attribute]


    def do_get_request(self, attribute, expected_response):
        # Send payload like {"state_button_3", ""} to the <device>/get topic
        # Wait for the new device state response
        payload = {attribute: ""}
        response = self.do_zigbee_request(self.z2m_name + '/get', payload, self.z2m_name, expected_response)
        return response[attribute]


    def get_state_change_msg(self, expected_state):
        state = "1" if expected_state == "ON" else ("0" if expected_state == "OFF" else "") 
        return f"SwitchEndpoint EP={self.ep}: do state change {state}"


    def switch(self, cmd, expected_state = None):
        # Calculate expected state. Toggle command may not be fully checked, but this ok as it simplifies the test
        if expected_state == None and cmd != "TOGGLE": 
            expected_state = cmd   

        # Send the On/Off/Toggle command, verify device log has the state change message
        msg = self.get_state_change_msg(expected_state)
        self.do_set_request('state_'+self.ep_name, cmd, msg)

        # Device will respond with On/Off state report
        state = self.wait_zigbee_state_change()

        # Verify response from Z2M if possible
        if expected_state != None:
            assert state == expected_state

        return state


    def get_state(self):
        msg = f"ZCL Read Attribute: EP={self.ep} Cluster=0006 Attr=0000"
        return self.do_get_request('state_'+self.ep_name, msg)


    def wait_device_state_change(self, expected_state):
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
            case 'operation_mode':
                return 'ff05'
            case 'interlock_mode':
                return 'ff06'
            case _:
                raise RuntimeError("Unknown attribute name")


    def set_attribute(self, attr, value):
        msg = f"ZCL Write Attribute: EP={self.ep} Cluster=0007 Attr={self.get_attr_id_by_name(attr)}"
        assert self.do_set_request(attr + '_' + self.ep_name, value, msg) == value
        self.wait_button_state('IDLE')


    def set_incorrect_attribute(self, attr, value):
        # Send the 'set' request to change the attribute value
        payload = {attr + '_' + self.ep_name: value}
        self.zigbee.publish(self.z2m_name + '/set', payload)

        # Verify that the device has received and rejected the request
        msg = f"ZCL Endpoint Callback: Check attribute {self.get_attr_id_by_name(attr)} on cluster 0007 range status 135"
        self.device.wait_str(msg)

        # As soon as device rejects the set request, do not expect anything from the z2m


    def get_attribute(self, attr):
        msg = f"ZCL Read Attribute: EP={self.ep} Cluster=0007 Attr={self.get_attr_id_by_name(attr)}"
        return self.do_get_request(attr + '_' + self.ep_name, msg)
    

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


    def wait_zigbee_msg(self):
        return self.zigbee.wait_msg(self.z2m_name)
    

    def wait_zigbee_state_change(self):
        return self.zigbee.wait_msg(self.z2m_name)['state_' + self.ep_name]


    def wait_zigbee_attribute_change(self, attribute):
        return self.zigbee.wait_msg(self.z2m_name)[attribute + '_' + self.ep_name]


    def wait_zigbee_action(self):
        return self.zigbee.wait_msg(self.z2m_name)['action']
