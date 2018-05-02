from pytl866.driver import Tl866Driver

class Tl866Context():
    def __init__(self, device, baud_rate=115200):
        self.device = device
        self.baud_rate = baud_rate

    def __enter__(self):
        self.drv = Tl866Driver(self.device, self.baud_rate)
        return self.drv

    def __exit__(self):
        self.drv.handle.close()
