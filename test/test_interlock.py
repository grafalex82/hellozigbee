import pytest

from smartswitch import *

# Interlock feature is not available on one-button devices. Skip all tests in this file
pytestmark = pytest.mark.skip_on_one_button_devices


def test_independent(switch_pair):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Switch off interlock, check that the second switch also turns into the same mode
    switch1.set_attribute('interlock_mode', "none")
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'none'

    # Start with both switches off
    switch1.switch('OFF')
    switch2.switch('OFF')

    # Check that switch2 remains OFF regardless of switch1 state
    switch1.switch('ON')
    assert switch2.get_state() == 'OFF'
    switch1.switch('OFF')
    assert switch2.get_state() == 'OFF'    

    # Check that switch2 remains ON regardless of switch1 state
    switch2.switch('ON')
    switch1.switch('ON')
    assert switch2.get_state() == 'ON'
    switch1.switch('OFF')
    assert switch2.get_state() == 'ON'        


@pytest.mark.parametrize('switch2_init', ['OFF', 'ON'])
def test_mutual_exclusion(switch_pair, switch2_init):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Enable mutual exclusion mode, check that the second switch also turns into the same mode
    switch1.set_attribute('interlock_mode', "mutualExclusion")
    switch2.wait_zigbee_state_change()  # The buddy endpoint may change its state
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'mutualExclusion'

    # Set the switch2 initial state
    switch2.switch(switch2_init)
    switch1.wait_zigbee_state_change()  # Ignore interlocked endpoint change state message

    # When switch1 is off, the interlocked switch does not change its state
    switch1.switch('OFF')
    assert switch2.wait_zigbee_state_change() == switch2_init

    # Both switches cannot be ON, so switch2 goes OFF
    switch1.switch('ON')
    assert switch2.wait_zigbee_state_change() == 'OFF'

    # It is ok to have both switches OFF in the mutex mode
    switch1.switch('OFF')
    assert switch2.wait_zigbee_state_change() == 'OFF'    


def test_opposite(switch_pair):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Enable opposite mode, check that the second switch also turns into the same mode
    switch1.set_attribute('interlock_mode', 'opposite')
    switch2.wait_zigbee_state_change()
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'opposite'

    # When one switch gets ON, another gets OFF...
    switch1.switch('ON')
    assert switch2.wait_zigbee_state_change() == 'OFF'

    # ... and vice versa
    switch1.switch('OFF')
    assert switch2.wait_zigbee_state_change() == 'ON'
