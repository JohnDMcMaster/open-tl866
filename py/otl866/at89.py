"""
CMD> ?
open-tl866 (AT89)
r addr range   Read from target
w addr data    Write to target
R addr         Read sysflash from target
e              Erase target
l mode         Set lock bits to MODE
s              Print signature bytes
B              Blank check
T              Run some tests
h              Print help
v              Reset VPP
V              Print version(s)
b              reset to bootloader
addr, range in hex
"""

import binascii

from otl866 import aclient
import binascii

sig_i2s = {
    0x0151FF: "AT89C51 (19651)",
    0x1E51FF: "AT89C51 (19052)",
    0x1EFF1E: "AT89C51RC",
    0x1E52FF: "AT89C52 (19652)",
}


def sig_str(sig):
    sigi = (sig[0] << 16) | (sig[1] << 8) | sig[2]
    return sig_i2s.get(sigi, "unknown (0x%06X)" % sigi)


class AT89(aclient.AClient):
    APP = "at89"

    def read(self, addr, bytes):
        """
        CMD> r 0 16
        000 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
        """
        res = self.cmd('r', "%04X" % addr, "%04X" % bytes)
        hexstr = self.match_line(r"[0-9A-F]{3} (.*)", res).group(1)
        hexstr = hexstr.replace(" ", "")
        return binascii.unhexlify(hexstr)

    def write(self, addr, data):
        assert 0x00 <= data <= 0xFF
        self.cmd('w', "%04X" % addr, "%02X" % data)

    def read_sf(self, addr, bytes):
        res = self.cmd('R', "%04X" % addr, "%04X" % bytes)
        hexstr = self.match_line(r"[0-9A-F]{3} (.*)", res).group(1)
        hexstr = hexstr.replace(" ", "")
        return binascii.unhexlify(hexstr)

    def erase(self):
        self.cmd('e')

    def lock(self, mode):
        """
        WARNING: locking may disable signature check

        CMD> l 2
        Locking with mode 2... 2
        done.
        CMD> s
        (0x30) Manufacturer: 1E
        (0x31) Model:        51
        (0x32) VPP Voltage:  FF
        Name: AT89C51 (19052)
        CMD> l 3
        Locking with mode 3... 3
        done.
        CMD> s
        (0x30) Manufacturer: 00
        (0x31) Model:        00
        (0x32) VPP Voltage:  00
        Name: unknown
        CMD> e
        Erasing... done.
        CMD> s
        (0x30) Manufacturer: 1E
        (0x31) Model:        51
        (0x32) VPP Voltage:  FF
        Name: AT89C51 (19052)
        """
        assert mode in (2, 3, 4)
        self.cmd('l', mode)

    def sig(self):
        """
        CMD> s
        (0x30) Manufacturer: 1E
        (0x31) Model:        51
        (0x32) VPP Voltage:  FF
        Name: AT89C51 (19052)
        """
        res = self.cmd('s')

        mfg = self.match_line(r"\(0x30\).*(..)", res).group(1)
        model = self.match_line(r"\(0x31\).*(..)", res).group(1)
        vpp_volt = self.match_line(r"\(0x32\).*(..)", res).group(1)
        return int(mfg, 16), int(model, 16), int(vpp_volt, 16)

    def sig_en(self, en):
        self.cmd('S', int(bool(en)))

    def blank(self):
        # WARNING: takes a long time, maybe 20 sec
        """
        CMD> B
        Performing a blank-check... done
        000 set to byte 00
        Result: not blank
        """
        res = self.match_line(r"Result: (.*)", self.cmd('B')).group(1)
        return res == "blank"

    """
    def tests(self):
        self.cmd('T')
        assert 0, 'FIXME: parse out'
    """

    def reset_vdd(self):
        self.cmd('v')
