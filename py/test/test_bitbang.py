#!/usr/bin/env python3

from otl866 import bitbang, util, aclient
import unittest
import os


class TestCase(unittest.TestCase):
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
        # Tried tristating unused but they float around
        self.tl.io_tri(0)
        for pini in range(40):
            mask = 1 << pini
            self.tl.io_w(mask)
            readback = self.tl.io_r()
            self.verbose and print("%010X" % mask, "%010X" % readback)
            self.assertEqual(mask, readback)

    def test_io_loop(self):
        """
        Loopback adapter required
        Short pins 1-2, 3-4, 5-6, etc
        """
        return

        print("")
        # low pins out, high pin in
        self.tl.io_tri(0xAAAAAAAAAA)
        for pini in range(0, 40, 2):
            for val in range(2):
                masko = 1 << pini
                maski = 1 << (pini + 1)
                self.tl.io_w(masko if val else 0)
                readback = self.tl.io_r()
                readbackm = readback & (masko | maski)
                expect = (masko | maski) if val else 0
                print("%010X" % expect, "%010X" % readbackm)
                self.assertEqual(expect, readbackm)

        print("")
        # high pins out, low pin in
        self.tl.io_tri(0x5555555555)
        for pini in range(0, 40, 2):
            for val in range(2):
                maski = 1 << pini
                masko = 1 << (pini + 1)
                self.tl.io_w(masko if val else 0)
                readback = self.tl.io_r()
                readbackm = readback & (masko | maski)
                expect = (masko | maski) if val else 0
                print("%010X" % expect, "%010X" % readbackm)
                self.assertEqual(expect, readbackm)

    def test_vpp(self):
        """Set VPP out and verify its set via digital I/O"""
        self.tl.vpp_en()
        # Any value really
        self.tl.vpp_volt(aclient.VPP_98)
        for pini in aclient.VPP_PINS0:
            # Pre-charge I/O low
            self.tl.io_tri(0)
            self.tl.io_w(0)
            self.tl.io_tri(0xFFFFFFFFFF)

            mask = 1 << pini
            self.tl.vpp_pins(mask)
            readback = self.tl.io_r()
            readbackm = readback & mask
            self.verbose and print("%010X" % mask, "%010X" % readbackm)
            self.assertEqual(mask, readbackm)

    def test_vdd(self):
        """Set VDD out and verify its set via digital I/O"""
        """
        Must tristate or will get contention
        Rely on some capacitive hold over to verify pin was actually toggled
        IO vs VDD test

        VDD_30, IO=Z: 3.21V
        VDD_30, IO=0: 2.36V
        VDD_30, IO=1: 3.40V

        VDD_65, IO=Z: 5.57V
        VDD_65, IO=0: 2.55V (and falling)
        VDD_65, IO=1: 5.49V
        """
        self.tl.vdd_en()
        # Any value really
        self.tl.vdd_volt(aclient.VDD_30)
        for pini in aclient.VDD_PINS0:
            # Pre-charge I/O low
            self.tl.io_tri(0)
            self.tl.io_w(0)
            self.tl.io_tri(0xFFFFFFFFFF)

            mask = 1 << pini
            self.tl.vdd_pins(mask)
            readback = self.tl.io_r()
            readbackm = readback & mask
            self.verbose and print("%010X" % mask, "%010X" % readbackm)
            self.assertEqual(mask, readbackm)


if __name__ == "__main__":
    unittest.main()  # run all tests
