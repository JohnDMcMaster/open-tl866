#!/usr/bin/env python3

import argparse

import ot866.bootloader.cli


def main():
    parser = argparse.ArgumentParser(
        description="TL866 open firmware CLI tool", )

    subgroup = parser.add_subparsers()
    ot866.bootloader.cli.build_argparse(subgroup)

    args = parser.parse_args()
    args.func(args)
