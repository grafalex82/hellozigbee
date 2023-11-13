# To run these tests install pytest, then run this command line:
# py.test -rfeEsxXwa --verbose --showlocals

import pytest
import serial
import time

@pytest.fixture
def port():
    return serial.Serial('COM5', baudrate=115200, timeout=1)


def wait_str(port, str, timeout=5):
    tstart = time.time()
    while True:
        if tstart + timeout < time.time():
            raise TimeoutError()

        line = port.readline().decode().rstrip()
        print(line)
        if str in line:
            return


def test_connect(port):
    wait_str(port, "Initialization of the Hello Zigbee Platform Finished")
