#!/usr/bin/env python3

from otl866 import at89, util, aclient
import unittest
import os


class TestCase(unittest.TestCase):
    def setUp(self):
        """Call before every test case."""
        port = util.default_port()
        self.verbose = os.getenv("VERBOSE", "N") == "Y"
        self.tl = at89.AT89(port, verbose=self.verbose)
        self.tl.led(1)

    def tearDown(self):
        """Call after every test case."""
        self.tl.led(0)

    def test_sig(self):
        self.tl.sig()

    """
    def test_blank(self):
        # WARNING: takes a long time, maybe 20 sec
        self.tl.blank()
    """

    def test_reset_vdd(self):
        self.tl.reset_vdd()

    def test_read(self):
        buff = self.tl.read(0, 16)
        assert len(buff) == 16, len(buff)

    def test_read_sf(self):
        buff = self.tl.read_sf(0, 16)
        assert len(buff) == 16, len(buff)

    def test_erase(self):
        self.tl.erase()

    def test_lock(self):
        self.tl.lock(2)
        #self.tl.lock(3)
        #self.tl.lock(4)


if __name__ == "__main__":
    unittest.main()  # run all tests
