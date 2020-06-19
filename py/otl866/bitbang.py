'''
CMD> ?
open-tl866 (bitbang)
VPP
E val      VPP: enable and/or disable (VPP_DISABLE/VPP_ENABLE)
V val      VPP: set voltage enum (VPP_SET)
p val      VPP: set active pins (VPP_WRITE)
VDD
e val      VDD: enable and/or disable (VDD_DISABLE/VDD_ENABLE)
v val      VDD: set voltage enum (VDD_SET)
d val      VDD: set active pins (VDD_WRITE)
GND
g val      GND: set active pins (GND_WRITE)
I/O
t val      I/O: set ZIF tristate setting (ZIF_DIR)
T          I/O: get ZIF tristate setting (ZIF_DIR_READ)
z val      I/O: set ZIF pins (ZIF_WRITE)
Z          I/O: get ZIF pins (ZIF_READ)
Misc
l val      LED on/off (LED_ON/LED_OFF)
m z val    Set pullup/pulldown (MYSTERY_ON/MYSTERY_OFF}
s          Print misc status
i          Re-initialize
b          Reset to bootloader (RESET_BOOTLOADER)
'''

import binascii

from otl866 import aclient
from otl866.aclient import VPPS, VDDS


class Bitbang(aclient.AClient):
    APP = "bitbang"
    '''
    NOTE
    C code stores LSB first
    Python formats as MSB first
    '''
    '''
    VPP
    '''

    def vpp_en(self, enable=True):
        '''VPP: enable and/or disable'''
        self.cmd('E', int(bool(enable)))

    def vpp_volt(self, val):
        '''VPP: set voltage enum'''
        assert val in VPPS
        self.cmd('V', val)

    def vpp_pins(self, val):
        '''VPP: set active pins'''
        self.assert_zif(val)
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
        self.cmd('v', val)

    def vdd_pins(self, val):
        '''VDD: set active pins'''
        self.assert_zif(val)
        self.cmd('d', self.zif_str(val))

    '''
    GND
    '''

    def gnd_pins(self, val):
        '''VDD: set active pins'''
        self.assert_zif(val)
        self.cmd('g', self.zif_str(val))

    '''
    I/O
    '''

    def io_tri(self, val):
        '''
        write ZIF tristate setting
        Bit set => tristate
        '''
        self.assert_zif(val)
        self.cmd('t', self.zif_str(val))

    def io_trir(self):
        '''read ZIF tristate setting'''
        return self.result_zif(self.cmd('T'))

    def io_w(self, val):
        '''write ZIF pins'''
        self.assert_zif(val)
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

    def pupd(self, val):
        '''pullup/pulldown'''
        self.cmd('m', int(bool(val)))
