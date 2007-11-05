#!/usr/bin/env python
#
# "Compile" a font image into a C include file containing the binary
# representation of the font, ready for displaying.
#

import sys

# Fix the PYTHONPATH for scons builds.
if sys.platform == 'darwin':
    sys.path.append('/opt/local/lib/python2.4/site-packages')

try:
    from PIL import Image
except ImportError:
    print "ERROR: Python Imaging Library required for font generation."
    sys.exit(2)


class Font(object):
    def __init__(self, font_file):
        # Extract the character size from the filename
        size = font_file.split('.')[-2]
        y, x = size.split('x')
        try:
            self.charx = int(x)
            self.chary = int(y)
            if self.chary != 8:
                raise ValueError
        except ValueError:
            print "ERROR: unparseable font size %s" % size
            sys.exit(1)

        # Open the image and check that its dimensions make sense
        self.img = Image.open(font_file).convert('1')

        if ((self.img.size[0] % self.charx) != 0 or
            (self.img.size[1] % self.chary) != 0):
            print "ERROR: Font image for %s font has non-multiple dimensions" % size
            sys.exit(1)

        # Remember how many font char rows and cols there are
        self.rows = self.img.size[1] / self.chary
        self.cols = self.img.size[0] / self.charx

    def _get_block_coords(self, x, y):
        return (x * self.charx,
                y * self.chary,
                (x+1) * self.charx,
                (y+1) * self.chary)

    def _byteify(self, scanline):
        byte = 0
        for x in xrange(8):
            if not scanline[x]: # Invert the value to get the correct
                                # NXT encoding.
                byte |= 1 << x
        return byte

    def chars(self):
        for y in xrange(self.rows):
            for x in xrange(self.cols):
                block_coords = self._get_block_coords(x, y)
                font_block = list(self.img.crop(block_coords).getdata())
                scanlines = []
                for x in xrange(len(font_block)/self.charx):
                    scanlines.append(font_block[x*self.charx:(x+1)*self.charx])
                scanlines = zip(*scanlines)
                yield [self._byteify(l) for l in scanlines]

def main():
    if len(sys.argv) != 4:
        print "Usage: %s <font file> <template file> <output file>"
        sys.exit(1)

    font_file = sys.argv[1]
    template_file = sys.argv[2]
    output_file = sys.argv[3]

    font = Font(font_file)

    font_chars = []
    for scanlines in font.chars():
        font_chars.append(', '.join(['0x%02X' % x for x in scanlines]))
    font_data = '\n  '.join(['{ %s },' % c for c in font_chars])

    f = open(template_file)
    template = f.read()
    f.close()

    template = template.replace(
        '@@FONT_SIZE@@', '%dX%d' % (font.chary, font.charx))
    template = template.replace('@@FONT_WIDTH@@', '%d' % font.charx)
    template = template.replace('@@FONT_DATA@@', font_data)

    f = open(output_file, 'w')
    f.write(template)
    f.close()

if __name__ == '__main__':
    main()
