#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2008,2009 the NxOS developers
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
import subprocess
import sys
import re

COPYRIGHT_RE = re.compile(r'Copyright \(([cC])\) ([0-9\-,]+) the NxOS developers')

current_year = time.gmtime()[0]

def git(*args):
    command = ['git'] + list(args)
    return subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]

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

def check_file(file_path, file_sha):
    requires_copyright = False

    if '.' in file_path:
        extension = file_path.rsplit('.', 1)[1]
        if extension in ('h', 'c', 'S', 'py', 'sh'):
            requires_copyright = True

    contents = git('cat-file', 'blob', file_sha)

    if requires_copyright and 'Copyright' not in contents:
        print 'No copyright notice found in %s' % file_path
        return False

    m = COPYRIGHT_RE.search(contents)
    if not m:
        return True

    if m.group(1) == 'C':
        print 'Copyright notice in %s uses "(C)"' % file_path
        print 'Correct notation is "(c)"'
        print m.group(0)
        return False

    years = parse_years(m.group(2))
    if not years:
        return False
    if current_year not in years:
        print '%d not present in copyright notice of %s:' % \
              (current_year, file_path)
        print m.group(0)
        return False
    return True

def main():
    ok = True

    changed_files = git('diff-index', '--cached', '-z', '-M', 'HEAD')

    for changed in changed_files.splitlines():
        changed = changed.split('\0')

        file_sha, file_status = changed[0].split()[3:5]
        file_path = changed[-2]

        if file_status not in ('M', 'C', 'R', 'A'):
            continue

        if not check_file(file_path, file_sha):
            ok = False

    if ok:
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main())
