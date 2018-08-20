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

```cd pytl866 && python3 setup.py install```

### Resetting to the Bootloader

The first step in flashing any firmware is to get the TL866 to reboot
into its bootloader. How exactly you need to go about that depends on
which firmware your TL866 is currently running.

#### From the Stock Firmware

If your TL866 is running an older version of the stock firmware the
update tool can request a reset into the bootloader without any help. If
you have version 03.2.85 or newer of the stock firmware there's
currently a bug that prevents that from working, so you'll need to use
the hardware method below.

#### From the Open Firmware

If your TL866 is running the open firmware the update tool can trigger
a reset into the bootloader, but it needs to be told which serial port
to use to talk to the TL866. You can do that with the `--reset-tty`
option, for example `--reset-tty COM6` on Windows or
`--reset-tty /dev/ttyACM0` on Linux.

If you flashed the open firmware using an ICSP programmer the bootloader
has been erased from your TL866. In order to use the update tool you'll
need to flash the stock firmware via ICSP to restore the bootloader.
You can find ICSP-ready images of the stock firmware in
[Radioman's repository][stock-img].

[stock-img]: https://github.com/radiomanV/TL866/tree/67487e2cd60fa8f755e977c9fc656559452f5092/TL866_Updater/C%23/firmware

#### The Hardware Method

If you're having trouble resetting to the bootloader from within the
firmware currently installed on your TL866, you can force it to boot
into the bootloader by shorting pin RC1 of the microcontroller to Vcc
while you plug it in to the USB. The easiest points to short with a
piece of jumper wire are from pin 2 of J1 (the ICSP header) or the tab
of the adjacent voltage regulator to the side of R26 nearest J1.

If you find yourself using this method frequently it can be worthwhile
to solder a mini tactile switch between R26 and the side of R2 nearest
the edge of the board. That side of R2 is also Vcc and is much closer to
R26 than the voltage regulator is.


### Flashing the Open Firmware

To flash the open firmware, open a command prompt and navigate to the
folder where you built the firmware. Call the update tool, passing it
the path to the Intel Hex file. If your TL866 is already running the
stock firmware you'll also need the `--reset-tty` option (see above).


```tl866 self update --reset-tty COM6 dist/default/production/firmware.production.hex```


### Flashing the Stock Firmware

The update tool can also flash the stock firmware. To do so you'll need
the `update.dat` file from the official software. If you installed it
with the default settings that should be at `C:\MiniPro\update.dat`.
You'll need to pass the `--stock` option to tell the updater that you
want to flash the stock firmware, and you'll also need to use the
`--reset-tty` option if your TL866 is running the open firmware.


```tl866 self update --reset-tty COM6 --stock C:\MiniPro\update.dat```



## Running

The TL866 with the open firmware will identify itself as a serial port (USB CDC).
A Python library is provided which makes it easy to drive the bitbang mode.
