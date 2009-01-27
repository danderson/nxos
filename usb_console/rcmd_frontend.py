#!/usr/bin/env python

# Copyright (c) 2008 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.

import atexit
import code
import nxt
import os
import readline
import struct
import sys
import time
from nxt.lowlevel import get_device

NXOS_INTERFACE = 0

class RcmdConsole(code.InteractiveConsole):

  def __init__(self, locals=None, filename="<console>",
               histfile=os.path.expanduser("~/.nxos-rcmd.hist")):
    code.InteractiveConsole.__init__(self)
    self.init_history(histfile)

  def init_history(self, histfile):
    readline.parse_and_bind("tab: complete")
    if hasattr(readline, "read_history_file"):
      try:
        readline.read_history_file(histfile)
      except IOError:
        pass
      atexit.register(self.save_history, histfile)

  def save_history(self, histfile):
    readline.write_history_file(histfile)


  def set_brick(self, brick):
    self.brick = brick

  def push(self, line):
    if line == 'quit' or line == 'exit':
      print "Use Ctrl-D (i.e. EOF) to exit"
      return False

    self.brick.write(line)
    if line == 'end':
      sys.exit(0)

    return False

def main():
    print "Looking for NXT...",
    brick = get_device(0x0694, 0xFF00, timeout=60)
    if not brick:
        print "not found!"
        return False

    brick.open(NXOS_INTERFACE)
    print "ok."

    prompt = RcmdConsole()
    prompt.set_brick(brick)
    prompt.interact('Remote robot command console.')

    print "Closing link...",
    brick.write('end')
    print "done."

if __name__ == "__main__":
    main()
