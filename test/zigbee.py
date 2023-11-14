import pytest
import paho.mqtt.client as mqtt


class ZigbeeNetwork():
    def __init__(self, options):
        self.client = mqtt.Client()
        self.client.connect(options.getini('mqtt_server'), int(options.getini('mqtt_port')))
        self.topic = options.getini('mqtt_topic')


    def publish(self, subtopic, message):
        topic = self.topic + '/' + subtopic
        print(f"Sending message to '{topic}'")
        self.client.publish(topic, message)


    def disconnect(self):
        self.client.disconnect()


@pytest.fixture(scope="session")
def zigbee(pytestconfig):
    net = ZigbeeNetwork(pytestconfig)
    yield net
    net.disconnect()
