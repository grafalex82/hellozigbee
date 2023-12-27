import time

class ZigbeeDevice():
    def __init__(self, port):
        self._port = port


    def reset(self):
        self._port.dtr = True
        time.sleep(0.1)
        self._port.dtr = False

        self._port.reset_input_buffer()

        self.wait_str("vAppMain(): Starting the main loop")


    def wait_str(self, str, timeout=15):
        tstart = time.time()
        while True:
            if tstart + timeout < time.time():
                raise TimeoutError()

            line = self._port.readline().decode().rstrip()
            print("    " + line)
            if str in line:
                return
            

    def send_str(self, str):
        print(f"Sending UART command: {str}")
        self._port.write((str + '\n').encode())
        self._port.flush()
