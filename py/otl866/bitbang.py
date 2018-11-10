'''
Exposes low level primitives and nothing more
'''

import binascii

from otl866 import aclient
from otl866.aclient import VPPS, VDDS


class NoSuchLine(Exception):
    pass


class Timeout(Exception):
    pass


class Bitbang(aclient.AClient):
    APP = "bitbang"
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

    def init(self):
        '''Re-initialize all internal state'''
        self.cmd('i')

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
