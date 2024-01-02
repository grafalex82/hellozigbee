import pytest

from smartswitch import *



# A fixture that adds the switch to the test group
@pytest.fixture(scope="function")
def switch_on_group(switch, group):
    # Remove device from the group if it was there already
    resp = group.remove_device(switch)
    print(resp)

    # Create a brand new group
    resp = group.add_device(switch)
    print(resp)
    assert resp['status'] == 'ok'

    yield switch

    # Cleanup group membership
    resp = group.remove_device(switch)
    print(resp)
    assert resp['status'] == 'ok'


# Test 1:
# - Create a test group (if not yet created)
# - Add endpoint to the group (if not yet created)
# - Send On/Off/Toggle (and later LevelCtrl) commands to the group
# - Verify the device handles these commands


def test_on_off_toggle(switch_on_group):
    pass

    # group.switch('ON')
    # switch.wait_state('ON')
