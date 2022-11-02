'''
CMD> ?
open-tl866 (bitbang)
VPP
E val      VPP: enable or disable
           1 = enable, 0 = disable
V val      VPP: set voltage enum
           val in range [0,7]
p val      VPP: set active pins
           val must be 10 hex digits
           LSB is ZIF pin 1
VDD
e val      VDD: enable or disable
           1 = enable, 0 = disable
v val      VDD: set voltage enum
           val in range [0,7]
d val      VDD: set active pins
           val must be 10 hex digits
           LSB is ZIF pin 1
GND
g val      GND: set active pins (GND_WRITE)
           val must be 10 hex digits
           LSB is ZIF pin 1
           NOTE: VDD must be enabled for GND to work
I/O
t val      I/O: set ZIF tristate setting
           1 = pin is driven, 0 = High Z
T          I/O: get ZIF tristate setting
           1 = pin is driven, 0 = High Z
z val      I/O: set ZIF pins (ZIF_WRITE)
           val must be 10 hex digits
           LSB is ZIF pin 1
Z          I/O: get ZIF pins (ZIF_READ)
           LSB is ZIF pin 1
Misc
L val      LED on/off
           1 = on, 0 = off
m z val    Set pullup/pulldown
s          Print misc status
i          Re-initialize
b          Reset to bootloader
'''

from otl866 import aclient
from otl866.aclient import VPPS, VDDS


class Bitbang(aclient.AClient):
    APP = "bitbang"
    '''
    NOTE
    C code stores LSB first
    Python formats as MSB first
    '''
    def __init__(self, *args, **kwargs):
        self.cache_check = True
        self.clear_cache()
        aclient.AClient.__init__(self, *args, **kwargs)

    def clear_cache(self):
        self.vdd_en_cache = None
        self.vpp_en_cache = None
        self.vpp_pins_cache = None
        self.vdd_pins_cache = None
        self.gnd_pins_cache = None
        self.vpp_volt_cache = None
        self.io_tri_cache = None
        self.io_w_cache = None
        self.pupd_cache = None

    '''
    VPP
    '''

    def vpp_en(self, enable=True):
        '''VPP: enable and/or disable'''
        enable = int(bool(enable))
        self.cmd('E', enable)
        self.vpp_en_cache = enable

    def vpp_volt(self, val):
        '''VPP: set voltage enum'''
        assert val in VPPS
        self.cmd('V', val)
        self.vpp_volt_cache = val

    def vpp_pins(self, val):
        '''VPP: set active pins'''
        self.assert_zif(val)
        self.cmd('p', self.zif_str(val))
        self.vpp_pins_cache = val

    '''
    VDD
    '''

    def vdd_en(self, enable=True):
        '''VDD: enable and/or disable'''
        enable = int(bool(enable))
        self.cmd('e', enable)
        self.vdd_en_cache = enable

    def vdd_volt(self, val):
        '''VDD: set voltage enum'''
        assert val in VDDS
        self.cmd('v', val)
        self.vdd_volt_cache = val

    def vdd_pins(self, val):
        '''VDD: set active pins'''
        self.assert_zif(val)
        self.cmd('d', self.zif_str(val))
        self.vdd_pins_cache = val

    '''
    GND
    '''

    def gnd_pins(self, val):
        '''VDD: set active pins'''
        self.assert_zif(val)
        self.cmd('g', self.zif_str(val))
        self.gnd_pins_cache = val

    '''
    I/O
    '''

    def io_tri(self, val=0xFFFFFFFFFF):
        '''
        write ZIF tristate setting
        Bit set => tristate
        '''
        self.assert_zif(val)
        self.cmd('t', self.zif_str(val))
        self.io_tri_cache = val

    def io_trir(self):
        '''read ZIF tristate setting'''
        ret = self.result_zif(self.cmd('T'))
        assert not self.cache_check or self.io_tri_cache is None or ret == self.io_tri_cache
        return ret

    def io_w(self, val):
        '''write ZIF pins'''
        self.assert_zif(val)
        self.cmd('z', self.zif_str(val))
        self.io_w_cache = val

    def io_r(self):
        '''read ZIF pins'''
        ret = self.result_zif(self.cmd('Z'))
        if self.cache_check and self.io_w_cache is not None and self.io_tri_cache is not None:
            mask = 0xFFFFFFFFFF ^ self.io_tri_cache
            assert (ret & mask) == (self.io_w_cache & mask)
        return ret

    '''
    Misc
    '''

    def init(self):
        '''Re-initialize all internal state'''
        self.cmd('i')
        self.clear_cache()

    def pupd(self, val):
        '''pullup/pulldown'''
        self.cmd('m', int(bool(val)))
        self.pupd_cache = val

    def status_str(self):
        '''Run status command'''
        # Result nVPP_EN:1 nVDD_EN:1 LED:0 PUPD:Z1V1
        return self.cmd('s').strip()[12:]

    def print_debug(self):
        def qmark(val, fmt=None):
            if val is None:
                return "?"
            elif fmt:
                return fmt % (val, )
            else:
                return str(val)

        # Result nVPP_EN:1 nVDD_EN:1 LED:0 PUPD:Z1V1
        print("Cache")
        print("  nVPP_EN:%s nVDD_EN:%s LED:%s PUPD:%s" %
              (qmark(self.vpp_en_cache), qmark(
                  self.vdd_en_cache), "?", qmark(self.pupd_cache)))
        print("  vpp_pins:", qmark(self.vpp_pins_cache, "0x%010X"))
        print("  vdd_pins:", qmark(self.vdd_pins_cache, "0x%010X"))
        print("  gnd_pins:", qmark(self.gnd_pins_cache, "0x%010X"))
        print("  io_tri:  ", qmark(self.io_tri_cache, "0x%010X"))
        print("  io_w:    ", qmark(self.io_w_cache, "0x%010X"))
        print("  vpp_volt:", qmark(self.vpp_volt_cache))
        print("status:", self.status_str())
        print("io_r: 0x%010X" % self.io_r())
        print("io_trir: 0x%010X" % self.io_trir())


"""
Higher level API
Less efficient but easier to use
"""


class EzBang:
    def __init__(self, ez=None, zero=True, cache=True, verbose=False):
        if ez is None:
            ez = Bitbang(verbose=verbose)
        self.ez = ez
        # Cache values are in ez, but whether to use them is enforced here
        self.cache = cache

        if zero:
            self.vdd_en(False)
            self.vpp_en(False)
            self.io_tri(0xFFFFFFFFFF)
            self.io_w(0)
            self.gnd_pins(0)

    def vdd_en(self, enable=True):
        self.ez.vdd_en(enable)
        self.vdd_en_cache = enable

    def vpp_en(self, enable=True):
        self.ez.vpp_en(enable)
        self.vpp_en_cache = enable

    def mask_pin(self, val, pin, isset):
        assert 0 <= pin <= 39
        mask = 1 << pin
        if isset:
            return val | mask
        else:
            return val & (0xFFFFFFFFFF ^ mask)

    def io_tri(self, val=0xFFFFFFFFFF):
        if self.cache and val == self.ez.io_tri_cache:
            return
        self.ez.io_tri(val)

    def io_tri_pin(self, pin, val):
        self.io_tri(self.mask_pin(self.ez.io_tri_cache, pin, val))

    def io_w(self, val):
        if self.cache and val == self.ez.io_w_cache:
            return
        self.ez.io_w(val)

    def io_w_pin(self, pin, val):
        self.io_w(self.mask_pin(self.ez.io_w_cache, pin, val))

    def io_r(self):
        return self.ez.io_r()

    def vdd_volt(self, val):
        self.ez.vdd_volt(val=val)

    def vdd_pins(self, val):
        if self.cache and val == self.ez.vdd_pins_cache:
            return
        self.ez.vdd_pins(val)

    def vpp_pins(self, val):
        if self.cache and val == self.ez.vpp_pins_cache:
            return
        self.ez.vpp_pins(val)

    def vpp_pin(self, pin, val=True):
        self.vpp_pins(self.mask_pin(self.ez.vpp_pins_cache, pin, val))

    def vpp_volt(self, val):
        self.ez.vpp_volt(val=val)

    def gnd_pins(self, val):
        if self.cache and val == self.ez.gnd_pins_cache:
            return
        self.ez.gnd_pins(val)

    def gnd_pin(self, pin, val=True):
        self.gnd_pins(self.mask_pin(self.ez.gnd_pins_cache, pin, val))

    def print_debug(self):
        # Result nVPP_EN:1 nVDD_EN:1 LED:0 PUPD:Z1V1
        self.ez.print_debug()
        print("ezbang cache")
        print("  vdd_en:", self.vdd_en_cache)
        print("  vpp_en:", self.vpp_en_cache)
        print("  io_tri_cache:", self.io_tri_cache)
        print("  io_w:", self.io_w_cache)
        print("  gnd_pins:", self.gnd_pins_cache)
