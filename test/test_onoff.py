import pytest

from smartswitch import *

def test_on_off(switch):
    switch.switch('ON')
    switch.switch('OFF')


def test_toggle(switch):
    switch.switch('OFF')
    assert switch.get_state() == 'OFF'

    switch.switch('TOGGLE', 'ON')
    assert switch.get_state() == 'ON'

    switch.switch('TOGGLE', 'OFF')
    assert switch.get_state() == 'OFF'


@pytest.mark.parametrize("switch_mode", ["toggle", "momentary", "multifunction"])
def test_oosc_attribute_switch_mode(switch, switch_mode):
    switch.set_attribute('switch_mode', switch_mode)
    assert switch.get_attribute('switch_mode') == switch_mode


@pytest.mark.parametrize("switch_actions", ["onOff", "offOn", "toggle"])
def test_oosc_attribute_switch_action(switch, switch_actions):
    switch.set_attribute('switch_actions', switch_actions)
    assert switch.get_attribute('switch_actions') == switch_actions


@pytest.mark.parametrize("relay_mode", ["unlinked", "front", "single", "double", "tripple", "long"])
def test_oosc_attribute_relay_mode(switch, relay_mode):
    switch.set_attribute('relay_mode', relay_mode)
    assert switch.get_attribute('relay_mode') == relay_mode


def test_oosc_attributes_survive_reboot(switch):
    # Set a specific OOSC options
    switch.set_attribute('switch_mode', 'multifunction')
    switch.set_attribute('relay_mode', 'double')
    switch.set_attribute('long_press_mode', 'levelCtrlUp')
    switch.set_attribute('max_pause', '152')
    switch.set_attribute('min_long_press', '602')

    # Reset the device
    switch.reset()

    # Expect the OOSC settings survive the reboot
    assert switch.get_attribute('switch_mode') == 'multifunction'
    assert switch.get_attribute('relay_mode') == 'double'
    assert switch.get_attribute('long_press_mode') == 'levelCtrlUp'
    assert switch.get_attribute('max_pause') == 152
    assert switch.get_attribute('min_long_press') == 602


def test_toggle_mode_btn_press(switch):
    # Ensure the switch is off on start, and the mode is 'toggle'
    switch.switch('OFF')
    switch.set_attribute('switch_mode', 'toggle')

    # Set relay_mode other than unlinked - the device will switch state when button is pressed
    switch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # In the toggle mode the switch is triggered immediately on button press
    switch.wait_device_state_change(True)

    # Release the button
    switch.release_button()
    switch.wait_button_state("IDLE")

    # Check the device state changed, and the action is generated (in this particular order)
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("single")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == "ON"


def test_toggle_mode_relay_unlinked(switch):
    # Ensure the switch is off on start, and the mode is 'toggle'
    switch.switch('OFF')
    switch.set_attribute('switch_mode', 'toggle')

    # Set relay_mode to unlinked - the device will not change its state, but only send the single press action
    switch.set_attribute('relay_mode', 'unlinked')

    # Emulate short button press
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # Release the button
    switch.release_button()
    switch.wait_button_state("IDLE")

    # Check the action is generated, but the state has not changed
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("single")
    assert switch.get_state() == "OFF"


@pytest.mark.parametrize("switch_actions, init_state, alter_state", [
    ('onOff', "OFF", "ON"),
    ('offOn', "ON", "OFF")
])
def test_momentary_on_off(switch, switch_actions, init_state, alter_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'
    alter_state_bool = alter_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    switch.set_attribute('switch_mode', 'momentary')
    switch.switch(init_state)

    # This test is focused on onOff and offOn switch actions
    switch.set_attribute('switch_actions', switch_actions)

    # Set relay_mode other than unlinked - the device will switch state when button is pressed, and then change again when depressed
    switch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # In the momentary mode the switch is triggered immediately on button press. The state is changed from init_state to alter_state
    switch.wait_device_state_change(alter_state_bool)

    # Release the button
    switch.release_button()
    switch.wait_button_state("IDLE")

    # Once the button is released, the state must get back to the init state
    switch.wait_device_state_change(init_state_bool)

    # Check the device state changed, and the action is generated (in this particular order)
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("hold")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == alter_state

    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("release")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == init_state


@pytest.mark.parametrize("init_state, alter_state", [
    ("OFF", "ON"),
    #("ON", "OFF")      # BUG: The code does not really toggles the state. but rather works as offOn mode
])
def test_momentary_toggle(switch, init_state, alter_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'
    alter_state_bool = alter_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    switch.set_attribute('switch_mode', 'momentary')
    switch.switch(init_state)

    # This test is focused on the 'toggle' switch actions mode (withing 'momentary' switch mode)
    switch.set_attribute('switch_actions', 'toggle')

    # Set relay_mode other than unlinked - the device will switch state when button is pressed, and then change again when depressed
    switch.set_attribute('relay_mode', 'front')

    # Emulate short button press
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # In the momentary-toggle mode the switch is toggled immediately on button press. The state is changed from init_state to alter_state
    switch.wait_device_state_change(alter_state_bool)

    # Release the button
    switch.release_button()
    switch.wait_button_state("IDLE")

    # Once the button is released, the state must get back to the init state
    switch.wait_device_state_change(init_state_bool)

    # Check the device state changed, and the action is generated (in this particular order)
    # On the release the state must return to the original state
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("hold")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == alter_state

    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("release")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == init_state



@pytest.mark.parametrize("switch_actions, init_state", [
    ('onOff', "OFF"),
    ('offOn', "ON")
])
def test_momentary_on_off_unlinked(switch, switch_actions, init_state):
    # Just handy variables
    init_state_bool = init_state == 'ON'

    # Ensure the switch is in init_state on start, and the mode is 'momentary'
    switch.set_attribute('switch_mode', 'momentary')
    switch.switch(init_state)

    # This test is focused on onOff and offOn switch actions
    switch.set_attribute('switch_actions', switch_actions)

    # Set relay_mode to unlinked - the device will generate actions, but do not really toggle the state
    switch.set_attribute('relay_mode', 'unlinked')

    # Emulate short button press
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # Release the button
    switch.release_button()
    switch.wait_button_state("IDLE")

    # Check that device actions were triggered, but the device state is unchanged
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("hold")
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("release")
    assert switch.get_state()  == init_state


def test_multifunction_front(switch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    switch.set_attribute('switch_mode', 'multifunction')
    switch.switch('OFF')

    # This test is focused on 'front' relay mode
    switch.set_attribute('relay_mode', 'front')

    # Emulate the button click
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # In 'front' mode we expect the LED to toggle immediately on button press
    switch.wait_device_state_change(True)

    # Do not forget to release the button
    switch.release_button()
    switch.wait_button_state("PAUSE1")
    switch.wait_button_state("IDLE")

    # Check the device state changed, and the single click action is generated
    # As a side effect of current state machine implementation, action gets aftecr state change if relay mode is front
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == "ON"
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("single")
    

def test_multifunction_single(switch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    switch.set_attribute('switch_mode', 'multifunction')
    switch.switch('OFF')

    # This test is focused on 'single' relay mode
    switch.set_attribute('relay_mode', 'single')

    # Emulate the button click
    switch.press_button()
    switch.wait_button_state("PRESSED1")

    # Then release the button and wait until single press is detected
    switch.release_button()
    switch.wait_button_state("PAUSE1")
    switch.wait_button_state("IDLE")

    # In the 'single' relay mode we expect the LED to toggle after the short button press
    switch.wait_device_state_change(True)

    # Check the device state changed, and the single click action is generated
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("single")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == "ON"


def test_multifunction_double(switch):
    # Ensure the switch is off on start, the mode is 'multifunction'
    switch.switch('OFF')
    switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'double' relay mode
    switch.set_attribute('relay_mode', 'double')

    # Emulate the first click
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.release_button()
    switch.wait_button_state("PAUSE1")

    # Emulate the second click
    switch.press_button()
    switch.wait_button_state("PRESSED2")
    switch.release_button()
    switch.wait_button_state("PAUSE2")

    # We expect the LED to toggle after the second button click
    switch.wait_device_state_change(True)

    # Check the device state changed, and the double click action is generated
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("double")
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == "ON"


def test_multifunction_tripple(switch):
    # Ensure the switch is off on start, the mode is 'multifunction'
    switch.switch('OFF')
    switch.set_attribute('switch_mode', 'multifunction')

    # This test is focused on 'tripple' relay mode
    switch.set_attribute('relay_mode', 'tripple')

    # Emulate the first click
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.release_button()
    switch.wait_button_state("PAUSE1")

    # Emulate the second click
    switch.press_button()
    switch.wait_button_state("PRESSED2")
    switch.release_button()
    switch.wait_button_state("PAUSE2")

    # Emulate the third click
    switch.press_button()
    switch.wait_button_state("PRESSED3")
    switch.release_button()
    switch.wait_button_state("IDLE")

    # We expect the LED to toggle after the third click
    switch.wait_device_state_change(True)

    # Check the device state changed, and the double click action is generated
    assert switch.wait_zigbee_state()[switch.get_z2m_attr_name('state')] == "ON"
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("tripple")


def test_multifunction_unlinked_single(switch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    switch.set_attribute('switch_mode', 'multifunction')
    switch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    switch.set_attribute('relay_mode', 'unlinked')

    # Emulate a single button click
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.release_button()
    switch.wait_button_state("PAUSE1")

    # Wait intil button state machine detects single click
    switch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("single")
    assert switch.get_state() == 'OFF'


def test_multifunction_unlinked_double(switch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    switch.set_attribute('switch_mode', 'multifunction')
    switch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    switch.set_attribute('relay_mode', 'unlinked')

    # Emulate the first click
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.release_button()
    switch.wait_button_state("PAUSE1")

    # Emulate the second click
    switch.press_button()
    switch.wait_button_state("PRESSED2")
    switch.release_button()
    switch.wait_button_state("PAUSE2")

    # Wait intil button state machine detects double click
    switch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("double")
    assert switch.get_state() == 'OFF'


def test_multifunction_unlinked_tripple(switch):
    # Ensure the switch is OFF on start, and the mode is 'multifunction'
    switch.set_attribute('switch_mode', 'multifunction')
    switch.switch('OFF')

    # This test is focused on 'unlinked' relay mode
    switch.set_attribute('relay_mode', 'unlinked')

    # Emulate the first click
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.release_button()
    switch.wait_button_state("PAUSE1")

    # Emulate the second click
    switch.press_button()
    switch.wait_button_state("PRESSED2")
    switch.release_button()
    switch.wait_button_state("PAUSE2")

    # Emulate the third click
    switch.press_button()
    switch.wait_button_state("PRESSED3")
    switch.release_button()
    
    # Wait intil button state machine detects double click
    switch.wait_button_state("IDLE")

    # Check the single click action is generated, but the state has not changed
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("tripple")
    assert switch.get_state() == 'OFF'


@pytest.fixture
def genLevelCtrl_bindings(bridge, switch, device_name):
    # Bind the endpoint with the coordinator
    send_bind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")

    yield

    # Cleanup bindings
    send_unbind_request(bridge, "genLevelCtrl", f"{device_name}/{switch.ep}", "Coordinator")


def test_level_control(switch, genLevelCtrl_bindings):
    # Ensure the switch will generate levelCtrlDown messages on long press
    switch.set_attribute('switch_mode', 'multifunction')
    switch.set_attribute('relay_mode', 'unlinked')
    switch.set_attribute('long_press_mode', 'levelCtrlDown')

    # Emulate the long button press, wait until the switch transits to the long press state
    switch.press_button()
    switch.wait_button_state("PRESSED1")
    switch.wait_button_state("LONG_PRESS")
    switch.wait_report_multistate(255)  # 255 means button long press
    switch.wait_report_level_ctrl("Move")

    # Verify the Level Control Move command has been received by the coordinator
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("hold")
    assert switch.wait_zigbee_state()['level_ctrl'] == {'command': 'commandMove', 'payload': {'movemode': 1, 'rate': 80}}

    # Do not forget to release the button
    switch.release_button()
    switch.wait_button_state("IDLE")
    switch.wait_report_multistate(0)
    switch.wait_report_level_ctrl("Stop")

    # Verify the Level Control Move command has been received by the coordinator
    assert switch.wait_zigbee_state()['action'] == switch.get_z2m_attr_name("release")
    assert switch.wait_zigbee_state()['level_ctrl']['command'] == 'commandStop'
