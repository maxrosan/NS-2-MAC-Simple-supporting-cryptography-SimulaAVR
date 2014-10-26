#!/usr/bin/env python
# Script to interact with simulavr by simulating a serial port.
import sys
import pysimulavr

SERIALBITS = 10 # 8N1 = 1 start, 8 data, 1 stop
BAUD = 19200

# Class to read serial data from AVR serial transmit pin.
class SerialRxPin(pysimulavr.PySimulationMember, pysimulavr.Pin):
    def __init__(self):
        pysimulavr.Pin.__init__(self)
        pysimulavr.PySimulationMember.__init__(self)
        self.sc = pysimulavr.SystemClock.Instance()
        self.delay = 10**9 / BAUD
        self.current = 0
        self.pos = -1
    def SetInState(self, pin):
        pysimulavr.Pin.SetInState(self, pin)
        self.state = pin.outState
        if self.pos < 0 and pin.outState == pin.LOW:
            self.pos = 0
            self.sc.Add(self)
    def DoStep(self, trueHwStep):
        ishigh = self.state == self.HIGH
        self.current |= ishigh << self.pos
        self.pos += 1
        if not self.pos:
            return int(self.delay * 1.5)
        if self.pos >= SERIALBITS:
            self.handleChar(chr((self.current >> 1) & 0xff))
            self.pos = -1
            self.current = 0
            return -1
        return self.delay
    def handleChar(self, c):
        sys.stdout.write(c)
        sys.stdout.flush()

# Class to send serial data to AVR serial receive pin.
class SerialTxPin(pysimulavr.PySimulationMember, pysimulavr.Pin):
    def __init__(self):
        pysimulavr.Pin.__init__(self)
        pysimulavr.PySimulationMember.__init__(self)
        self.SetPin('H')
        self.sc = pysimulavr.SystemClock.Instance()
        self.delay = 10**9 / BAUD
        self.current = 0
        self.pos = 0
        self.queue = ""
    def DoStep(self, trueHwStep):
        if not self.pos:
            if not self.queue:
                return -1
            self.current = (ord(self.queue[0]) << 1) | 0x200
            self.queue = self.queue[1:]
        newstate = 'L'
        if self.current & (1 << self.pos):
            newstate = 'H'
        self.SetPin(newstate)
        self.pos += 1
        if self.pos >= SERIALBITS:
            self.pos = 0
        return self.delay
    def pushChars(self, c):
        queueEmpty = not self.queue
        self.queue += c
        if queueEmpty:
            self.sc.Add(self)

def main():
    proc, elffile, speed = sys.argv[1:]

    # launch simulator
    sc = pysimulavr.SystemClock.Instance()
    dev = pysimulavr.AvrFactory.instance().makeDevice(proc)
    dev.Load(elffile)
    dev.SetClockFreq(10**9 / int(speed))
    sc.Add(dev)

    # Setup rx pin
    rxpin = SerialRxPin()
    net = pysimulavr.Net()
    net.Add(rxpin)
    net.Add(dev.GetPin("D1"))

    # Setup tx pin
    txpin = SerialTxPin()
    net2 = pysimulavr.Net()
    net2.Add(dev.GetPin("D0"))
    net2.Add(txpin)

    # Run loop
    sc.RunTimeRange(200000)
    txpin.pushChars("Sending data...")
    sc.RunTimeRange(400000000)

if __name__ == '__main__':
    main()
