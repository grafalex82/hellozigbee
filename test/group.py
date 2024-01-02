from zigbee import *

class Group:
    def __init__(self, zigbee, bridge, name, id):
        self.zigbee = zigbee
        self.bridge = bridge
        self.name = name
        self.id = id


    def create(self):
        payload = {"friendly_name": self.name, "id": self.id}
        return self.bridge.request('request/group/add', payload, 'response/group/add')


    def delete(self):
        payload = {"friendly_name": self.name, "id": self.id}
        return self.bridge.request('request/group/remove', payload, 'response/group/remove')


    def add_device(self, device):
        payload = {
            "device": device.get_full_name(), 
            "group": self.name,
            "skip_disable_reporting": "true"
            }
        return self.bridge.request('request/group/members/add', payload, 'response/group/members/add')


    def remove_device(self, device):
        payload = {
            "device": device.get_full_name(), 
            "group": self.name,
            "skip_disable_reporting": "true"
            }
        return self.bridge.request('request/group/members/remove', payload, 'response/group/members/remove')

