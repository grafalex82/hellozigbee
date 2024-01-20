import pytest

from smartswitch import *

@pytest.mark.parametrize("switch_mode", ["toggle", "momentary", "multifunction"])
def test_attribute_switch_mode(switch, switch_mode):
    switch.set_attribute('switch_mode', switch_mode)
    assert switch.get_attribute('switch_mode') == switch_mode


@pytest.mark.parametrize("switch_actions", ["onOff", "offOn", "toggle"])
def test_attribute_switch_action(switch, switch_actions):
    switch.set_attribute('switch_actions', switch_actions)
    assert switch.get_attribute('switch_actions') == switch_actions


@pytest.mark.parametrize("relay_mode", ["unlinked", "front", "single", "double", "tripple", "long"])
def test_attribute_relay_mode(switch, relay_mode):
    switch.set_attribute('relay_mode', relay_mode)
    assert switch.get_attribute('relay_mode') == relay_mode


@pytest.mark.parametrize("operation_mode", ["server", "client"])
def test_attribute_operation_mode(switch, operation_mode):
    switch.set_attribute('operation_mode', operation_mode)
    assert switch.get_attribute('operation_mode') == operation_mode


def test_attributes_survive_reboot(switch):
    # Set a specific OOSC options
    switch.set_attribute('operation_mode', 'client')
    switch.set_attribute('switch_mode', 'multifunction')
    switch.set_attribute('relay_mode', 'double')
    switch.set_attribute('long_press_mode', 'levelCtrlUp')
    switch.set_attribute('max_pause', '152')
    switch.set_attribute('min_long_press', '602')

    # Reset the device
    switch.reset()

    # Expect the OOSC settings survive the reboot
    assert switch.get_attribute('operation_mode') == 'client'
    assert switch.get_attribute('switch_mode') == 'multifunction'
    assert switch.get_attribute('relay_mode') == 'double'
    assert switch.get_attribute('long_press_mode') == 'levelCtrlUp'
    assert switch.get_attribute('max_pause') == 152
    assert switch.get_attribute('min_long_press') == 602
