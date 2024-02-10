import pytest

from smartswitch import *

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


def test_mutual_exclusion(switch_pair):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Enable mutual exclusion mode, check that the second switch also turns into the same mode
    switch1.set_attribute('interlock_mode', "mutualExclusion")
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'mutualExclusion'

    # Start with both switches off
    switch1.switch('OFF')
    switch2.switch('OFF')

    # Check that switch2 remains OFF regardless of switch1 state
    switch1.switch('ON')
    assert switch2.get_state() == 'OFF'
    switch1.switch('OFF')
    assert switch2.get_state() == 'OFF'    

    # When switch1 turns ON while switch2 is also ON, the switch2 shall get OFF
    switch2.switch('ON')
    switch1.switch('ON')
    assert switch2.wait_zigbee_state_change() == 'OFF'

    # It is ok to have both switches OFF in the mutex mode
    switch1.switch('OFF')   
    assert switch2.get_state() == 'OFF'


def test_opposite(switch_pair):
    switch1 = switch_pair[0]
    switch2 = switch_pair[1]

    # Enable opposite mode, check that the second switch also turns into the same mode
    switch1.set_attribute('interlock_mode', 'opposite')
    assert switch2.wait_zigbee_attribute_change('interlock_mode') == 'opposite'

    # When one switch gets ON, another gets OFF...
    switch1.switch('ON')
    assert switch2.wait_zigbee_state_change() == 'OFF'

    # ... and vice versa
    switch1.switch('OFF')
    assert switch2.wait_zigbee_state_change() == 'ON'
