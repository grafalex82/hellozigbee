import pytest
import time
import json
import paho.mqtt.client as mqtt

class ZigbeeNetwork():
    def __init__(self, options):
        self.topic = options.getini('device_mqtt_topic')
        self.z2m_topic = options.getini('bridge_mqtt_topic')
        self.message_received = None
        self.topic_to_wait = None

        self.client = mqtt.Client()
        self.client.on_message = self.on_message_received
        self.client.connect(options.getini('mqtt_server'), int(options.getini('mqtt_port')))


    def on_message_received(self, client, userdata, msg):
        print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
        if not self.topic_to_wait or msg.topic.startswith(self.topic_to_wait):
            self.message_received = msg.payload


    def get_topic(self, subtopic, bridge=False):
        if bridge:
            return self.z2m_topic + '/' + subtopic

        if subtopic:
            return self.topic + '/' + subtopic
        
        return self.topic


    def subscribe(self, subtopic = "", bridge=False):
        self.client.subscribe(self.get_topic(subtopic, bridge))
        self.message_received = None
        self.topic_to_wait = None


    def publish(self, subtopic, message, bridge=False):
        topic = self.get_topic(subtopic, bridge)

        if isinstance(message, dict):
            message = json.dumps(message)

        print(f"Sending message to '{topic}': {message}")
        self.client.publish(topic, message)


    def wait_msg(self, subtopic=None, bridge=False, timeout=60):
        # Set the topic to wait if we are waiting for a specific one, or set to None if any topic needs to be listen to
        self.topic_to_wait = None
        if subtopic != None:
            self.topic_to_wait = self.get_topic(subtopic, bridge)

        # Wait for a topic in the loop until timeout is due
        start_time = time.time()
        while self.message_received is None:
            self.client.loop(timeout=1)

            elapsed_time = time.time() - start_time
            if elapsed_time > timeout:
                raise TimeoutError
        
        payload = self.message_received.decode()
        self.message_received = None
        self.topic_to_wait = None
        return json.loads(payload)
            
            
    def disconnect(self):
        self.client.disconnect()
