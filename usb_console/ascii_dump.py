#!/usr/bin/env python

# ASCII dump beautifier
def beautify(data, size):
    s = ""
    for i in xrange(size):
      s = "%s%s" % (s, chr(data[i]))
    print s

