import time
import json
import paho.mqtt.client as mqtt

class ZigbeeNetwork():
    def __init__(self, server, port, base_topic):
        self.base_topic = base_topic

        self.message_received = None
        self.topic_to_wait = None

        self.retained_topics = {}

        self.client = mqtt.Client()
        self.client.on_message = self.on_message_received
        self.client.connect(server, int(port))


    def on_message_received(self, client, userdata, msg):
        # Cache retained topics in the own cache
        if msg.topic in self.retained_topics:
            print(f"Updated retained topic {msg.topic}")
            self.retained_topics[msg.topic] = msg.payload
        else:
            print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")

        # If the topic is the one we are waiting for - prepare for returning the payload to the client
        if not self.topic_to_wait or msg.topic.startswith(self.topic_to_wait):
            self.message_received = msg.payload


    def subscribe(self, topic):
        self.client.subscribe(self.base_topic + '/' + topic)
        self.message_received = None
        self.topic_to_wait = None


    def publish(self, topic, message):
        if isinstance(message, dict):
            message = json.dumps(message)

        topic = self.base_topic + '/' + topic
        print(f"Sending message to '{topic}': {message}")
        self.client.publish(topic, message)


    def wait_msg(self, topic=None, timeout=60):
        # Set the topic to wait if we are waiting for a specific one, or set to None if any topic needs to be listen to
        self.topic_to_wait = (self.base_topic + '/' + topic) if topic else None

        print(f"Waiting a message on topic '{self.topic_to_wait}'")

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
            
    
    def get_retained_topic(self, topic):
        full_topic = self.base_topic + '/' + topic
        if full_topic in self.retained_topics:
            return json.loads(self.retained_topics[full_topic].decode())
        
        self.retained_topics[full_topic] = ""
        self.subscribe(topic)
        return self.wait_msg(topic)

            
    def disconnect(self):
        self.client.disconnect()
