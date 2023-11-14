# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import time

from device import *
from zigbee import *


# def test_on(device, zigbee):
#     device.wait_str("Initialization of the Hello Zigbee Platform Finished")
#     zigbee.publish('{"state":"ON"}')
#     device.wait_str("ON")
#     assert False

# SwitchEndpoint EP=2: do state change 0
# SwitchEndpoint EP=3: do state change 1

def test_on(zigbee):
    zigbee.publish('set', '{"state_button_2":"ON"}')
    assert False