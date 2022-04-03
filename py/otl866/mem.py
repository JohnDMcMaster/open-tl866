"""
Bitbang memory utilities
(originally from pal866)
"""

from . import aclient
"""
DIP package at the top most part of the socket
Provides a HAL for setting pins by number and some basic tristate and power setup
"""


class DIPPackage:
    def __init__(
        self,
        # EzBang object
        ez,
        # Number of pins (say 40 for DIP40)
        npins,
        # As DIP pin numbers
        output_pins,
        # Set pins to logic 0 and 1 at reset
        lo_pins=[],
        hi_pins=[],
        # Optionally do power control
        pwr=True,
        # Define optional power control pins
        vdd_pins=[],
        gnd_pins=[],
        vdd_volt=aclient.VDD_51,
        verbose=False):
        self.verbose = verbose
        self.ez = ez

        self.pwr = pwr
        self.vdd_pins = vdd_pins
        self.gnd_pins = gnd_pins
        self.output_pins = output_pins
        self.vdd_volt = vdd_volt

        self.lo_pins = lo_pins
        self.hi_pins = hi_pins

        assert npins % 2 == 0
        self.npins = npins

        # Should calculate this dynamicly
        if npins == 16:
            to_zif_ = [1,  2,  3,  4,  5,  6,  7,  8, \
                            33, 34, 35, 36, 37, 38, 39, 40]
        elif npins == 20:
            to_zif_ = [1,  2,  3,  4,  5,  6,  7,  8,  9,  10, \
                            31, 32, 33, 34, 35, 36, 37, 38, 39, 40]
        elif npins == 32:
            to_zif_ = [1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, \
                            25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40]
        else:
            assert 0, "FIXME"

        self.pin_pack2zif = dict([(i + 1, x - 1)
                                  for i, x in enumerate(to_zif_)])
        assert self.pin_pack2zif[npins] == 39, self.pin_pack2zif[npins]

    def setup_pins(self):
        if self.pwr:
            self.verbose and print("reset")
            self.ez.vdd_en(False)
            self.ez.gnd_pins(0)
        self.ez.io_tri()

        if self.pwr:
            if self.gnd_pins:
                gnd = self.pins_to_zif(self.gnd_pins)
                self.verbose and print("  reset gnd: 0x%010X" % gnd)
                self.ez.gnd_pins(gnd)

            if self.vdd_pins:
                vcc = self.pins_to_zif(self.vdd_pins)
                self.verbose and print("  reset vdd: 0x%010X" % vcc)
                self.ez.vdd_pins(vcc)
                self.ez.vdd_volt(self.vdd_volt)

        # Default all pins low
        self.ez.io_w(0)
        if self.pwr:
            self.ez.vdd_en()
        # Initialize I/O
        self.set_io_dir()
        for pin in self.lo_pins:
            self.set_pin(pin, False)
        for pin in self.hi_pins:
            self.set_pin(pin, True)

    def set_pin(self, pin, val):
        self.ez.io_w_pin(self.pin_pack2zif[pin], val)

    def pins_to_zif(self, l):
        """Convert list of pins to ZIF mask"""
        return sum([1 << self.pin_pack2zif[x] for x in l])

    def set_io_dir(self):
        # All high impedance by default
        tristate = 0xFFFFFFFFFF
        for pin in self.output_pins:
            tzif = self.pin_pack2zif[pin]
            mask = 1 << tzif
            self.verbose and print("  Z %u => %u => 0x%010X" %
                                   (pin, tzif, mask))
            tristate ^= mask
        self.verbose and print("  reset tristate: 0x%010X" % tristate)
        self.ez.io_tri(tristate)

        # print("I_LINES: %s" % (self.addr_pins,))
        # print("O_LINES: %s" % (self.data_pins,))

    """
    def calculate_io(self, opins=[]):
        self.addr_pins = list(self.addr_pins_base)
        self.data_pins = list(self.data_pins_base)
        for pin in self.IO_LINES:
            if pin in opins:
                self.data_pins.append(pin)
            else:
                self.addr_pins.append(pin)
        self.addr_pins = sorted(self.addr_pins)
        self.data_pins = sorted(self.data_pins)
        self.verbose and print("i/o I: ", self.addr_pins)
        self.verbose and print("i/o O: ", self.data_pins)
    """


"""
Utility to treat a large number of pins as an address/data bus
Requires a Package object to do pin translation
LSB first
"""


class DataBus:
    def __init__(
        self,
        pack,
        # LSB first
        data_pins,
        addr_pins,
        verbose=False):
        self.verbose = verbose
        self.data_pins = data_pins
        self.addr_pins = addr_pins
        self.pack = pack
        self.verbose and print("Bus data: %s" % (self.data_pins, ))
        self.verbose and print("Bus addr: %s" % (self.addr_pins, ))

    def words(self):
        return 1 << len(self.addr_pins)

    def pin_data_mask(self, pin):
        """Return the data mask for given pin"""
        return 1 << self.data_pins.index(pin)

    def pin_addr_mask(self, pin):
        """Return the address mask for given pin"""
        return 1 << self.addr_pins.index(pin)

    def pins_for_addr(self, addr):
        """
        Return pins that should be set high
        """

        ret = []
        for biti, pin in enumerate(self.addr_pins):
            if addr & (1 << biti):
                ret.append(pin)

        return ret

    def ez2data(self, zif_val):
        '''
        Given socket state as integer mask, return the current data byte on data bus
        '''
        # LSB to MSB
        ret = 0
        for biti, pinp in enumerate(self.data_pins):
            # print("check d", zif_val, biti, pin20)
            if (1 << self.pack.pin_pack2zif[pinp]) & zif_val:
                ret |= 1 << biti

        # print("ez2data: 0x%010X => 0x%02X" % (zif_val, ret))
        return ret

    def addr(self, val):
        """Set addr on bus"""
        self.pack.ez.io_w(self.pack.pins_to_zif(self.pins_for_addr(val)))

    def read(self):
        """Read val from bus"""
        return self.ez2data(self.pack.ez.io_r())

    def read_all(self, verbose=None):
        if verbose is None:
            verbose = self.verbose
        ret = bytearray()
        verbose and print("Reading %u words" % self.words())
        for addr in range(self.words()):
            if addr % (self.words() // 100) == 0:
                verbose and print("%0.1f%%" % (addr / self.words() * 100.0, ))
            self.addr(addr)
            ret.append(self.read())
        return ret
