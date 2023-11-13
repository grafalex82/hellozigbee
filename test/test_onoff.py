# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import time

from device import *



def test_connect(device):
    device.wait_str("Initialization of the Hello Zigbee Platform Finished")

def test_connect2(device):
    device.wait_str("Initialization of the Hello Zigbee Platform Finished")

def test_connect3(device):
    device.wait_str("Initialization of the Hello Zigbee Platform Finished")
