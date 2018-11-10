'''
Exposes low level primitives and nothing more
'''

import re
import serial
from time import sleep
import pexpect
import pexpect.fdpexpect
import os
import sys
import errno
import time
import binascii
import struct

VPPS = (VPP_98, VPP_126, VPP_140, VPP_166, VPP_144, VPP_171, VPP_185,
        VPP_212) = range(8)
# My measurements: 9.83, 12.57, 14.00, 16.68, 14.46, 17.17, 18.56, 21.2

VDDS = (VDD_30, VDD_35, VDD_46, VDD_51, VDD_43, VDD_48, VDD_60,
        VDD_65) = range(8)

# My measurements: 2.99, 3.50, 4.64, 5.15, 4.36, 4.86, 6.01, 6.52

# Package pin #
VPP_PINS = set([1, 2, 3, 4, 9, 10, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40])
VDD_PINS = set([
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 21, 30, 32, 33, 34, 35, 36, 37,
    38, 39, 40
])

# 0 indexed
VPP_PINS0 = set([x - 1 for x in VPP_PINS])
VDD_PINS0 = set([x - 1 for x in VDD_PINS])
'''
icky workaround
read_nonblocking() is treating empty read as EOF for some reason
"BSD-style EOF"
'''


def my_read_nonblocking(self, size=1, timeout=None):
    """This reads data from the file descriptor.

    This is a simple implementation suitable for a regular file. Subclasses using ptys or pipes should override it.

    The timeout parameter is ignored.
    """

    try:
        s = os.read(self.child_fd, size)
    except OSError as err:
        if err.args[0] == errno.EIO:
            # Linux-style EOF
            self.flag_eof = True
            raise pexpect.exceptions.EOF(
                'End Of File (EOF). Exception style platform.')
        raise
    # FIXME: file pexpect issue?
    """
    if s == b'':
        # BSD-style EOF
        self.flag_eof = True
        raise EOF('End Of File (EOF). Empty string style platform.')
    """

    s = self._decoder.decode(s, final=False)
    self._log(s, 'read')
    return s


pexpect.spawnbase.SpawnBase.read_nonblocking = my_read_nonblocking


class NoSuchLine(Exception):
    pass


class Timeout(Exception):
    pass


# ASCII serial port to be compatible with expect
class ASerial(serial.Serial):
    def __init__(self, *args, **kwargs):
        serial.Serial.__init__(self, *args, **kwargs)

    def read(self, n):
        ret = serial.Serial.read(self, n)
        return ret.decode('ascii', 'ignore')

    def write(self, data):
        serial.Serial.write(self, data.encode('ascii', 'ignore'))


class AClient:
    '''ASCII client common'''

    # Help menu printing: open-tl866 (APP)
    APP = None

    def __init__(self, device, verbose=False):
        self.verbose = verbose
        self.verbose = True
        self.verbose and print("port: %s" % device)
        self.ser = ASerial(device, timeout=0, baudrate=115200, writeTimeout=0)
        self.ser.flushInput()
        self.e = pexpect.fdpexpect.fdspawn(self.ser.fileno(), encoding="ascii")
        self.assert_ver()

    def expect(self, s, timeout=0.5):
        self.e.expect(s, timeout=timeout)
        return self.e.before

    def cmd(self, cmd, reply=True, *args):
        '''Send raw command and get string result'''
        cmd = str(cmd)
        if len(cmd) != 1:
            raise ValueError('Invalid cmd %s' % cmd)
        # So far no commands have more than one arg
        if len(args) > 1:
            raise ValueError('cmd must take no more than 1 arg')
        strout = cmd + " " + ''.join([str(arg) for arg in args]) + "\n"
        self.verbose and print("cmd out: %s" % strout.strip())
        self.ser.write(strout)
        self.ser.flush()

        if not reply:
            return None

        ret = self.expect('CMD>')
        self.verbose and print('cmd ret: chars %u' % (len(ret), ))
        return ret

    def match_line(self, a_re, res):
        # print(len(self.e.before), len(self.e.after), len(res))
        for l in res.split('\n'):
            l = l.strip()
            m = re.match(a_re, l)
            if m:
                self.verbose and print("match: %s" % l)
                return m
        else:
            if self.verbose:
                print("Failed lines")
                for l in res.split('\n'):
                    print("  %s" % l.strip())
            raise NoSuchLine("Failed to match re: %s" % a_re)

    def assert_ver(self):
        """Verify the expected app is running"""
        # FIXME: veirfy we are in the bitbang app
        res = self.cmd('?')
        # print(len(self.e.before), len(self.e.after), len(res))
        app = self.match_line(r"open-tl866 \((.*)\)", res).group(1)
        assert self.APP is None or app == self.APP
        self.verbose and print("App type OK")

    def bootloader(self):
        '''reset to bootloader'''
        self.cmd('b', reply=False)
