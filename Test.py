# pylint: disable=suppressed-message
# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring
import time

from JacobRobot import JacobRobot


class Test:
    def __init__(self, host, port, debug=False):
        self.rob = JacobRobot(host, port, debug=debug)
        print("Pruebas de RoboBase", flush=True)
        print("-------------------", flush=True)

    def test_info(self):
        print("Obteniendo info: ", flush=True, end="")
        print(self.rob.get_info(), flush=True)

    def test_beep(self, sec):
        ms = int(sec * 1000)
        print("\nGenerando sonidos:", flush=True)
        self.rob.beep(130.81, ms)  # Do3
        self.rob.beep(146.83, ms)  # Re3
        self.rob.beep(164.81, ms)  # Mi3
        self.rob.beep(174.61, ms)  # Fa3
        self.rob.beep(196.00, ms)  # Sol3
        self.rob.beep(220.00, ms)  # La3
        self.rob.beep(246.94, ms)  # Si3
        self.rob.beep(261.63, ms)  # Do4

    def test_ping(self, max_distance, sec):
        t = time.time()
        while time.time() - t < sec:
            print(self.rob.ping(max_distance), flush=True)
            time.sleep(0.1)

    def test_motors(self):
        print("\nMoviendo motor derecho hacia adelante:", flush=True)
        self.rob.set_motors(0, 255)
        time.sleep(4)
        self.rob.set_motors(0, 200)
        time.sleep(4)

        print("\nMoviendo motor izquierdo hacia adelante:", flush=True)
        self.rob.set_motors(255, 0)
        time.sleep(4)
        self.rob.set_motors(200, 0)
        time.sleep(4)

        print("\nMoviendo motor derecho hacia atras:", flush=True)
        self.rob.set_motors(0, -255)
        time.sleep(4)
        self.rob.set_motors(0, -200)
        time.sleep(4)

        print("\nMoviendo motor izquierdo hacia atras:", flush=True)
        self.rob.set_motors(-255, 0)
        time.sleep(4)
        self.rob.set_motors(-200, 0)
        time.sleep(4)

        self.rob.set_motors(0, 0)

    def close(self):
        self.rob.close()


# --- show time ---
app = Test("jacobrobot.local", 1963, True)
app.test_info()
app.test_beep(0.5)
# app.test_ping(25, 10)
app.test_motors()
app.close()
