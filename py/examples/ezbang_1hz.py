"""
ezbang API example
Creates a 1 Hz square wave going from IO on pin 1 to ground on pin 0
Pin 0 is the upper left pin (closest to lever), pin 1 is the pin below it
"""

from otl866 import bitbang
import time

# Open tl866 via ezbang API
eb = bitbang.EzBang()
# Enable power transistor on pin 0, shorting it to ground
eb.gnd_pin(0, True)
# IOs are tristated by default
# Disable tristate to enable IO to drive pin
eb.io_tri_pin(1, False)

# Loop at 1 Hz
while True:
    # Drive IO to high logic level (3.3V)
    eb.io_w_pin(1, True)
    time.sleep(0.5)
    # Drive IO to low logic level (0V)
    eb.io_w_pin(1, False)
    time.sleep(0.5)

