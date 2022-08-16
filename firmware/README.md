# Introduction

Open-TL866 operates using *modes*. Each mode supports one or
more target chips. A mode is used by uploading the .hex file
for that mode to the TL866 (see
[parent README](../README.md)), and then running the
corresponding Python command line tool from the `py/otl866`
directory.

# Supporting a new mode

To support a new mode:

1.  Create a directory in `firmware/modes` for your new mode.

1.  Create a `main.c` file in your mode's directory. It is
    best to start by copying it from one of the other modes.

1.  Create a `<mode>.h` and `<mode>.c` file in `firmware`
    for your new mode.

1.  Define low-level primitives in your `<mode>.*` files, using
    the basic calls in `io.h`
    (see [Firmware API](#Firmware-API)).

1.  Define text commands in your `main.c` file to call the
    low-level primitives.

1.  Write a Python script `py/otl866/<mode>.py` to connect to
    your firmware and execute your commands. It is best to
    start by copying your script from one of the other modes.

1.  Test, test, test.

# Firmware API

For ZIF pin voltage supply capabilities, see
[pin_mapping.md](pin_mapping.md).

The most useful API is specified in `firmware/io.h`. See that
file for detailed documentation. Briefly:

| Function | Brief description |
| -------- | ----------------- |
| io_init  | Initializes/resets all settings. |
| vdd_val  | Sets voltage level for VDD. |
| set_vdd  | Sets the given ZIF pins to output VDD. |
| vdd_en   | Enable VDD output. |
| vdd_dis  | Disable VDD output. |
| vdd_state | Returns the enable/disable state of VDD output. |
| vpp_val  | Sets voltage level for VPP. |
| set_vpp  | Sets the given ZIF pins to output VPP. |
| vpp_en   | Enable VPP output. |
| vpp_dis  | Disable VPP output. |
| vdd_state | Returns the enable/disable state of VPP output. |
| set_gnd  | Sets the given ZIF pins to output GND, immediately connecting them. |
| dir_write | Sets all ZIF pin directions. |
| dir_read | Reads all ZIF pin directions. |
| zif_write | Sets all ZIF pin states whose directions are set for write. |
| zif_read | Reads all ZIF pin states. |
| pupd | Sets the state of the pull resistors. |
