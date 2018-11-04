import sys
import time
from intelhex import IntelHex

from otl866.bootloader import driver, firmware
from otl866.bitbang import Bitbang


def find_dev():
    devs = driver.list_devices()

    if len(devs) > 1:
        sys.stderr.write("more than one TL866 is connected\n")
        sys.exit(1)

    elif not devs:
        sys.stderr.write("no TL866 devices were found\n")
        sys.exit(1)

    else:
        return driver.BootloaderDriver(devs[0])


def cmd_identify(args):
    dev = find_dev()
    report = dev.report()

    if report.status == dev.STATUS_BOOTLOADER:
        sys.stdout.write("TL866%s Bootloader\n" %
                         ('A' if report.model == dev.MODEL_TL866A else 'CS', ))

    elif report.hardware_version == 255:
        sys.stdout.write("TL866%s Open Firmware v%d.%d\n" % (
            'A' if report.model == dev.MODEL_TL866A else 'CS',
            report.firmware_version_major,
            report.firmware_version_minor,
        ))

    else:
        sys.stdout.write("TL866%s Stock Firmware v%02d.%d.%d\n" % (
            'A' if report.model == dev.MODEL_TL866A else 'CS',
            report.hardware_version,
            report.firmware_version_major,
            report.firmware_version_minor,
        ))

    sys.stdout.write("Serial %s, Device %s\n" % (
        report.serial_number.decode('latin1'),
        report.device_code.decode('latin1'),
    ))


def otl866_reset(reset_tty):
    serial = Bitbang(reset_tty)
    serial.bootloader()

    # wait for the device to show up
    devs = None
    stop_time = time.time() + 5  # timeout 5s
    while not devs and time.time() < stop_time:
        time.sleep(0.100)
        devs = driver.list_devices()


def reset_to_bootloader(dev):
    sys.stdout.write("resetting to bootloader\n")
    dev.reset()

    report = dev.report()
    if report.status != dev.STATUS_BOOTLOADER:
        sys.stderr.write("device did not reset to booloader\n")
        sys.exit(2)


def read_fw(args, model_a):
    # load the firmware image
    stock = None
    image = None
    if args.stock:
        with open(args.firmware, 'rb') as source:
            stock = firmware.UpdateFile(source.read())
            image = firmware.Firmware(
                stock.a_firmware if model_a else stock.cs_firmware,
                encrypted=True,
            )
    else:
        with open(args.firmware, 'r') as source:
            hexfile = IntelHex(source)
            image = firmware.Firmware(
                hexfile.tobinstr(start=0x1800, end=0x1FBFF),
                encrypted=False,
            )
    return stock, image


def load_keys(args, stock, image, model_a):
    # load encryption and erase keys
    target_key = None
    target_erase = None
    if args.keys_from:
        with open(args.keys_from, 'rb') as source:
            keyfile = firmware.UpdateFile(source.read())
            keyfw = firmware.Firmware(
                keyfile.a_firmware if model_a else keyfile.cs_firmware,
                encrypted=True,
            )
            target_key = keyfw.key
            target_erase = keyfile.a_erase if model_a else keyfile.cs_erase
    elif stock:
        target_key = image.key
        target_erase = stock.a_erase if model_a else stock.cs_erase
    else:
        target_key = firmware.KEY_A if model_a else firmware.KEY_CS
        target_erase = firmware.ERASE_A if model_a else firmware.ERASE_CS
    return target_key, target_erase


def cmd_update(args):
    if args.reset_tty:
        sys.stdout.write("resetting to bootloader via serial\n")
        otl866_reset(args.reset_tty)

    dev = find_dev()

    report = dev.report()
    model_a = (report.model == dev.MODEL_TL866A)
    if report.status != dev.STATUS_BOOTLOADER:
        reset_to_bootloader(dev)

    stock, image = read_fw(args, model_a)
    target_key, target_erase = load_keys(args, stock, image, model_a)

    if not image.valid:
        raise RuntimeError("firmware image invalid")

    sys.stdout.write("erasing\n")
    dev.erase(target_erase)

    sys.stdout.write("programming\n")
    addr = 0x1800
    cryptbuf = image.encrypt(target_key)
    for off in range(0, len(cryptbuf), 80):
        dev.write(addr, 80, cryptbuf[off:off + 80])
        addr += 64

    report = dev.report()
    sys.stdout.write("done, result: %s\n" % (report.status, ))
    # On failure does it exit bootloader?
    if report.status != dev.STATUS_NORMAL:
        sys.stdout.write("MCU reset\n")
        dev.reset()


def build_argparse(parent):
    parser = parent.add_parser(
        'self',
        description="Manage the firmware of the TL866 itself",
    )

    subgroup = parser.add_subparsers()

    ident = subgroup.add_parser(
        'identify',
        description="Display the device firmware version and status.",
    )
    ident.set_defaults(func=cmd_identify)

    update = subgroup.add_parser(
        'update',
        description="Update the TL866 firmware",
    )

    update.add_argument(
        '--stock',
        '-s',
        help="Load a stock update.dat file instead of Intel Hex.",
        action='store_true',
    )

    update.add_argument(
        '--keys-from',
        help="Load keys from a (different) update.dat file.",
    )

    update.add_argument(
        '--reset-tty',
        help="Reset open firmware to bootloader via serial port.",
    )

    update.add_argument(
        'firmware',
        help="The firmware file to program.",
    )

    update.set_defaults(func=cmd_update)
