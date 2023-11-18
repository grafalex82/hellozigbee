import pytest
import serial
import time

@pytest.fixture(scope="session")
def port(pytestconfig):
    ser = serial.Serial(pytestconfig.getini("port"), baudrate=115200, timeout=1)
    ser.dtr = False # Release reset signal so that the device can boot
    yield ser


class ZigbeeDevice():
    def __init__(self, port):
        self._port = port


    def reset(self):
        self._port.dtr = True
        time.sleep(0.1)
        self._port.dtr = False

        self._port.reset_input_buffer()

        self.wait_str("vAppMain(): Starting the main loop")


    def wait_str(self, str, timeout=5):
        tstart = time.time()
        while True:
            if tstart + timeout < time.time():
                raise TimeoutError()

            line = self._port.readline().decode().rstrip()
            print("    " + line)
            if str in line:
                return



@pytest.fixture(scope="session")
def device_session(port):
    dev = ZigbeeDevice(port)
    yield dev


@pytest.fixture(scope="function")
def device(device_session):
    device_session.reset()
    yield device_session
