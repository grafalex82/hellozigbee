import pytest
import time

from smartswitch import *


# A fixture that creates a test_group, and cleans it up after all tests completed
@pytest.fixture(scope="session")
def group(bridge):
    # Remove previously created group (if any). Ignore errors if there was no group before.
    resp = delete_group(bridge, "test_group", 1234)

    # Create a brand new group
    resp = create_group(bridge, "test_group", 1234)
    assert resp['status'] == 'ok'
    yield

    # Cleanup our group
    resp = delete_group(bridge, "test_group", 1234)
    assert resp['status'] == 'ok'


# A fixture that adds the switch to the test group
# @pytest.fixture(scope="function")
# def switch_on_group(switch, group):
#     # Remove device from the group if it was there already
#     resp = switch.remove_from_group("test_group")
#     print(resp)

#     # Create a brand new group
#     resp = switch.add_to_group("test_group")
#     print(resp)
#     assert resp['status'] == 'ok'

#     yield switch

#     # Cleanup group membership
#     resp = switch.remove_from_group("test_group")
#     print(resp)
#     assert resp['status'] == 'ok'


# Test 1:
# - Create a test group (if not yet created)
# - Add endpoint to the group (if not yet created)
# - Send On/Off/Toggle (and later LevelCtrl) commands to the group
# - Verify the device handles these commands


def test_on_off_toggle(group):
    pass
    #    do_zigbee_request(switch_on_group.device, switch_on_group.zigbee, 'test_group/set', {"state":"ON"}, 'test_group', switch_on_group.get_state_change_msg(True))



    # group.switch('ON')
    # switch.wait_state('ON')
