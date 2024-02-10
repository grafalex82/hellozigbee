import pytest

from smartswitch import *

def test_independent(switch_pair):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Switch off interlock
    switch1.set_attribute('interlock_mode', "none")
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'none'

    # Start with both switches off
    switch1.switch('OFF')
    switch2.switch('OFF')

    # Check that switch2 is OFF regardless of switch1 state
    switch1.switch('ON')
    assert switch2.get_state() == 'OFF'
    switch1.switch('OFF')
    assert switch2.get_state() == 'OFF'    

    # Check that switch2 is ON regardless of switch1 state
    switch2.switch('ON')
    switch1.switch('ON')
    assert switch2.get_state() == 'ON'
    switch1.switch('OFF')
    assert switch2.get_state() == 'ON'        