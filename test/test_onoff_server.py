import pytest

from smartswitch import *


def test_on_off(sswitch):
    sswitch.switch('ON')
    sswitch.switch('OFF')


def test_on_off_unlinked(sswitch):
    # Set 'unlinked' mode to indicate the button is unlinked from the relay
    sswitch.switch('OFF')
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Verify that the relay still can be toggled remotely
    sswitch.switch('ON')
    sswitch.switch('OFF')


def test_toggle(sswitch):
    sswitch.switch('OFF')
    assert sswitch.get_state() == 'OFF'

    sswitch.switch('TOGGLE', 'ON')
    assert sswitch.get_state() == 'ON'

    sswitch.switch('TOGGLE', 'OFF')
    assert sswitch.get_state() == 'OFF'


def test_toggle_mode_btn_press(sswitch):
    # Ensure the switch is off on start, and the mode is 'toggle'
    sswitch.switch('OFF')
    sswitch.set_attribute('switch_mode', 'toggle')

    # Set relay_mode other than unlinked - the device will switch state when button is pressed
    sswitch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # In the toggle mode the switch is triggered immediately on button press
    sswitch.wait_device_state_change(True)

    # Release the button
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # Check the device state changed, and the action is generated (in this particular order)
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("single")
    assert sswitch.wait_zigbee_state_change() == "ON"


def test_toggle_mode_relay_unlinked(sswitch):
    # Ensure the switch is off on start, and the mode is 'toggle'
    sswitch.switch('OFF')
    sswitch.set_attribute('switch_mode', 'toggle')

    # Set relay_mode to unlinked - the device will not change its state, but only send the single press action
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Emulate short button press
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # Release the button
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # Check the action is generated, but the state has not changed
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("single")
    assert sswitch.get_state() == "OFF"


@pytest.mark.parametrize("switch_actions, init_state, alter_state", [
    ('onOff', "OFF", "ON"),
    ('offOn', "ON", "OFF")
])
def test_momentary_on_off(sswitch, switch_actions, init_state, alter_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'
    alter_state_bool = alter_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    sswitch.set_attribute('switch_mode', 'momentary')
    sswitch.switch(init_state)

    # This test is focused on onOff and offOn switch actions
    sswitch.set_attribute('switch_actions', switch_actions)

    # Set relay_mode other than unlinked - the device will switch state when button is pressed, and then change again when depressed
    sswitch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # In the momentary mode the switch is triggered immediately on button press. The state is changed from init_state to alter_state
    sswitch.wait_device_state_change(alter_state_bool)

    # Release the button
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # Once the button is released, the state must get back to the init state
    sswitch.wait_device_state_change(init_state_bool)

    # Check the device state changed, and the action is generated (in this particular order)
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("hold")
    assert sswitch.wait_zigbee_state_change() == alter_state

    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("release")
    assert sswitch.wait_zigbee_state_change() == init_state


@pytest.mark.parametrize("init_state, alter_state", [
    ("OFF", "ON"),
    ("ON", "OFF")
])
def test_momentary_toggle(sswitch, init_state, alter_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'
    alter_state_bool = alter_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    sswitch.set_attribute('switch_mode', 'momentary')
    sswitch.switch(init_state)

    # This test is focused on the 'toggle' switch actions mode (withing 'momentary' switch mode)
    sswitch.set_attribute('switch_actions', 'toggle')

    # Set relay_mode other than unlinked - the device will switch state when button is pressed, and then change again when depressed
    sswitch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # In the momentary-toggle mode the switch is toggled immediately on button press. The state is changed from init_state to alter_state
    sswitch.wait_device_state_change(alter_state_bool)

    # Release the button
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # Once the button is released, the state must get back to the init state
    sswitch.wait_device_state_change(init_state_bool)

    # Check the device state changed, and the action is generated (in this particular order)
    # On the release the state must return to the original state
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("hold")
    assert sswitch.wait_zigbee_state_change() == alter_state

    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("release")
    assert sswitch.wait_zigbee_state_change() == init_state


@pytest.mark.parametrize("switch_actions, init_state", [
    ('onOff', "OFF"),
    ('offOn', "ON")
])
def test_momentary_on_off_unlinked(sswitch, switch_actions, init_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    sswitch.set_attribute('switch_mode', 'momentary')
    sswitch.switch(init_state)

    # This test is focused on onOff and offOn switch actions
    sswitch.set_attribute('switch_actions', switch_actions)

    # Set relay_mode to unlinked - the device will generate actions, but do not really toggle the state
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Emulate short button press
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # Release the button
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # Check that device actions were triggered, but the device state is unchanged
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("hold")
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("release")
    assert sswitch.get_state()  == init_state


def test_multifunction_front(sswitch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    sswitch.set_attribute('switch_mode', 'multifunction')
    sswitch.switch('OFF')

    # This test is focused on 'front' relay mode
    sswitch.set_attribute('relay_mode', 'front')

    # Emulate the button click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # In 'front' mode we expect the LED to toggle immediately on button press
    sswitch.wait_device_state_change(True)

    # Do not forget to release the button
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")
    sswitch.wait_button_state("IDLE")

    # Check the device state changed, and the single click action is generated
    # As a side effect of current state machine implementation, action gets aftecr state change if relay mode is front
    assert sswitch.wait_zigbee_state_change() == "ON"
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("single")
    

def test_multifunction_single(sswitch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    sswitch.set_attribute('switch_mode', 'multifunction')
    sswitch.switch('OFF')

    # This test is focused on 'single' relay mode
    sswitch.set_attribute('relay_mode', 'single')

    # Emulate the button click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")

    # Then release the button and wait until single press is detected
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")
    sswitch.wait_button_state("IDLE")

    # In the 'single' relay mode we expect the LED to toggle after the short button press
    sswitch.wait_device_state_change(True)

    # Check the device state changed, and the single click action is generated
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("single")
    assert sswitch.wait_zigbee_state_change() == "ON"


def test_multifunction_double(sswitch):
    # Ensure the switch is off on start, the mode is 'multifunction'
    sswitch.switch('OFF')
    sswitch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'double' relay mode
    sswitch.set_attribute('relay_mode', 'double')

    # Emulate the first click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")

    # Emulate the second click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED2")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE2")

    # We expect the LED to toggle after the second button click
    sswitch.wait_device_state_change(True)

    # Check the device state changed, and the double click action is generated
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("double")
    assert sswitch.wait_zigbee_state_change() == "ON"


def test_multifunction_tripple(sswitch):
    # Ensure the switch is off on start, the mode is 'multifunction'
    sswitch.switch('OFF')
    sswitch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'tripple' relay mode
    sswitch.set_attribute('relay_mode', 'tripple')

    # Emulate the first click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")

    # Emulate the second click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED2")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE2")

    # Emulate the third click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED3")
    sswitch.release_button()
    sswitch.wait_button_state("IDLE")

    # We expect the LED to toggle after the third click
    sswitch.wait_device_state_change(True)

    # Check the device state changed, and the double click action is generated
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("tripple")
    assert sswitch.wait_zigbee_state_change() == "ON"


def test_multifunction_unlinked_single(sswitch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    sswitch.set_attribute('switch_mode', 'multifunction')
    sswitch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Emulate a single button click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")

    # Wait intil button state machine detects single click
    sswitch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("single")
    assert sswitch.get_state() == 'OFF'


def test_multifunction_unlinked_double(sswitch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    sswitch.set_attribute('switch_mode', 'multifunction')
    sswitch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Emulate the first click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")

    # Emulate the second click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED2")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE2")

    # Wait intil button state machine detects double click
    sswitch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("double")
    assert sswitch.get_state() == 'OFF'


def test_multifunction_unlinked_tripple(sswitch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    sswitch.set_attribute('switch_mode', 'multifunction')
    sswitch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    sswitch.set_attribute('relay_mode', 'unlinked')

    # Emulate the first click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED1")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE1")

    # Emulate the second click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED2")
    sswitch.release_button()
    sswitch.wait_button_state("PAUSE2")

    # Emulate the third click
    sswitch.press_button()
    sswitch.wait_button_state("PRESSED3")
    sswitch.release_button()
    
    # Wait intil button state machine detects double click
    sswitch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert sswitch.wait_zigbee_action() == sswitch.get_action_name("tripple")
    assert sswitch.get_state() == 'OFF'


