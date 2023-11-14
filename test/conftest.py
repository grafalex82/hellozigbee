import pytest

def pytest_addoption(parser):
    parser.addini('port', 'COM port where the Zigbee Device is connected to')
