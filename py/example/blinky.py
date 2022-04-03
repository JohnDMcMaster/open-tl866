#!/usr/bin/env python3

from otl866 import bitbang, util
import time


def run(port, verbose=False):
    tl = bitbang.Bitbang(port, verbose=verbose)
    while True:
        tl.led(1)
        time.sleep(0.5)
        tl.led(0)
        time.sleep(0.5)


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Blink the LED')
    parser.add_argument('--port',
                        default=util.default_port(),
                        help='Device serial port')
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    run(args.port, verbose=args.verbose)


if __name__ == "__main__":
    main()
