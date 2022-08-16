from ctypes import *
import itertools
import time
from uuid import UUID


class GUID(Structure):
    _fields_ = [
        ("Data1", c_ulong),
        ("Data2", c_ushort),
        ("Data3", c_ushort),
        ("Data4", c_byte * 8),
    ]

    @staticmethod
    def from_uuid(value):
        return GUID(value.time_low, value.time_mid, value.time_hi_version,
                    (c_byte * 8).from_buffer_copy(value.bytes[8:]))


ERROR_NO_MORE_ITEMS = 259
ERROR_INSUFFICIENT_BUFFER = 122

INVALID_HANDLE_VALUE = -1

setupapi = windll.setupapi

HDEVINFO = c_void_p

DIGCF_DEFAULT = 0x00000001
DIGCF_PRESENT = 0x00000002
DIGCF_ALLCLASSES = 0x00000004
DIGCF_PROFILE = 0x00000008
DIGCF_DEVICEINTERFACE = 0x00000010

SetupDiGetClassDevs = setupapi.SetupDiGetClassDevsW
SetupDiGetClassDevs.restype = HDEVINFO
SetupDiGetClassDevs.argtypes = [
    # CONST GUID *ClassGuid
    POINTER(GUID),
    # PCWSTR Enumerator
    c_wchar_p,
    # HWND hwndParent
    c_void_p,
    # DWORD Flags
    c_ulong,
]


def call_SetupDiGetClassDevs(*args):
    handle = SetupDiGetClassDevs(*args)
    if handle == INVALID_HANDLE_VALUE:
        raise WinError()
    else:
        return handle


SetupDiDestroyDeviceInfoList = setupapi.SetupDiDestroyDeviceInfoList
SetupDiDestroyDeviceInfoList.restype = c_bool
SetupDiDestroyDeviceInfoList.argtypes = [
    # HDEVINFO DeviceInfoSet
    HDEVINFO,
]


def call_SetupDiDestroyDeviceInfoList(*args):
    if not SetupDiDestroyDeviceInfoList(*args):
        raise WinError()


class SP_DEVINFO_DATA(Structure):
    _fields_ = [
        ("cbSize", c_ulong),
        ("ClassGuid", GUID),
        ("DevInst", c_ulong),
        ("Reserved", POINTER(c_ulong)),
    ]


SetupDiEnumDeviceInfo = setupapi.SetupDiEnumDeviceInfo
SetupDiEnumDeviceInfo.restype = c_bool
SetupDiEnumDeviceInfo.argtypes = [
    # HDEVINFO DeviceInfoSet
    HDEVINFO,
    # DWORD MemberIndex
    c_ulong,
    # PSP_DEVINFO_DATA DeviceInfoData
    POINTER(SP_DEVINFO_DATA),
]


def iter_SetupDiEnumDeviceInfo(devinfoset):
    for idx in itertools.count():
        data = SP_DEVINFO_DATA()
        data.cbSize = sizeof(data)
        if SetupDiEnumDeviceInfo(devinfoset, idx, data):
            yield data
        else:
            err = GetLastError()
            if err == ERROR_NO_MORE_ITEMS:
                break
            else:
                raise WinError(err)


class SP_DEVICE_INTERFACE_DATA(Structure):
    _fields_ = [
        ("cbSize", c_ulong),
        ("InterfaceClassGuid", GUID),
        ("Flags", c_ulong),
        ("Reserved", POINTER(c_ulong)),
    ]


SetupDiEnumDeviceInterfaces = setupapi.SetupDiEnumDeviceInterfaces
SetupDiEnumDeviceInterfaces.restype = c_bool
SetupDiEnumDeviceInterfaces.argtypes = [
    # HDEVINFO DeviceInfoSet
    HDEVINFO,
    # PSP_DEVINFO_DATA DeviceInfoData
    POINTER(SP_DEVINFO_DATA),
    # CONST GUID *InterfaceClassGuid
    POINTER(GUID),
    # DWORD MemberIndex
    c_ulong,
    # PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    POINTER(SP_DEVICE_INTERFACE_DATA),
]


def iter_SetupDiEnumDeviceInterfaces(devinfoset, devinfo, classguid):
    for idx in itertools.count():
        data = SP_DEVICE_INTERFACE_DATA()
        data.cbSize = sizeof(data)
        ok = SetupDiEnumDeviceInterfaces(devinfoset, devinfo, classguid, idx,
                                         data)
        if ok:
            yield data
        else:
            err = GetLastError()
            if err == ERROR_NO_MORE_ITEMS:
                break
            else:
                raise WinError(err)


class SP_DEVICE_INTERFACE_DETAIL_DATA(Structure):
    _fields_ = [
        ("cbSize", c_ulong),
        ("DevicePath", c_wchar * 1),
    ]


SetupDiGetDeviceInterfaceDetail = setupapi.SetupDiGetDeviceInterfaceDetailW
SetupDiGetDeviceInterfaceDetail.restype = c_bool
SetupDiGetDeviceInterfaceDetail.argtypes = [
    # HDEVINFO DeviceInfoSet
    HDEVINFO,
    # PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    POINTER(SP_DEVICE_INTERFACE_DATA),
    # PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceIterfaceDetailData
    POINTER(SP_DEVICE_INTERFACE_DETAIL_DATA),
    # DWORD DeviceInterfaceDetailDataSize
    c_ulong,
    # PDWORD RequiredSize
    POINTER(c_ulong),
    # PSP_DEVINFO_DATA DeviceInfoData
    POINTER(SP_DEVINFO_DATA),
]


def call_SetupDiGetDeviceInterfaceDetail(devinfoset, iface):
    # get the required size
    size = c_ulong()
    ok = SetupDiGetDeviceInterfaceDetail(
        devinfoset,
        iface,
        None,  # DeviceInterfaceDetailData
        0,  # DeviceInterfaceDetailDataSize
        size,
        None  # DeviceInfoData
    )
    if not ok:
        err = GetLastError()
        if err != ERROR_INSUFFICIENT_BUFFER:
            raise WinError(err)

    # allocate a buffer
    data = SP_DEVICE_INTERFACE_DETAIL_DATA()
    data.cbSize = sizeof(data)
    resize(data, size.value)

    # call again to get the actual data
    ok = SetupDiGetDeviceInterfaceDetail(
        devinfoset,
        iface,
        data,
        size,
        None,  # RequiredSize
        None  # DeviceInfoData
    )
    if not ok:
        raise WinError()

    # extract and return the string
    return wstring_at(byref(data, type(data).DevicePath.offset))


kernel32 = windll.kernel32

GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
GENERIC_EXECUTE = 0x20000000
GENERIC_ALL = 0x10000000

FILE_SHARE_DELETE = 0x00000004
FILE_SHARE_READ = 0x00000001
FILE_SHARE_WRITE = 0x00000002

CREATE_ALWAYS = 2
CREATE_NEW = 1
OPEN_ALWAYS = 4
OPEN_EXISTING = 3
TRUNCATE_EXISTING = 5

FILE_ATTRIBUTE_NORMAL = 0x00000080

CreateFile = kernel32.CreateFileW
CreateFile.restype = c_void_p
CreateFile.argtypes = [
    # LPCWSTR lpFileName
    c_wchar_p,
    # DWORD dwDesiredAccess
    c_ulong,
    # DWORD dwShareMode
    c_ulong,
    # LPSECURITY_ATTRIBUTES lpSecurityAttributes
    c_void_p,
    # DWORD dwCreationDisposition
    c_ulong,
    # DWORD dwFlagsAndAttributes
    c_ulong,
    # HANDLE hTemplateFile
    c_void_p,
]


def call_CreateFile(*args):
    handle = CreateFile(*args)
    if handle == INVALID_HANDLE_VALUE:
        raise WinError()
    else:
        return handle


CloseHandle = kernel32.CloseHandle
CloseHandle.restype = c_bool
CloseHandle.argtypes = [
    # HANDLE hObject
    c_void_p,
]


def call_CloseHandle(*args):
    if not CloseHandle(*args):
        raise WinError()


DeviceIoControl = kernel32.DeviceIoControl
DeviceIoControl.restype = c_bool
DeviceIoControl.argtypes = [
    # HANDLE hDevice
    c_void_p,
    # DWORD dwIoControlCode
    c_ulong,
    # LPVOID lpInBuffer
    c_void_p,
    # DWORD nInBufferSize
    c_ulong,
    # LPVOID lpOutBuffer
    c_void_p,
    # DWORD nOutBufferSize
    c_ulong,
    # LPDWORD lpBytesReturned
    POINTER(c_ulong),
    # LPOVERLAPPED lpOverlapped
    c_void_p,
]


def call_DeviceIoControl(*args):
    if not DeviceIoControl(*args):
        raise WinError()


TL866_GUID = GUID.from_uuid(UUID('{85980D83-32B9-4ba1-8FDF-12A711B99CA2}'))

TL866_IOCTL_READ = 0x00222004
TL866_IOCTL_WRITE = 0x00222000


def list_devices():
    devices = list()

    devinfoset = call_SetupDiGetClassDevs(
        TL866_GUID,
        None,  # Enumerator
        None,  # hwndParent
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)

    try:
        interfaces = iter_SetupDiEnumDeviceInterfaces(devinfoset, None,
                                                      TL866_GUID)
        for iface in interfaces:
            path = call_SetupDiGetDeviceInterfaceDetail(devinfoset, iface)
            devices.append(WindowsDevice(path))
    finally:
        call_SetupDiDestroyDeviceInfoList(devinfoset)

    return devices


class WindowsDevice():
    def __init__(self, path):
        self._path = path
        self._file = None

    def open(self):
        if self._file is not None:
            raise RuntimeError("device is already open")

        self._file = call_CreateFile(
            self._path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            None,  # lpSecurityAttributes
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            None  # hTemplateFile
        )

    def close(self):
        if self._file is None:
            raise RuntimeError("device is already closed")

        call_CloseHandle(self._file)
        self._file = None

    def reopen(self):
        self.close()
        time.sleep(1)
        self.open()

    def read(self, size, timeout=None):
        if self._file is None:
            raise RuntimeError("device is not open")

        bytes_read = c_ulong()
        in_buffer = create_string_buffer(5)
        out_buffer = create_string_buffer(size)
        call_DeviceIoControl(
            self._file,
            TL866_IOCTL_READ,
            in_buffer,
            sizeof(in_buffer),
            out_buffer,
            sizeof(out_buffer),
            bytes_read,
            None  # lpOverlapped
        )

        return bytes(out_buffer.raw[:bytes_read.value])

    def write(self, data, timeout=None):
        if self._file is None:
            raise RuntimeError("device is not open")

        in_buffer = create_string_buffer(bytes(data))
        out_buffer = create_string_buffer(4096)
        bytes_written = c_ulong()
        call_DeviceIoControl(
            self._file,
            TL866_IOCTL_WRITE,
            in_buffer,
            sizeof(in_buffer),
            out_buffer,
            sizeof(out_buffer),
            bytes_written,
            None  # lpOverlapped
        )
