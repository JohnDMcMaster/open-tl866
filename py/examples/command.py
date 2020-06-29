#!/usr/bin/env python3

from otl866 import aclient, util
import sys


def run(port, cmd, args, reply=True, check=True, verbose=False):
    tl = aclient.AClient(port, verbose=verbose)
    res = tl.cmd(cmd, *args, reply=reply, check=check)
    res = res.replace('\r', '')
    # Each command echo
    res = res[res.find('\n') + 1:]
    print(res.strip())


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Blink the LED')
    parser.add_argument('--port',
                        default=util.default_port(),
                        help='Device serial port')
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("cmd")
    parser.add_argument("args", nargs='*')
    args = parser.parse_args()

    run(args.port, args.cmd, args.args, verbose=args.verbose)


if __name__ == "__main__":
    main()
