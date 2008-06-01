#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2008 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.
#
# Receives a unified diff on standard input. Looks for adds that
# insert trailing whitespace, and reports them along with
# file/line/function info. If any added trailing whitespace is found,
# exits in error. Also reports win32 EOLs.

import os
import sys

file = '<file unknown>'
function = '<function unknown>'
ok = True

for line in sys.stdin:
    if line.startswith('+++'):
        file = line.split()[1][2:]
        function = '<function unknown>'
        continue
    elif line.startswith('@@'):
        function = line.split('@@')[2].strip() or '<function unknown>'
        continue
    elif line.startswith('+'):
        line = line.rstrip('\n')
        if line.endswith(' ') or line.endswith('\t'):
            print 'Trailing whitespace at %s in %s:' % (file, function)
            print line
            ok = False
        if line.endswith('\r'):
            print 'Windows-style EOL at %s in %s:' % (file, function)
            print line
            ok = False

if ok:
    sys.exit(0)
else:
    os.system('hg tip --template {desc} >commit.msg')
    print 'Commit message saved in commit.msg'
    sys.exit(1)
