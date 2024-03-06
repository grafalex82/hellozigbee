from device import *
from zigbee import *

class Bridge:
    def __init__(self, zigbee):
        self.zigbee = zigbee
        self.topic = 'bridge'


    def get_request_topic(self, subtopic):
        return self.topic + '/request/' + subtopic


    def get_response_topic(self, subtopic):
        return self.topic + '/response/' + subtopic


    def subscribe(self, subtopic):
        self.zigbee.subscribe(self.get_topic(subtopic))


    def publish(self, subtopic, payload):
        topic = self.get_request_topic(subtopic)
        print(f"Bridge: publish to topic '{topic}'")
        self.zigbee.publish(topic, payload)


    def wait_msg(self, subtopic, timeout=60):
        topic = self.get_response_topic(subtopic)
        self.zigbee.wait_msg(topic, timeout)


    def request(self, topic, payload):
        # Prepare for waiting a zigbee2mqtt response
        self.zigbee.subscribe(self.get_response_topic(topic))

        # Publish the request
        self.zigbee.publish(self.get_request_topic(topic), payload)

        # Wait the response from zigbee2mqtt
        return self.zigbee.wait_msg(self.get_response_topic(topic))


    def get_coordinator_address(self):
        info = self.zigbee.get_retained_topic(self.topic + '/devices')

        for device in info:
            if device['friendly_name'] == 'Coordinator':
                return device['ieee_address']

        return None


    def get_device_bindings(self, device_name):
        # Iterate on the list of all devices
        info = self.zigbee.get_retained_topic(self.topic + '/devices')

        bindings = []
        for device in info:
            # Search for the requested device
            if device['friendly_name'] != device_name:
                continue

            # Binding information is located in the 'endpoints' section
            for ep in device['endpoints']:
                ep_struct = device['endpoints'][ep]

                for binding in device['endpoints'][ep]['bindings']:
                    cluster = binding['cluster']

                    if binding['target']['type'] == 'group':
                        target_addr = binding['target']['id']
                        bindings.append({"endpoint": ep, "cluster": cluster, "target_addr": target_addr})
                    else:
                        target_addr = binding['target']['ieee_address']
                        target_ep = binding['target']['endpoint']
                        bindings.append({"endpoint": ep, "cluster": cluster, "target_addr": target_addr, "target_ep": target_ep})
    
        return bindings
    

    def get_device_model(self, device_name):
        # Iterate on the list of all devices, look for the requested device
        info = self.zigbee.get_retained_topic(self.topic + '/devices')
        for device in info:
            if device['friendly_name'] != device_name:
                continue

            # All devices for this project must identify itself as hello.zigbee.<board_name>
            return device['model_id'].replace("hello.zigbee.", "")



    def get_groups(self):
        return self.zigbee.get_retained_topic(self.topic + '/groups')


    def send_bind_request(self, clusters, src, dst):
        # clusters attribute must be a list
        if isinstance(clusters, str):
            clusters = [clusters]

        # Send the bind request
        payload = {"clusters": clusters, "from": src, "to": dst, "skip_disable_reporting": "true"}
        self.request('device/bind', payload)


    def send_unbind_request(self, clusters, src, dst):
        # clusters attribute must be a list
        if isinstance(clusters, str):
            clusters = [clusters]

        # Send the bind request
        payload = {"clusters": clusters, "from": src, "to": dst, "skip_disable_reporting": "true"}
        self.request('device/unbind', payload)
