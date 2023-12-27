import pytest

def test_on(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON, and check it sets non-zero brightness level
    state, level = switch.switch('ON')
    assert level != 0

