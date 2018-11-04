import glob
import platform


def default_port():
    '''Try to guess the serial port, if we can find a reasonable guess'''
    if platform.system() == "Linux":
        acms = glob.glob('/dev/ttyACM*')
        if len(acms) == 0:
            return None
        return acms[0]
    else:
        return None
