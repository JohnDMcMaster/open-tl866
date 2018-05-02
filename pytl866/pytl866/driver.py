import re
import serial

class Tl866Driver():
    def __init__(self, device, baud_rate=115200, ignore_fail=True):
        self.read_re = re.compile(r"(?:Ok|Fail)\s*([0-9A-F]{1,10})(?:\n|\r\n)")
        self.handle = serial.Serial(device, baud_rate)
        self.ignore_fail = True  # Not used right now, but perhaps check whether command errored out?

    # Create a command without actually sending it. Useful for sending
    # commands in bulk.
    def mk_cmd(cmd, val):
        return cmd + " " + format(val, "010X") + "\n"

    def get_retval():
        ret = self.handle.readline()
        match = self.read_re.match(ret)

        if match is None:
            raise DriverError(ret, "Unexpected response from Tl866.")
        else:
            return int(match.group(1), 16)

    def mk_and_send_cmd(cmd, val):
        cmd_str = self.mk_cmd(cmd, val)
        self.handle.write(cmd_str)
        return self.get_retval()


class DriverError(Exception):
    def __init__(self, str, message):
        self.str = str
        self.message = message
