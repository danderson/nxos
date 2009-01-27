#!/usr/bin/env python

# I2C pin information beautifier
def beautify(data, size):
    print "SDA:",
    s = ""
    for i in xrange(size/2):
        s = "%s%s" % (s, data[2*i])
    print s

    print "SCL:",
    s = ""
    for i in xrange(size/2):
        s = "%s%s" % (s, data[2*i+1])
    print s

