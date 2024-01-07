from zigbee import *

class Group:
    def __init__(self, zigbee, bridge, name, id):
        self.zigbee = zigbee
        self.bridge = bridge
        self.name = name
        self.id = id


    def create(self):
        payload = {"friendly_name": self.name, "id": self.id}
        return self.bridge.request('group/add', payload)


    def delete(self):
        payload = {"friendly_name": self.name, "id": self.id}
        return self.bridge.request('group/remove', payload)


    def add_device(self, device):
        payload = {
            "device": device.get_full_name(), 
            "group": self.name,
            "skip_disable_reporting": "true"
            }
        return self.bridge.request('group/members/add', payload)


    def remove_device(self, device):
        payload = {
            "device": device.get_full_name(), 
            "group": self.name,
            "skip_disable_reporting": "true"
            }
        return self.bridge.request('group/members/remove', payload)


    def switch(self, state):
        # Prepare for waiting a group state response
        self.zigbee.subscribe(self.name)

        # Publish the request
        payload = {"state": state}
        self.zigbee.publish(self.name + "/set", payload)

        # Wait the response from zigbee2mqtt (actually the state may not be really relevant, but returning just in case)
        return self.zigbee.wait_msg(self.name)
