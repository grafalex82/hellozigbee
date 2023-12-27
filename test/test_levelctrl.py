import pytest

def test_on(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON, and check it sets non-zero brightness level
    state, level = switch.switch('ON')
    assert level != 0


def test_off_set_level(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON by setting non-zeron brightness level
    state, level = switch.set_level(123)
    assert state == "ON"
    assert level == 123


def test_on_set_level(switch):
    # start with switch is ON
    switch.switch('ON')

    # Change brightness to non-zero level
    state, level = switch.set_level(123)
    assert state == "ON"
    assert level == 123


def test_on_set_zero_level(switch):
    # start with switch is ON
    switch.switch('ON')

    # Change brightness to non-zero level
    state, level = switch.set_level(0)
    assert state == "OFF"
    assert level == 0
