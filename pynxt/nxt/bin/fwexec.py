import sys
import os.path
from nxt.samba import SambaBrick, SambaOpenError

USAGE = """Syntax: %s <firmware image file> <RAM load address>""" % sys.argv[0]

def parse_args():
    if len(sys.argv) != 3:
        print USAGE
        sys.exit(1)

    fw_file = sys.argv[1]
    if not os.path.isfile(fw_file):
        print "Error: %s is not a file." % fw_file
        sys.exit(1)
    fd = open(fw_file, 'rb')
    fw = fd.read()
    fd.close()
    if len(fw) > (56*1024):
        print "Error: The firmware is too big to fit in RAM."
        print "Maximum: %d bytes, Actual: %d bytes" % (56*1024, len(fw))
        sys.exit(1)

    load_addr = sys.argv[2]
    if load_addr.startswith('0x'):
        load_addr = int(load_addr, 16)
    else:
        load_addr = int(load_addr, 10)

    return fw, load_addr

def main():
    fw, load_addr = parse_args()
    s = SambaBrick()

    try:
        print "Looking for the NXT in SAM-BA mode..."
        s.open(timeout=5)
        print "Brick found!"
    except SambaOpenError, e:
        print 'Error: %s.' % str(e)
        return 1

    print "Uploading firmware..."
    s.write_buffer(load_addr, fw)
    print "Upload complete, jumping to 0x%x..." % load_addr
    s.jump(load_addr)
    print "Firmware started."
    s.close()
    return 0
