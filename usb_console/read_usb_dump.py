#!/usr/bin/env python

# A small utility to read data from the brick through the USB
# connection. This script expects the following communication
# protocol:
#
# A data size (U32, 4 bytes), immediately followed by the data itself,
# <data size> bytes.

import nxt
import struct
import sys
from nxt.lowlevel import get_device

NXOS_INTERFACE = 0

def main():
    print "Looking for NXT...",
    brick = get_device(0x0694, 0xFF00, timeout=60)
    if not brick:
        print "not found!"
        return False

    brick.open(NXOS_INTERFACE)
    print "ok."

    # Read data size
    print "Waiting for data size...",
    read_size = brick.read(4, 5000)
    if not read_size:
        print "timeout!"
        return False

    size = struct.unpack("<L", read_size)[0]
    print "ok. Expecting %d bytes." % size

    # Receive data
    print "Receiving data...",
    data = brick.read(size, 10000)
    if not data:
        print "timeout!"
        return False
    print "ok."

    data = [ ord(i) for i in data ]

    if len(sys.argv) > 1:
      if sys.argv[1] == 'i2c':
        from i2c_pin_data import beautify
        beautify(data, size)
      elif sys.argv[1] == 'ascii':
        from ascii_dump import beautify
        beautify(data, size)
      else:
        print [ str(i) for i in data ]

    return True


if __name__ == "__main__":
    main()
