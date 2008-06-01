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
# Receives a unified diff on standard input. Scans all altered files
# for copyright notices, and warns about missing notices in relevant
# files, as well as copyrights that should be tweaked.

import time
import os
import sys
import re

COPYRIGHT_RE = re.compile(r'Copyright \(([cC])\) ([0-9\-,]+) the NxOS developers')

current_year = time.gmtime()[0]

def years_in_group(group):
    if '-' in group:
        min, max = (int(x) for x in group.split('-'))
        for year in xrange(min, max+1):
            yield year
    else:
        yield int(group)

def parse_years(years_str):
    years = set()
    for group in years_str.split(','):
        try:
            for year in years_in_group(group):
                if year in years:
                    print 'Year %d mentionned twice in %s' % (year, years)
                    return False
                years.add(year)
        except:
            print 'Could not parse copyright years %s' % years
            return False
    return years

def check_file(filepath):
    requires_copyright = False

    basename = os.path.basename(filepath)
    if '.' in basename:
        extension = basename.rsplit('.', 1)[1]
        if extension in ('h', 'c', 'S', 'py', 'sh'):
            requires_copyright = True

    contents = file(filepath).read()
    if requires_copyright and 'Copyright' not in contents:
        print 'No copyright notice found in %s' % filepath
        return False

    m = COPYRIGHT_RE.search(contents)
    if not m:
        return True

    if m.group(1) == 'C':
        print 'Copyright notice in %s uses "(C)"' % filepath
        print 'Correct notation is "(c)"'
        print m.group(0)
        return False

    years = parse_years(m.group(2))
    if not years:
        return False
    if current_year not in years:
        print '%d not present in copyright notice of %s:' % \
              (current_year, filepath)
        print m.group(0)
        return False
    return True

ok = True

for line in sys.stdin:
    if line.startswith('+++'):
        filepath = line.split()[1][2:]
        if not check_file(filepath):
            ok = False

if ok:
    sys.exit(0)
else:
    os.system('hg tip --template {desc} >commit.msg')
    print 'Commit message saved in commit.msg'
    sys.exit(1)
