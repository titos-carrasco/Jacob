# pylint: disable=suppressed-message
# pylint: disable=missing-module-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=missing-function-docstring

import time
import socket


class JacobRobot:
    PACKET_LENGTH = 8

    def __init__(self, host, port=1963, debug=False):
        self.debug = debug
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn.settimeout(5)
        self.conn.connect((host, port))
        if self.debug:
            print(f"Conectado a {host}:{port}", flush=True)

    def is_connected(self):
        return self.conn is not None

    def close(self):
        if self.conn:
            self.conn.close()
            self.conn = None

    def set_motors(self, motor1, motor2):
        motor1, motor2 = int(motor1), int(motor2)
        if motor1 < 0:
            motor1_dir = ord("B")
            motor1 = -motor1
        else:
            motor1_dir = ord("F")
        if motor2 < 0:
            motor2_dir = ord("B")
            motor2 = -motor2
        else:
            motor2_dir = ord("F")

        packet = self._make_packet("M")
        packet[1] = motor1_dir
        packet[2] = motor1 & 0xFF
        packet[3] = motor2_dir
        packet[4] = motor2 & 0xFF
        self._send_command(packet)

    def ping(self, max_distance):
        max_distance = abs(int(max_distance)) & 0xFFFF

        packet = self._make_packet("P")
        packet[1] = max_distance >> 8
        packet[2] = max_distance & 0xFF
        self._send_command(packet)

        ini = self._read_bytes(1)
        if ini != b"$":
            return 0

        r = self._read_int16() / 100.0

        fin = self._read_bytes(1)
        if fin != b"#":
            return 0

        return r

    def beep(self, freq, duration):
        freq = abs(int(freq)) & 0xFFFF
        duration = abs(int(duration)) & 0xFFFF

        packet = self._make_packet("B")
        packet[1] = freq >> 8
        packet[2] = freq & 0xFF
        packet[3] = duration >> 8
        packet[4] = duration & 0xFF
        self._send_command(packet)
        time.sleep(duration / 1000.0)

    def get_info(self):
        packet = self._make_packet("I")
        self._send_command(packet)
        r = self._read_line()
        return r

    ###################################################################
    def _make_packet(self, cmd: int) -> bytearray:
        packet = bytearray(self.PACKET_LENGTH)
        packet[0] = ord(cmd[0])
        return packet

    def _send_command(self, packet):
        packet = b"$" + packet + b"#"
        self.conn.sendall(packet)

        if self.debug:
            print("Enviando: ", flush=True, end="")
            print(list(packet), flush=True)

        r = self._read_bytes(1 + self.PACKET_LENGTH + 1)
        r = bytearray(r)
        if packet != r:
            print("Packet Mismatch: ", flush=True, end="")
            print(list(r), flush=True)

    def _read_line(self):
        line = b""
        while True:
            b = self.conn.recv(1)
            if b == b"\n":
                break
            line = line + b
        return line.decode("ascii", errors="")

    def _read_bytes(self, n):
        buff = bytearray(n)
        i = 0
        while i < n:
            buff[i] = ord(self.conn.recv(1))
            i = i + 1
        return bytes(buff)

    def _read_int8(self):
        return ord(self.conn.recv(1))

    def _read_int16(self):
        return (ord(self.conn.recv(1)) << 8) + ord(self.conn.recv(1))
