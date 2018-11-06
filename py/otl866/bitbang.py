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


class NoSuchLine(Exception):
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


class Bitbang:
    def __init__(self, device, verbose=False):
        self.verbose = verbose
        self.verbose and print("port: %s" % device)
        self.ser = ASerial(device, timeout=0, baudrate=115200, writeTimeout=0)
        self.ser.flushInput()
        self.e = pexpect.fdpexpect.fdspawn(self.ser.fileno(), encoding="ascii")
        self.assert_ver()

    def expect(self, s, timeout=0.5):
        tstart = time.time()
        buff = ""
        while time.time() - tstart < timeout:
            try:
                self.e.expect(s, timeout=timeout)
                buff += self.e.before
                return buff
            except pexpect.exceptions.EOF:
                buff += self.e.before
                continue
        else:
            raise Exception("timeout")

    def cmd(self, cmd, *args):
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
        # FIXME: veirfy we are in the bitbang app
        res = self.cmd('?')
        # print(len(self.e.before), len(self.e.after), len(res))
        app = self.match_line(r"open-tl866 \((.*)\)", res).group(1)
        assert app == "bitbang"
        self.verbose and print("App type OK")

    '''
    NOTE
    C code stores LSB first
    Python formats as MSB first
    '''

    def result_zif(self, res):
        '''
        Grab CLI ZIF output and return as single integer

        ZIF output is LSB first

         Z
        Result: 00 00 00 00 00
        CMD> 
        '''
        hexstr_raw = self.match_line(r"Result: (.*)", res).group(1)
        hexstr_lsb = hexstr_raw.replace(" ", "")
        ret = 0
        for wordi, word in enumerate(binascii.unhexlify(hexstr_lsb)):
            ret |= word << (wordi * 8)
        return ret

    def zif_str(self, val):
        '''
        Make ZIF CLI input from ZIF as a single integer
        '''
        ret = ""
        for _wordi in range(5):
            ret += "%02X" % (val & 0xFF, )
            val = val >> 8
        return ret

    '''
    VPP
    '''

    def vpp_en(self, enable=True):
        '''VPP: enable and/or disable'''
        self.cmd('E', int(bool(enable)))

    def vpp_volt(self, val):
        '''VPP: set voltage enum'''
        assert val in VPPS
        self.cmd('P', val)

    def vpp_pins(self, val):
        '''VPP: set active pins'''
        assert 0 <= val <= 0xFFFFFFFFFF
        self.cmd('p', self.zif_str(val))

    '''
    VDD
    '''

    def vdd_en(self, enable=True):
        '''VDD: enable and/or disable'''
        self.cmd('e', int(bool(enable)))

    def vdd_volt(self, val):
        '''VDD: set voltage enum'''
        assert val in VDDS
        self.cmd('D', val)

    def vdd_pins(self, val):
        '''VDD: set active pins'''
        assert 0 <= val <= 0xFFFFFFFFFF
        self.cmd('d', self.zif_str(val))

    '''
    GND
    '''

    def gnd_pins(self, val):
        '''VDD: set active pins'''
        assert 0 <= val <= 0xFFFFFFFFFF
        self.cmd('g', self.zif_str(val))

    '''
    I/O
    '''

    def io_tri(self, val):
        '''write ZIF tristate setting'''
        assert 0 <= val <= 0xFFFFFFFFFF, "%10X" % val
        self.cmd('t', self.zif_str(val))

    def io_trir(self):
        '''read ZIF tristate setting'''
        return self.result_zif(self.cmd('T'))

    def io_w(self, val):
        '''write ZIF pins'''
        assert 0 <= val <= 0xFFFFFFFFFF
        self.cmd('z', self.zif_str(val))

    def io_r(self):
        '''read ZIF pins'''
        return self.result_zif(self.cmd('Z'))

    '''
    Misc
    '''

    def led(self, val):
        '''Write LED on/off'''
        self.cmd('l', int(bool(val)))

    def ledr(self):
        '''Read LED state'''
        self.cmd('L')
        assert 0, 'FIXME: read'

    def pupd(self, val):
        '''pullup/pulldown'''
        self.cmd('m')

    def bootloader(self):
        '''reset to bootloader'''
        try:
            self.cmd('b')
        except pexpect.exceptions.EOF:
            pass
