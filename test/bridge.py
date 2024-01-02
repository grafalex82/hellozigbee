from device import *
from zigbee import *

class Bridge:
    def __init__(self, zigbee):
        self.zigbee = zigbee
        self.topic = 'bridge'


    def get_topic(self, subtopic):
        return self.topic + '/' + subtopic


    def subscribe(self, subtopic):
        self.zigbee.subscribe(self.get_topic(subtopic))


    def publish(self, subtopic, payload):
        topic = self.get_topic(subtopic)
        print(f"Bridge: publish to topic '{topic}'")
        self.zigbee.publish(topic, payload)


    def wait_msg(self, subtopic=None, timeout=60):
        topic = self.get_topic(subtopic)
        self.zigbee.wait_msg(topic, timeout)


    def request(self, request_topic, payload, response_topic):
        # Prepare for waiting a zigbee2mqtt response
        self.zigbee.subscribe(self.get_topic(response_topic))

        # Publish the request
        self.zigbee.publish(self.get_topic(request_topic), payload)

        # Wait the response from zigbee2mqtt
        return self.zigbee.wait_msg(self.get_topic(response_topic))
