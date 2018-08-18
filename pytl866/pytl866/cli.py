import argparse

import pytl866.bootloader.cli


def main():
    parser = argparse.ArgumentParser(
        description="TL866 open firmware CLI tool",
    )

    subgroup = parser.add_subparsers()
    pytl866.bootloader.cli.build_argparse(subgroup)

    args = parser.parse_args()
    args.func(args)
