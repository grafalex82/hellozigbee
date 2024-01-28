import pytest
import time

from smartswitch import *


# Make sure the switch is bound to the coordinator for the tests below
@pytest.fixture
def bound_switch(cswitch, bridge):
    # Set all necessary bindings for the switch functions
    bridge.send_bind_request("genLevelCtrl", cswitch.get_full_name(), "Coordinator")
    bridge.send_bind_request("genOnOff", cswitch.get_full_name(), "Coordinator")

    yield cswitch

    # Cleanup bindings
    bridge.send_unbind_request("genLevelCtrl", cswitch.get_full_name(), "Coordinator")
    bridge.send_unbind_request("genOnOff", cswitch.get_full_name(), "Coordinator")


def test_toggle_mode_btn_press(bound_switch):
    # In the toggle mode the switch will send a single 'toggle' command on button press
    bound_switch.set_attribute('switch_mode', 'toggle')

    # The relay mode shall be any other than 'unlinked', otherwise sending On/Off commands will be switched off
    bound_switch.set_attribute('relay_mode', 'front')    

    # Emulate short button press
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")

    # Release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")

    # Check the device has sent the 'single' action to coordinator, and the 'toggle' command to the bound device
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("single")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'


@pytest.mark.parametrize("switch_actions, press_command, release_command", [
    ('onOff', "commandOn", "commandOff"),
    ('offOn', "commandOff", "commandOn"),
    ('toggle', "commandToggle", "commandToggle")
])
def test_momentary(bound_switch, switch_actions, press_command, release_command):
    # Ensure the switch is in 'momentary' mode
    bound_switch.set_attribute('switch_mode', 'momentary')

    # long press mode shall be 'none' in order to not mix up with level ctrl messages
    bound_switch.set_attribute('long_press_mode', 'none')

    # The relay mode shall be any other than 'unlinked', otherwise sending On/Off commands will be switched off
    bound_switch.set_attribute('relay_mode', 'front')

    # This test is focused on various switch_actions options
    bound_switch.set_attribute('switch_actions', switch_actions)

    # Emulate short button press
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")

    # Check the device state changed, and the action is generated (in this particular order)
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("hold")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == press_command

    time.sleep(1)

    # Release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")

    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("release")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == release_command


def test_momentary_unlinked(bound_switch):
    # Ensure the switch is in 'momentary' mode, do not care about current switch_action mode
    bound_switch.set_attribute('switch_mode', 'momentary')

    # long press mode shall be 'none' in order to not mix up with level ctrl messages
    bound_switch.set_attribute('long_press_mode', 'none')

    # The relay mode in this test is 'unlinked', to ensure only actions are sent
    bound_switch.set_attribute('relay_mode', 'unlinked')

    # Emulate short button press
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")

    # Check the device state changed, and the action is generated (in this particular order)
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("hold")

    # Release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")

    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("release")


def test_multifunction_front(bound_switch):
    # Ensure the switch is in 'multifunction' mode
    bound_switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'front' relay mode
    bound_switch.set_attribute('relay_mode', 'front')

    # Emulate the button click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")

    # The toggle command is sent immediately
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'

    # Do not forget to release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE1")
    bound_switch.wait_button_state("IDLE")

    # The switch can detect single state only after the button is released
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("single")


def test_multifunction_single(bound_switch):
    # Ensure the switch is in 'multifunction' mode
    bound_switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'single' relay mode
    bound_switch.set_attribute('relay_mode', 'single')

    # Emulate the button click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")

    # Then release the button and wait until single press is detected
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE1")
    bound_switch.wait_button_state("IDLE")

    # Check the single click action is generated, and command to bound device is sent
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("single")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'


def test_multifunction_double(bound_switch):
    # Ensure the switch is in 'multifunction' mode
    bound_switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'double' relay mode
    bound_switch.set_attribute('relay_mode', 'double')

    # Emulate the first click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE1")

    # Emulate the second click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED2")
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE2")

    # Check that double click action is generated, and the switch sends toggle command to the bound device
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("double")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'


def test_multifunction_tripple(bound_switch):
    # Ensure the switch is in 'multifunction' mode
    bound_switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'tripple' relay mode
    bound_switch.set_attribute('relay_mode', 'tripple')

    # Emulate the first click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE1")

    # Emulate the second click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED2")
    bound_switch.release_button()
    bound_switch.wait_button_state("PAUSE2")

    # Emulate the third click
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED3")
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")

    # Check the device state changed, and the double click action is generated
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("tripple")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'


def test_multifunction_long_press(bound_switch):
    # Ensure the switch will generate action messages and toggle on long press
    bound_switch.set_attribute('switch_mode', 'multifunction')
    bound_switch.set_attribute('relay_mode', 'long')
    bound_switch.set_attribute('long_press_mode', 'none')
    bound_switch.set_attribute('switch_actions', 'onOff')

    # Emulate the long button press, wait until the switch transits to the long press state
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")
    bound_switch.wait_button_state("LONG_PRESS")
    bound_switch.wait_report_multistate(255)  # 255 means button long press

    # Verify the Level Control Move command has been received by the coordinator
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("hold")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandOn'

    # Do not forget to release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")
    bound_switch.wait_report_multistate(0)

    # Verify the Level Control Move command has been received by the coordinator
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("release")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandOff'


@pytest.mark.parametrize("long_press_mode, command, up_down", [
    ('levelCtrlDown', "commandMove", 1),        # Move down decreases brightness to minimum, but does not switch off
    ('levelCtrlUp', "commandMoveWithOnOff", 0)  # Move up will switch light on when moving up
])
def test_level_control(bound_switch, long_press_mode, command, up_down):
    # Ensure the switch will generate levelCtrlDown messages on long press
    bound_switch.set_attribute('switch_mode', 'multifunction')
    bound_switch.set_attribute('relay_mode', 'unlinked')
    bound_switch.set_attribute('long_press_mode', long_press_mode)

    # Emulate the long button press, wait until the switch transits to the long press state
    bound_switch.press_button()
    bound_switch.wait_button_state("PRESSED1")
    bound_switch.wait_button_state("LONG_PRESS")
    bound_switch.wait_report_multistate(255)  # 255 means button long press
    bound_switch.wait_report_level_ctrl("Move")

    # Verify the Level Control Move command has been received by the coordinator
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("hold")
    assert bound_switch.wait_zigbee_msg()['debug'] == {'command': command, 'endpoint': bound_switch.ep, 'payload': {'movemode': up_down, 'rate': 80}}

    # Do not forget to release the button
    bound_switch.release_button()
    bound_switch.wait_button_state("IDLE")
    bound_switch.wait_report_multistate(0)
    bound_switch.wait_report_level_ctrl("Stop")

    # Verify the Level Control Move command has been received by the coordinator
    assert bound_switch.wait_zigbee_action() == bound_switch.get_action_name("release")
    assert bound_switch.wait_zigbee_msg()['debug']['command'] == 'commandStop'
