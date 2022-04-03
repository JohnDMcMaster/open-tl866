#!/usr/bin/env python3

from otl866 import aclient, util
from otl866 import bitbang
from otl866 import mem
import sys


def run(device, fn_out, verbose=False):
    ez = bitbang.EzBang(verbose=verbose)
    addr_pins = []
    data_pins = []
    vdd_pins = []
    gnd_pins = []
    hi_pins = []
    lo_pins = []
    vdd_volt = aclient.VDD_51

    if device == "63S281N":
        """
        63S281N
        """
        npins = 20
        addr_pins = [1, 2, 3, 4, 5, 17, 18, 19]
        data_pins = [6, 7, 8, 9, 11, 12, 13, 14]
        vdd_pins = [20]
        gnd_pins = [10]
        # These feed a 2NAND into a tristate buffer
        PIN_E1n = 15
        PIN_E2n = 16
        # Keep pins lo to remove tristate
        lo_pins = [PIN_E1n, PIN_E2n]
        output_pins = addr_pins + lo_pins
    elif device == "74S287":
        npins=16
        assert 0
    else:
        assert 0

    pack = mem.DIPPackage(ez, npins=npins,
                          output_pins=output_pins,
                          vdd_volt=vdd_volt,
                          vdd_pins=vdd_pins, gnd_pins=gnd_pins,
                          lo_pins=lo_pins, hi_pins=hi_pins,
                          verbose=verbose)
    pack.setup_pins()
    db = mem.DataBus(pack, addr_pins=addr_pins, data_pins=data_pins, verbose=verbose)
    rom = db.read_all()
    if fn_out:
        open(fn_out, "wb").write(rom)
    else:
        util.hexdump(rom)


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Issue a raw command and read response (try command "?"))')
    parser.add_argument('--port',
                        default=util.default_port(),
                        help='Device serial port')
    parser.add_argument("--device", required=True)
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("out", nargs="?", help="Write to file")
    args = parser.parse_args()

    run(device=args.device, fn_out=args.out, verbose=args.verbose)


if __name__ == "__main__":
    main()
