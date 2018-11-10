#!/usr/bin/env python3

from otl866 import bitbang, util, aclient
import unittest
import os


class BitbangTestCase(unittest.TestCase):
    def setUp(self):
        """Call before every test case."""
        port = util.default_port()
        self.verbose = os.getenv("VERBOSE", "N") == "Y"
        self.tl = bitbang.Bitbang(port, verbose=self.verbose)
        self.tl.led(1)
        self.tl.init()

    def tearDown(self):
        """Call after every test case."""
        self.tl.led(0)
        self.tl.init()

    def test_io(self):
        """Set I/O out and read back that value"""
        # Tried tristating unused but the float around
        self.tl.io_tri(0)
        for pini in range(40):
            mask = 1 << pini
            self.tl.io_w(mask)
            readback = self.tl.io_r()
            self.verbose and print("%010X" % mask, "%010X" % readback)
            self.assertEqual(mask, readback)

    def test_vpp(self):
        """Set VPP out and verify its set via digital I/O"""
        self.tl.vpp_en()
        # Any value really
        self.tl.vpp_volt(aclient.VPP_98)
        for pini in aclient.VPP_PINS0:
            mask = 1 << pini
            self.tl.vpp_pins(mask)
            readback = self.tl.io_r()
            readbackm = readback & mask
            self.verbose and print("%010X" % mask, "%010X" % readbackm)
            self.assertEqual(mask, readbackm)

    def test_vdd(self):
        """Set VDD out and verify its set via digital I/O"""
        self.tl.vdd_en()
        # Any value really
        self.tl.vdd_volt(aclient.VDD_30)
        for pini in aclient.VDD_PINS0:
            mask = 1 << pini
            self.tl.vdd_pins(mask)
            readback = self.tl.io_r()
            readbackm = readback & mask
            self.verbose and print("%010X" % mask, "%010X" % readbackm)
            self.assertEqual(mask, readbackm)


if __name__ == "__main__":
    unittest.main()  # run all tests
