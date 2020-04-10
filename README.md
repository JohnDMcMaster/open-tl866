# Open-TL866
Open-TL866 is open-source firmware for the TL866-series of chip programmers.
This firmware replaces the proprietary firmware for programming EPROMs, MCUs, GALs, etc.
**Caution: This is alpha software. Use at your own risk.**

## Prerequisites

1. The XC8 compiler from Microchip is used to compile our C code.
   Currently version 1.x is required.
   [Download it from Microchip's site][xc8] and install it.
   When activating, select the Free version.

   **Note:** If you are installing to a 64-bit Ubuntu distribution,
   you must first get the 32-bit libc that isn't installed by default:
   `sudo apt-get install libc6:i386`. Then you can run Microchip's
   installer.

1. CMake is needed to generate the build configuration. On Linux you
   should install it from your distribution's package manager
   (e.g. `sudo apt-get install cmake`). For Windows, an installer is
   available [from the CMake website][cmake].

1. Certain dependencies are managed as git submodules.
   Install them with `git submodule update --init`.
   * [our copy](https://github.com/ProgHQ/m-stack)
     of [m-stack](http://www.signal11.us/oss/m-stack/)
   * [cmake-microchip](https://github.com/Elemecca/cmake-microchip)

[xc8]: http://www.microchip.com/development-tools/pic-and-dspic-downloads-archive
[cmake]: https://cmake.org/download/

## Building

1. `cd` to the `firmware` directory.
1. Generate the build configuration by running `cmake .`
1. Build the project by running `make`
1. Flashable images are in `dist/tl866-*.hex`

The build configuration is maintained with CMake in `CMakeLists.txt`.
Calling `cmake .` interprets the CMake configuration and produces a set
of makefiles in the source tree. If you prefer an out-of-source build,
just call `cmake` from the directory where you want the build output and
pass it the path to the `firmware` directory.

There are multiple variants of the firmware with different
functionality, which are currently called "modes". Each mode produces a
separate firmware image under `firmware/dist`. Each mode has a
corresponding target in the makefiles, so you can build just one mode
with e.g. `make tl866-bitbang`. The currently implemented modes are:

* `tl866-bitbang`
* `tl866-at89`

## Device Drivers and Configuration

On Linux systems, the udev rules file `contrib/96-opentl866.rules`
should be copied into `/etc/udev/rules.d/`. Doing so will allow non-root
users who are members of the `plugdev` group to access the devices. Once
the rules file is installed udev needs to be reloaded. On modern Linux
systems that use systemd that can be done by running:

```sudo systemctl restart systemd-udevd```

## Programming

The Python client library provides a command-line client for the stock
bootloader which can be used to flash any firmware to the TL866.
To install the CLI tool, run:

```cd py && python3 setup.py install```

And then TLDR: `otl866 self update firmware/dist/tl866-epromv.hex`

If that doesn't work, read on.


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
a reset into the bootloader: `otl866 self update firmware/dist/tl866-epromv.hex`
On Windows or if you multiple serial ports you may need to add
`--reset-tty`(ex: `--reset-tty COM6` on Windows)

If you flashed the open firmware onto the TL866 using _a separate_ ICSP
programmer the bootloader has been erased from your TL866. Do not reprogram a
TL866's firmware using the ICSP connector on the same TL866. In order to use
the update tool you'll need to flash the stock firmware via ICSP to restore
the bootloader. You can find ICSP-ready images of the stock firmware in
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
