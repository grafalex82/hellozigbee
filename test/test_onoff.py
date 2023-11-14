# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import time

from device import *
from zigbee import *


def test_on_off(device, zigbee):
    zigbee.publish('set', '{"state_button_2":"ON"}')
    device.wait_str("SwitchEndpoint EP=3: do state change 1")

    zigbee.publish('set', '{"state_button_2":"OFF"}')
    device.wait_str("SwitchEndpoint EP=3: do state change 0")


def test_toggle(device, zigbee):
    zigbee.publish('set', '{"state_button_2":"TOGGLE"}')
    device.wait_str("SwitchEndpoint EP=3: do state change 1")

    zigbee.publish('set', '{"state_button_2":"TOGGLE"}')
    device.wait_str("SwitchEndpoint EP=3: do state change 0")
