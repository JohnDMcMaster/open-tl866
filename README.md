# Open-TL866
Open-TL866 is open-source firmware for the TL866-series of chip programmers.
This firmware replaces the proprietary firmware for programming EPROMs, MCUs, GALs, etc.
**Caution: This is alpha software. Use at your own risk.**

## Prerequisites
1. The `open-tl866` firmware requires the `XC8` compiler from Microchip and the `MPLAB X` IDE to generate build system files.
  * Download and install the [MPLAB X IDE](http://www.microchip.com/mplab/mplab-x-ide).
  * Download and Install the [XC8 Compiler](http://www.microchip.com/mplab/compilers).
  When activating, use the Free version.

2. We also need [our copy](https://github.com/ProgHQ/m-stack) of [m-stack](http://www.signal11.us/oss/m-stack/).
Run `git submodule update --init` at the root of this repository so that our `MPLAB X` project can find the USB stack source code.

## Building
1. Open the `firmware` project in MPLAB-X. This will generate the build system from the `.xml` files committed in the repository.
2. Click "Build Main Project" button (hammer icon) to compile the project.
Your output will be available under `dist/default/production/firmware.production.hex`.
  * Note that the proprietary firmware/bootloader expects a raw binary file that has been trimmed from both the head and tail (to prevent overwriting decryption tables and the bootloader).
  Microchip tools prefer emitting [Intel HEX](https://en.wikipedia.org/wiki/Intel_HEX) and so we use that format for convenience.

3. If changing any project options (right-click Project name and select "Properties") or adding new source files (right-click Source Files logical folder and click "New" or "Add Existing"), make sure to commit any changes to `nbproject/*.xml` files to the repository.
The [correct files](http://microchipdeveloper.com/faq:72) are already under version control.

## Programming

The Python client library provides a command-line client for the stock
bootloader which can be used to flash any firmware to the TL866.
To install the CLI tool, run:

```python pytl866/setup.py install```

Once the client is installed, you can use it to flash your firmware:

```tl866 self update dist/default/production/firmware.production.hex```

To go back to the stock firmware, just use the "Reflash firmware"
command in the official software.

## Running

The TL866 with the open firmware will identify itself as a serial port (USB CDC).
A Python library is provided which makes it easy to drive the bitbang mode.
