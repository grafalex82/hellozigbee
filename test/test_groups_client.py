import pytest

from smartswitch import *


# Groups Client scenarios are absolutely the same as normal Client On/Off tests. That is because sending commands
# to a bound group uses the same binding mechanism as a binding to a light. So all functions that suppose to work
# in the Client On/Off scenarios will work absolutely the same in Groups Client mode. Thus this file contains
# just a single test to verify the overall mechanism, while all the device side scenarios are covered in
# test_onoff_client.py

# Make sure the switch is bound to the group for the tests below
@pytest.fixture
def bound_switch(cswitch, bridge, group):
    # Set all necessary bindings for the switch functions
    bridge.send_bind_request("genOnOff", cswitch.get_full_name(), group.get_name())

    yield cswitch

    # Cleanup bindings
    bridge.send_unbind_request("genOnOff", cswitch.get_full_name(), group.get_name())


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
    