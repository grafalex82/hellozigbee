import pytest

from smartswitch import *

@pytest.mark.parametrize("cswitch_mode", ["toggle", "momentary", "multifunction"])
def test_attribute_cswitch_mode(cswitch, cswitch_mode):
    cswitch.set_attribute('switch_mode', cswitch_mode)
    assert cswitch.get_attribute('switch_mode') == cswitch_mode


@pytest.mark.parametrize("cswitch_actions", ["onOff", "offOn", "toggle"])
def test_attribute_cswitch_action(cswitch, cswitch_actions):
    cswitch.set_attribute('switch_actions', cswitch_actions)
    assert cswitch.get_attribute('switch_actions') == cswitch_actions


@pytest.mark.parametrize("relay_mode", ["unlinked", "front", "single", "double", "tripple", "long"])
def test_attribute_relay_mode(cswitch, relay_mode):
    cswitch.set_attribute('relay_mode', relay_mode)
    assert cswitch.get_attribute('relay_mode') == relay_mode


@pytest.mark.parametrize("operation_mode", ["server", "client"])
def test_attribute_operation_mode(sswitch, operation_mode):
    # Check operation mode to accept `server` and `client` values only for server endpoints
    sswitch.set_attribute('operation_mode', operation_mode)
    assert sswitch.get_attribute('operation_mode') == operation_mode


@pytest.mark.skip_on_one_button_devices
def test_attribute_client_only_operation_mode(bswitch):
    # Check that client only endpoint does not allow setting server mode
    bswitch.set_incorrect_attribute('operation_mode', 'server')
    assert bswitch.get_attribute('operation_mode') == 'client'


def test_attributes_survive_reboot(cswitch):
    # Set a specific OOSC options
    cswitch.set_attribute('operation_mode', 'client')
    cswitch.set_attribute('switch_mode', 'multifunction')
    cswitch.set_attribute('relay_mode', 'double')
    cswitch.set_attribute('long_press_mode', 'levelCtrlUp')
    cswitch.set_attribute('max_pause', '152')
    cswitch.set_attribute('min_long_press', '602')

    # Reset the device
    cswitch.reset()

    # Expect the OOSC settings survive the reboot
    assert cswitch.get_attribute('operation_mode') == 'client'
    assert cswitch.get_attribute('switch_mode') == 'multifunction'
    assert cswitch.get_attribute('relay_mode') == 'double'
    assert cswitch.get_attribute('long_press_mode') == 'levelCtrlUp'
    assert cswitch.get_attribute('max_pause') == 152
    assert cswitch.get_attribute('min_long_press') == 602
