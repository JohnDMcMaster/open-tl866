import re
import serial

(VPP_98, VPP_126, VPP_140, VPP_166, VPP_144, VPP_171, VPP_185, VPP_212) = range(8)
# My measurements: 9.83, 12.57, 14.00, 16.68, 14.46, 17.17, 18.56, 21.2

(VDD_30, VDD_35, VDD_46, VDD_51, VDD_43, VDD_48, VDD_60, VDD_65) = range(8)
# My measurements: 2.99, 3.50, 4.64, 5.15, 4.36, 4.86, 6.01, 6.52

class Tl866Driver():
    def __init__(self, device, baud_rate=115200, ignore_fail=True):
        self.read_re = re.compile(rb"(?:Ok|Fail)\s*([0-9A-F]{1,10})(?:\n|\r\n)")
        self.handle = serial.Serial(device, baud_rate)
        self.ignore_fail = True  # Not used right now, but perhaps check whether command errored out?

    # Create a command without actually sending it. Useful for sending
    # commands in bulk.
    def mk_cmd(self, cmd, val):
        return (cmd + " " + format(val, "010X") + "\n").encode("utf-8")

    def get_retval(self):
        ret = self.handle.readline()
        match = self.read_re.match(ret)

        if match is None:
            raise DriverError(ret, "Unexpected response from Tl866.")
        else:
            return int(match.group(1), 16)

    def mk_and_send_cmd(self, cmd, val):
        cmd_str = self.mk_cmd(cmd, val)
        self.handle.write(cmd_str)
        return self.get_retval()

    # Thin command wrappers
    def dd(self):
        return self.mk_and_send_cmd("dd", 0)

    def de(self):
        return self.mk_and_send_cmd("de", 0)

    def ds(self, val):
        return self.mk_and_send_cmd("ds", val)

    def dw(self, val):
        return self.mk_and_send_cmd("dw", val)

    def ee(self):
        return self.mk_and_send_cmd("ee", 0)

    def eo(self):
        return self.mk_and_send_cmd("eo", 0)

    def gw(self, val):
        return self.mk_and_send_cmd("gw", val)

    def ll(self):
        return self.mk_and_send_cmd("ll", 0)

    def lo(self):
        return self.mk_and_send_cmd("lo", 0)

    def lq(self):
        return self.mk_and_send_cmd("lq", 0)

    def mm(self):
        return self.mk_and_send_cmd("mm", 0)

    def mo(self):
        return self.mk_and_send_cmd("mo", 0)

    def pd(self):
        return self.mk_and_send_cmd("pd", 0)

    def pe(self):
        return self.mk_and_send_cmd("pe", 0)

    def ps(self, val):
        return self.mk_and_send_cmd("ps", val)

    def pw(self, val):
        return self.mk_and_send_cmd("pw", val)

    def zd(self, val):
        return self.mk_and_send_cmd("zd", val)

    def zr(self):
        return self.mk_and_send_cmd("zr", 0)

    def zw(self, val):
        return self.mk_and_send_cmd("zw", val)


class DriverError(Exception):
    def __init__(self, str, message):
        self.str = str
        self.message = message
