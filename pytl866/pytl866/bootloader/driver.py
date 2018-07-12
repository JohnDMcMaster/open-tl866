from collections import namedtuple
import struct
import time
import usb.core
import usb.util


USB_VENDOR = 0x04D8
USB_PRODUCT = 0xE11C


def list_devices():
    return [
        BootloaderDriver(d)
        for d in usb.core.find(
            idVendor=USB_VENDOR,
            idProduct=USB_PRODUCT,
            find_all=True,
        )
    ]


class BootloaderDriver():
    CMD_REPORT = 0x00
    CMD_RESET = 0xFF
    CMD_WRITE = 0xAA
    CMD_ERASE = 0xCC

    STATUS_NORMAL = 1
    STATUS_BOOTLOADER = 2

    MODEL_TL866A = 1
    MODEL_TL866CS = 2

    REPORT_FORMAT = struct.Struct('< xBxxBBB 8s 24s B 4x')
    Report = namedtuple('Report', [
        'status',
        'firmware_version_minor',
        'firmware_version_major',
        'model',
        'device_code',
        'serial_number',
        'hardware_version',
    ])

    def __init__(self, device):
        self.device = device

    def close(self):
        usb.util.dispose_resources(self.device)

    def _recv(self, size, timeout=None):
        return self.device.read(usb.util.ENDPOINT_IN | 1, size, timeout)

    def _send(self, buf, timeout=None):
        self.device.write(usb.util.ENDPOINT_OUT | 1, buf, timeout)

    def reset(self):
        self._send(struct.pack('< B 3x', self.CMD_RESET))
        self.close()

        dev = None
        stop_time = time.time() + 5  # timeout = 5s
        while dev is None and time.time() < stop_time:
            time.sleep(0.100)  # interval = 100ms
            dev = usb.core.find(
                port_numbers=self.device.port_numbers,
                custom_match=lambda d: d.address != self.device.address
            )

        if dev is None:
            raise RuntimeError("device did not reconnect after reset")

        if dev.idVendor != USB_VENDOR or dev.idProduct != USB_PRODUCT:
            raise RuntimeError("wrong device reconnected after reset")

        self.device = dev

    def report(self):
        self._send(struct.pack('< B 4x', self.CMD_REPORT))
        buf = bytes(self._recv(self.REPORT_FORMAT.size))

        # the bootloader only returns 39 of the 44 bytes
        # and firmware versions <03.2.85 only return 40
        # pad out with zeroes to make struct happy
        if len(buf) < self.REPORT_FORMAT.size:
            buf += bytes([0] * (self.REPORT_FORMAT.size - len(buf)))

        return self.Report._make(self.REPORT_FORMAT.unpack(buf))

    def erase(self, key):
        self._send(struct.pack('< B 6x B 12x', self.CMD_ERASE, key))
        ret = self._recv(32, timeout=500000)
        if ret[0] != self.CMD_ERASE:
            raise RuntimeError("invalid response from erase command")

    def write(self, address, length, data, safe=True):
        if address < 0 or address >= 0x20000:
            raise ValueError('address must be within [0, 0x20000)')

        if length < 1 or length > 0xFFFF:
            raise ValueError('length must be within [1, 0xFFFF]')

        if safe and address < 0x01800:
            raise ValueError('refusing to overwrite bootloader in safe mode')

        # (length / 80 * 64) compensates for decryption shrinkage
        if safe and address + (length / 80 * 64) > 0x1FC00:
            raise ValueError('refusing to overwrite data block in safe mode')

        self._send(
            # struct doesn't support 3-byte fields, so pack it by hand...
            bytes([
                self.CMD_WRITE & 0xFF, (self.CMD_WRITE >> 8) & 0xFF,
                length & 0xFF, (length >> 8) & 0xFF,
                address & 0xFF, (address >> 8) & 0xFF,
                (address >> 16) & 0xFF,
            ]) + bytes(data)
        )
