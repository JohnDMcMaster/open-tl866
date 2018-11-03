#!/usr/bin/env python3

import pytl866
import sys

if len(sys.argv) < 2:
    print("Usage: pin_test.py [port]")
    exit(-1)


with pytl866.Tl866Context(sys.argv[1]) as tl:
    tl.cmd_echo_off()
    tl.cmd_zif_dir(0)
    masks = [i << j for j in range(0, 40, 8) for i in range(256)]

    for m in masks:
        tl.cmd_zif_write(m)
        print("Write: ", format(m, "010X"))
        print("Read: ", format(tl.cmd_zif_read(), "010X"))
