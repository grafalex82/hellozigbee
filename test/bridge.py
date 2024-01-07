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
