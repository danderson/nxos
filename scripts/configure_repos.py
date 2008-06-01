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
# Sets up a freshly cloned repository with a few useful things.

import os
import sys
import ConfigParser

REPOS_URL='https://ssl.natulte.net/nxos/%s'

def main():
    if not os.path.isdir('.hg'):
        print 'Please run from the root of your repository.'
        return 1

    conf = ConfigParser.SafeConfigParser()
    conf.read('.hg/hgrc')

    # Add shorthands for the mainstream repo, as well as all crew
    # repos.
    if not conf.has_section('paths'):
        conf.add_section('paths')
    conf.set('paths', 'devel', REPOS_URL % 'devel')
    for crew in ('alexandre', 'dave', 'fleurda', 'jflesch', 'sam', 'sarah'):
        conf.set('paths', crew, REPOS_URL % 'crew/'+crew)

    # Add a pre-commit hook that checks for insertion of trailing
    # whitespace, and another to validate copyrights.
    if not conf.has_section('hooks'):
        conf.add_section('hooks')
    conf.set('hooks', 'pretxncommit.whitespace',
             'hg export tip | ./scripts/trailing_whitespace_report.py')
    conf.set('hooks', 'pretxncommit.copyright',
             'hg export tip | ./scripts/copyright_report.py')
    conf.set('hooks', 'pretxncommit.metadata', './scripts/metadata_report.sh')

    conf.write(open('.hg/hgrc', 'w'))

    return 0

if __name__ == '__main__':
    sys.exit(main())
