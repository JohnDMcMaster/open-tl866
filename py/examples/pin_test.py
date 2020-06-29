#!/usr/bin/env python3

from otl866 import bitbang, util
import binascii


def run(port):
    tl = bitbang.Bitbang(port)

    tl.cmd_zif_dir(0)

    for pin in range(40):
        pin_off = pin // 8
        pin_mask = 1 << (pin % 8)

        buff_w = bytearray(5)
        buff_w[pin_off] = pin_mask

        tl.io_w(buff_w)
        print("Write: %s" % binascii.hexlify(buff_w))
        print("Read: %s" % binascii.hexlify(tl.io_r()))


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='Write pins and read back value')
    parser.add_argument('--port',
                        default=util.default_port(),
                        help='Device serial port')
    args = parser.parse_args()

    run(args.port)


if __name__ == "__main__":
    main()
