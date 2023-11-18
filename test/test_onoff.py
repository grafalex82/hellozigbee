# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import time
import re

from device import *
from zigbee import *




def set_device_state(device, zigbee, state, ep):
    # Make json payload like {"state_button_3", "ON"}
    channel = f"state_button_{ep-1}"
    payload = '{{"{}":"{}"}}'.format(channel, state)    

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Publish the 'set state' command
    zigbee.publish('set', payload)

    # Verify that the device has received the state change command
    device.wait_str(f"SwitchEndpoint EP={ep}: do state change")

    # Wait the reply from zigbee2mqtt with the device state
    state = zigbee.wait_msg()
    return re.search(f'"{channel}":"(.*?)"', state).group(1)


def get_device_state(device, zigbee, ep):
    # Make json payload like {"state_button_3", ""}
    channel = f"state_button_{ep-1}"
    payload = f'{{"{channel}":""}}'                     

    # Prepare for waiting a zigbee2mqtt message on the default device topic
    zigbee.subscribe()

    # Request the device state
    zigbee.publish('get', payload)                      

    # Verify the device received read attribute command
    device.wait_str(f"ZCL Read Attribute: EP={ep} Cluster=0006 Command=00 Attr=0000")

    # Wait the reply from zigbee2mqtt with the device state
    state = zigbee.wait_msg()
    return re.search(f'"{channel}":"(.*?)"', state).group(1)


def test_on_off(device, zigbee):    
    assert set_device_state(device, zigbee, 'ON', 3) == "ON"
    assert set_device_state(device, zigbee, 'OFF', 3) == "OFF"


def test_toggle(device, zigbee):
    assert set_device_state(device, zigbee, 'OFF', 3) == "OFF"
    assert get_device_state(device, zigbee, 3) == "OFF"

    assert set_device_state(device, zigbee, 'TOGGLE', 3) == "ON"
    assert get_device_state(device, zigbee, 3) == "ON"

    assert set_device_state(device, zigbee, 'TOGGLE', 3) == "OFF"
    assert get_device_state(device, zigbee, 3) == "OFF"
