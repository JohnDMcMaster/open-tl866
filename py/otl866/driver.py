'''
Exposes low level primitives and nothing more
'''

import re
import serial
from time import sleep

VPPS = (VPP_98, VPP_126, VPP_140, VPP_166, VPP_144, VPP_171, VPP_185,
        VPP_212) = range(8)
# My measurements: 9.83, 12.57, 14.00, 16.68, 14.46, 17.17, 18.56, 21.2

VDDS = (VDD_30, VDD_35, VDD_46, VDD_51, VDD_43, VDD_48, VDD_60,
        VDD_65) = range(8)

# My measurements: 2.99, 3.50, 4.64, 5.15, 4.36, 4.86, 6.01, 6.52


class Tl866Driver():
    def __init__(self, device, ser_timeout=1.0, verbose=False):
        self.handle = serial.Serial(
            device, timeout=ser_timeout, baudrate=115200, writeTimeout=0)
        self.ser.flushInput()
        self.ser.flushOutput()
        self.verbose = verbose

    def expect(self, s, timeout=3.0):
        return self.e.expect(s, timeout=timeout)

    def cmd(self, cmd, *args):
        '''Send raw command and get string result'''
        cmd = str(cmd)
        if len(cmd) != 1:
            raise ValueError('Invalid cmd %s' % cmd)
        # So far no commands have more than one arg
        if len(args) > 1:
            raise ValueError('cmd must take no more than 1 arg')
        self.ser.write(cmd + ''.join([str(arg) for arg in args]) + "\r\n")
        self.ser.flush()

        self.expect('CMD>')
        if self.verbose:
            print('cmd %s: before %s' % (cmd, self.e.before.strip()))
        return self.e.before

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

    def vpp_pins(self, zifstr):
        '''VPP: set active pins'''
        assert len(zifstr) == 5
        self.cmd('p', zifstr)

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

    def vdd_pins(self, zifstr):
        '''VDD: set active pins'''
        assert len(zifstr) == 5
        self.cmd('d', zifstr)

    '''
    GND
    '''

    def gnd_pins(self, zifstr):
        '''VDD: set active pins'''
        assert len(zifstr) == 5
        self.cmd('g', zifstr)

    '''
    I/O
    '''

    def io_tri(self, zifstr):
        '''write ZIF tristate setting'''
        assert len(zifstr) == 5
        self.cmd('t', zifstr)

    def io_trir(self):
        '''read ZIF tristate setting'''
        self.cmd('T')
        assert 0, 'FIXME: read'

    def io_w(self, zifstr):
        '''write ZIF pins'''
        assert len(zifstr) == 5
        self.cmd('z', zifstr)

    def io_r(self):
        '''read ZIF pins'''
        self.cmd('Z')
        assert 0, 'FIXME: read'

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
        self.cmd('b')
