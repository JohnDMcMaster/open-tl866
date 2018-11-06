#!/usr/bin/env python3

from otl866 import bitbang, util
from timeit import default_timer as timer
import binascii

DATA_BUS2ZIF = [1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, \
                27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40]

a_lines = [10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2]
d_lines = [11, 12, 13, 15, 16, 17, 18, 19]
ce = 20
oe = 22
pgm = 27


# 1-based index
def zif_to_int(ls):
    i = 0
    for bit in ls:
        i = i + 2**(bit - 1)
    return i


# 1-based index
def eprom_to_int(pins):
    zif = [DATA_BUS2ZIF[p - 1] for p in pins]
    return zif_to_int(zif)


def addr_bits(adr):
    tmp = adr
    i = 0
    hi_lines = []
    adr_len = len(a_lines)
    while i < adr_len:
        if tmp & 0x01:
            hi_lines.append(a_lines[i])

        tmp = tmp >> 1
        i = i + 1

    return eprom_to_int(hi_lines)


def get_data(read_val):
    data_val = 0
    data_bit_pos = 0

    while data_bit_pos < 8:
        zif_pos = DATA_BUS2ZIF[d_lines[data_bit_pos] - 1] - 1

        bit = 1 if (read_val & (1 << zif_pos)) != 0 else 0
        data_val = data_val | (bit << data_bit_pos)
        data_bit_pos = data_bit_pos + 1

    return data_val


def idle():
    return eprom_to_int([ce, oe])


def print_zif(v):
    print(format(v, "010X"))


def run(port, fn_out=None, verbose=False):
    size = 8192
    size = 16

    tl = bitbang.Bitbang(util.default_port(), verbose=verbose)

    # Default direction (all output).
    tl.io_tri(0)

    # Set voltages
    tl.vpp_pins(0)
    tl.gnd_pins(0x0000002000)
    tl.vdd_pins(1 << 39)
    tl.vdd_volt(3)

    # Set data lines as input.
    tl.io_tri(eprom_to_int(d_lines))
    res = bytearray()
    print("Initialization done. Starting read loop...")

    start = timer()
    for i in range(size):
        tl.io_w(addr_bits(i) | eprom_to_int([pgm, ce, oe]))  # Set up addr
        tl.io_w(addr_bits(i) | eprom_to_int([pgm, oe]))  # CE low
        tl.io_w((addr_bits(i) & ~eprom_to_int([oe, ce]))
                | eprom_to_int([pgm]))  # OE low

        res += get_data(tl.io_r()).to_bytes(1, byteorder="little")
    end = timer()

    if fn_out:
        with open(fn_out, "wb") as f:
            f.write(res)
    else:
        util.hexdump(res)

    elapsed = end - start
    print("Done. Read 8192 bytes in {} seconds ({} bytes/sec)".format(
        elapsed, size / elapsed))


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Read EPROM')
    parser.add_argument(
        '--port', default=util.default_port(), help='Device serial port')
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    run(args.port, verbose=args.verbose)


if __name__ == "__main__":
    main()
