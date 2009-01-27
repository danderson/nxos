import sys
import os.path
from nxt.samba import SambaBrick, SambaOpenError
import nxt.flash

USAGE = """Syntax: %s <firmware image file>""" % sys.argv[0]

def parse_args():
    if len(sys.argv) != 2:
        print USAGE
        sys.exit(1)

    fw_file = sys.argv[1]
    if not os.path.isfile(fw_file):
        print "Error: %s is not a file." % fw_file
        sys.exit(1)
    fd = open(fw_file, 'rb')
    fw = fd.read()
    fd.close()
    if len(fw) > (256*1024):
        print "Error: The firmware is too big to fit in ROM."
        print "Maximum: %d bytes, Actual: %d bytes" % (256*1024, len(fw))
        sys.exit(1)

    return fw

def main():
    fw = parse_args()
    s = SambaBrick()

    try:
        print "Looking for the NXT in SAM-BA mode..."
        s.open(timeout=5)
        print "Brick found!"
    except SambaOpenError, e:
        print 'Error: %s.' % e.message
        return 1

    print "Flashing firmware..."
    f = nxt.flash.FlashController(s)
    f.flash(fw)
    print "Flashing complete, jumping to 0x100000..."
    s.jump(0x100000)
    print "Firmware started."
    s.close()
    return 0
