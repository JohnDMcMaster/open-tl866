import glob
import platform
import sys


def default_port():
    '''Try to guess the serial port, if we can find a reasonable guess'''
    if platform.system() == "Linux":
        acms = glob.glob(
            '/dev/serial/by-id/usb-ProgHQ_Open-TL866_Programmer_*')
        if len(acms) == 0:
            return None
        return acms[0]
    else:
        return None


def hexdump(data,
            label=None,
            indent='',
            address_width=8,
            f=sys.stdout,
            pos_offset=0):
    def isprint(c):
        return c >= ' ' and c <= '~'

    if label:
        f.write(label + "\n")

    bytes_per_half_row = 8
    bytes_per_row = 16
    data = bytearray(data)
    data_len = len(data)

    def hexdump_half_row(start):
        left = max(data_len - start, 0)

        real_data = min(bytes_per_half_row, left)

        f.write(''.join('%02X ' % c for c in data[start:start + real_data]))
        f.write(''.join('   ' * (bytes_per_half_row - real_data)))
        f.write(' ')

        return start + bytes_per_half_row

    pos = 0
    while pos < data_len:
        row_start = pos
        f.write(indent)
        if address_width:
            f.write(('%%0%dX  ' % address_width) % (pos + pos_offset))
        pos = hexdump_half_row(pos)
        pos = hexdump_half_row(pos)
        f.write("|")
        # Char view
        left = data_len - row_start
        real_data = min(bytes_per_row, left)

        strline = data[row_start:row_start + real_data].decode(
            "ascii", "replace")
        f.write(''.join([c if isprint(c) else '.' for c in strline]))
        f.write((" " * (bytes_per_row - real_data)) + "|\n")
