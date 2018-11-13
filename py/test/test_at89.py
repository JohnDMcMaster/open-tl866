#!/usr/bin/env python3

from otl866 import at89, util
import unittest
import os
import time
import random


class TestCase(unittest.TestCase):
    def setUp(self):
        """Call before every test case."""
        print("")
        port = util.default_port()
        self.verbose = os.getenv("VERBOSE", "N") == "Y"
        self.tl = at89.AT89(port, verbose=self.verbose)
        self.tl.led(1)
        self.tl.sig_en(1)

    def tearDown(self):
        """Call after every test case."""
        self.tl.sig_en(1)
        self.tl.led(0)

    def test_sig(self):
        sig = self.tl.sig()
        print("Device: %s" % at89.sig_str(sig))

    """
    def test_blank(self):
        # WARNING: takes a long time, maybe 20 sec
        self.tl.blank()
    """

    def test_read(self):
        buff = self.tl.read(0, 16)
        assert len(buff) == 16, len(buff)

    def test_read_sf(self):
        buff = self.tl.read_sf(0, 16)
        assert len(buff) == 16, len(buff)

    def test_erase(self):
        self.tl.erase()

    def test_lock(self):
        self.tl.sig_en(1)

        self.tl.erase()
        sig = self.tl.sig()
        print("Device: %s" % at89.sig_str(sig))

        self.tl.erase()
        self.tl.lock(2)
        sig = self.tl.sig()
        print("Device: %s" % at89.sig_str(sig))

        self.tl.sig_en(0)

        self.tl.erase()
        self.tl.lock(3)
        sig = self.tl.sig()
        print("Device: %s" % at89.sig_str(sig))

        self.tl.erase()
        self.tl.lock(4)
        sig = self.tl.sig()
        print("Device: %s" % at89.sig_str(sig))

        self.tl.sig_en(1)

    def test_reset_torture(self):
        '''
        Used to troubleshoot issue where chip would sometimes
        come out of reset improperly and fail signature
        Root cause: rails were floated, so if you waited just right amount of time you'd glitch rail
        Solution: code now grounds all pins on shutdown
        '''

        if os.getenv('ALL_TESTS', 'N') != 'Y':
            return

        i = 0
        while True:
            i += 1
            print("Loop %u" % i)
            #tl.sig()
            self.tl.erase()
            time.sleep(0.001 * random.randint(0, 1200))


if __name__ == "__main__":
    unittest.main()  # run all tests
