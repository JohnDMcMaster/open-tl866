#!/usr/bin/env python3

import argparse

import otl866.bootloader.cli


def main():
    parser = argparse.ArgumentParser(
        description="TL866 open firmware CLI tool", )

    subgroup = parser.add_subparsers()
    otl866.bootloader.cli.build_argparse(subgroup)

    args = parser.parse_args()

    if "func" in vars(args):
        args.func(args)
    else:
        parser.print_usage()
