#!/usr/bin/env python3

from otl866 import bitbang, util
from timeit import default_timer as timer
import binascii

# 0 index ZIF28 pin to 1 index ZIF40 pin
ZIF_28TO40 = [1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, \
                27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40]

# 15 bit address => A14 max
# 27C128
A_LINES = [10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2, 26, 27]
D_LINES = [11, 12, 13, 15, 16, 17, 18, 19]
CE = 20
OE = 22
# removed for 27C64 => 27C256
# PGM = 27

SIZE = 2**len(A_LINES)


# 1-based index
def zif_to_int(ls):
    i = 0
    for bit in ls:
        i = i + 2**(bit - 1)
    return i


# 1-based index
def eprom_to_int(pins):
    zif = [ZIF_28TO40[p - 1] for p in pins]
    return zif_to_int(zif)


def addr_bits(adr):
    tmp = adr
    i = 0
    hi_lines = []
    adr_len = len(A_LINES)
    while i < adr_len:
        if tmp & 0x01:
            hi_lines.append(A_LINES[i])

        tmp = tmp >> 1
        i = i + 1

    return eprom_to_int(hi_lines)


def get_data(zif_val):
    '''
    Given socket state as integer mask, return the current data byte on data bus
    '''
    # LSB to MSB
    ret = 0
    for bit_idx in range(len(D_LINES)):
        zif_idx = ZIF_28TO40[D_LINES[bit_idx] - 1] - 1

        if zif_val & (1 << zif_idx):
            ret |= 1 << bit_idx

    # print("zif %010X => %02X" % (zif_val, ret))

    return ret


def idle():
    return eprom_to_int([CE, OE])


def print_zif(v):
    print(format(v, "010X"))


def offmask(pin):
    index = ZIF_28TO40[pin - 1]
    off = index // 8
    mask = 1 << (index % 8)
    return off, mask


def zif_clear(bits, pin):
    off, mask = offmask(pin)
    bits[off] &= 0xFF ^ mask


def eprom_r_init(tl):
    # All high impedance by default
    tristate = 0xFFFFFFFFFF
    for pin in A_LINES + [CE, OE]:
        index = ZIF_28TO40[pin - 1] - 1
        tristate ^= 1 << index
    tl.io_tri(tristate)

    # Set voltages
    tl.vpp_pins(0)  # none
    tl.gnd_pins(1 << (14 - 1))  # DIP28-14 => DIP40-14
    tl.vdd_pins(1 << (40 - 1))  # DIP28-28 => DIP40-40
    tl.vdd_volt(bitbang.VDD_51)
    tl.vpp_en()
    tl.vdd_en()


def eprom_r_next(tl, addr, size):
    res = bytearray()
    for i in range(addr, addr + size):
        tl.io_w(addr_bits(i) | eprom_to_int([CE, OE]))  # Set up addr
        tl.io_w(addr_bits(i) | eprom_to_int([OE]))  # CE low
        tl.io_w((addr_bits(i) & ~eprom_to_int([OE, CE])))  # OE low

        res += get_data(tl.io_r()).to_bytes(1, byteorder="little")
    return res


def run(port, addr, size, fn_out=None, verbose=False):
    tl = bitbang.Bitbang(port, verbose=verbose)
    tl.led(1)

    try:
        eprom_r_init(tl)

        if fn_out:
            f = open(fn_out, "wb")
            printout = False
        else:
            f = None
            printout = True

        start = timer()

        for this_addr in range(addr, size, 16):
            chunk = eprom_r_next(tl, this_addr, min(size - addr, 16))
            if f:
                f.write(chunk)
            if printout:
                util.hexdump(chunk, pos_offset=this_addr)
        end = timer()

        elapsed = end - start
        print("Read %u bytes in %0.1f seconds (%0.1f bytes/sec)" %
              (size, elapsed, size / elapsed))
    finally:
        tl.led(0)


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Read 27C256 EPROM')
    parser.add_argument('--port',
                        default=util.default_port(),
                        help='Device serial port')
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("addr", default="0", help="Address")
    parser.add_argument("size", default=str(SIZE), help="Size")
    args = parser.parse_args()

    addr = int(args.addr, 0)
    size = int(args.size, 0)

    run(args.port, addr, size, verbose=args.verbose)


if __name__ == "__main__":
    main()
