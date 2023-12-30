import pytest

def test_on(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON via On/Off cluster, and check it sets non-zero brightness level
    state, level = switch.switch('ON')
    assert level != 0


def test_on_toggle(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON via On/Off cluster, and check it sets non-zero brightness level
    state, level = switch.switch('TOGGLE')
    assert state == "ON"
    assert level != 0


def test_off(switch):
    # start with switch is ON
    switch.switch('ON')

    # Turn the switch off using OFF On/Off cluster command, and check the device switches off
    state, level = switch.switch('OFF')
    assert level == 0


def test_off_toggle(switch):
    # start with switch is ON
    switch.switch('ON')

    # Turn the switch OFF using TOGGLE command, and check it switches device off
    state, level = switch.switch('TOGGLE')
    assert state == "OFF"
    assert level == 0


def test_off_set_level(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Turn the switch ON by setting non-zeron brightness level
    state, level = switch.set_level(123)
    assert state == "ON"
    assert level == 123


def test_off_set_zero_level(switch):
    # start with switch is OFF
    switch.switch('OFF')

    # Set the zero brightness level, and verify the switch is still off
    state, level = switch.set_level(0)
    assert state == "OFF"
    assert level == 0


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


def test_restore_to_previous_level_via_on_off(switch):
    # Set a specific level
    state, level = switch.set_level(123)
    assert state == "ON"
    assert level == 123

    # Switch off and back to on
    switch.switch('OFF')    # Use On/Off cluster to switch off
    state, level = switch.switch('ON')
    assert state == "ON"
    assert level == 123


def test_restore_to_previous_level_via_level_ctrl(switch):
    # Set a specific level
    state, level = switch.set_level(123)
    assert state == "ON"
    assert level == 123

    # Switch off and back to on
    switch.set_level(0)     # Use LevelCtrl cluster to switch off
    state, level = switch.switch('ON')
    assert state == "ON"
    assert level == 123


