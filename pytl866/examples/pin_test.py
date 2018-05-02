#!/usr/bin/env python3

import pytl866
import sys

if len(sys.argv) < 2:
    print("Usage: pin_test.py [port]")
    exit(-1)


with pytl866.Tl866Context(sys.argv[1]) as tl:
    print(tl.mk_and_send_cmd("eo", 1))
    print(tl.mk_and_send_cmd("zd", 0))
    masks = [i << j for j in range(0, 40, 8) for i in range(256)]

    for m in masks:
        tl.mk_and_send_cmd("zw", m)
        print("Write: ", format(m, "010X"))
        print("Read: ", format(tl.mk_and_send_cmd("zr", 0), "010X"))
