import pytest
import time

from smartswitch import *


# Make sure the switch is in client mode for the tests below
@pytest.fixture
def cswitch(switch, bridge, device_name):
    # Set the Client mode
    switch.set_attribute('operation_mode', 'client')

    # Set all necessary bindings for the switch functions
    send_bind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")
    send_bind_request(bridge, "genOnOff", f"{device_name}/{switch.ep}", "Coordinator")

    yield switch

    # Cleanup bindings
    send_unbind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")
    send_unbind_request(bridge, "genOnOff", f"{device_name}/{switch.ep}", "Coordinator")


def test_toggle_mode_btn_press(cswitch):
    # In the toggle mode the switch will send a single 'toggle' command on button press
    cswitch.set_attribute('switch_mode', 'toggle')

    # Emulate short button press
    cswitch.press_button()
    cswitch.wait_button_state("PRESSED1")

    # Release the button
    cswitch.release_button()
    cswitch.wait_button_state("IDLE")

    # Check the device has sent the 'single' action to coordinator, and the 'toggle' command to the bound device
    assert cswitch.wait_zigbee_action() == cswitch.get_action_name("single")
    assert cswitch.wait_zigbee_msg()['debug']['command'] == 'commandToggle'


@pytest.mark.parametrize("switch_actions, press_command, release_command", [
    ('onOff', "commandOn", "commandOff"),
    ('offOn', "commandOff", "commandOn"),
    ('toggle', "commandToggle", "commandToggle")
])
def test_momentary(cswitch, switch_actions, press_command, release_command):
    # Ensure the switch is in 'momentary' mode
    cswitch.set_attribute('switch_mode', 'momentary')

    # long press mode shall be 'none' in order to not mix up with level ctrl messages
    cswitch.set_attribute('long_press_mode', 'none')

    # The relay mode shall be any other than 'unlinked', otherwise sending On/Off commands will be switched off
    cswitch.set_attribute('relay_mode', 'front')

    # This test is focused on various switch_actions options
    cswitch.set_attribute('switch_actions', switch_actions)

    # Emulate short button press
    cswitch.press_button()
    cswitch.wait_button_state("PRESSED1")

    # Check the device state changed, and the action is generated (in this particular order)
    assert cswitch.wait_zigbee_action() == cswitch.get_action_name("hold")
    assert cswitch.wait_zigbee_msg()['debug']['command'] == press_command

    time.sleep(1)

    # Release the button
    cswitch.release_button()
    cswitch.wait_button_state("IDLE")

    assert cswitch.wait_zigbee_action() == cswitch.get_action_name("release")
    assert cswitch.wait_zigbee_msg()['debug']['command'] == release_command



# @pytest.fixture
# def genLevelCtrl_bindings(bridge, switch, device_name):
#     # Bind the endpoint with the coordinator
#     send_bind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")

#     yield

#     # Cleanup bindings
#     send_unbind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")


# def test_level_control(switch, genLevelCtrl_bindings):
#     # Ensure the switch will generate levelCtrlDown messages on long press
#     switch.set_attribute('switch_mode', 'multifunction')
#     switch.set_attribute('relay_mode', 'unlinked')
#     switch.set_attribute('long_press_mode', 'levelCtrlDown')

#     # Emulate the long button press, wait until the switch transits to the long press state
#     switch.press_button()
#     switch.wait_button_state("PRESSED1")
#     switch.wait_button_state("LONG_PRESS")
#     switch.wait_report_multistate(255)  # 255 means button long press
#     switch.wait_report_level_ctrl("Move")

#     # Verify the Level Control Move command has been received by the coordinator
#     assert switch.wait_zigbee_action() == switch.get_action_name("hold")
#     assert switch.wait_zigbee_msg()['level_ctrl'] == {'command': 'commandMove', 'payload': {'movemode': 1, 'rate': 80}}

#     # Do not forget to release the button
#     switch.release_button()
#     switch.wait_button_state("IDLE")
#     switch.wait_report_multistate(0)
#     switch.wait_report_level_ctrl("Stop")

#     # Verify the Level Control Move command has been received by the coordinator
#     assert switch.wait_zigbee_action() == switch.get_action_name("release")
#     assert switch.wait_zigbee_msg()['level_ctrl']['command'] == 'commandStop'
