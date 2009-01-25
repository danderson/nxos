#!/bin/bash
#
# Copyright (c) 2008,2009 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.
#
# Check the commit metadata for common mistakes:
#  - Author is the auto-guessed email, instead of a nice name.
#  - Commit message is way too short.

AUTHOR=`git config --get user.name`
EMAIL=`git config --get user.email`

if [ "x$AUTHOR" = "x" -o "x$EMAIL" = "x" ]; then
    echo "Author or email unspecified in configuration."
    echo "Please configure both by running:"
    echo "  git config --global user.name \"John Doe\""
    echo "  git config --global user.email \"john.doe@example.com\""
    exit 1
fi

exit 0
