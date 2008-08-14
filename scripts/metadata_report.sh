#!/bin/bash
#
# Copyright (c) 2008 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.
#
# Check the commit metadata for common mistakes:
#  - Author is the auto-guessed email, instead of a nice name.
#  - Commit message is way too short.

AUTHOR=`hg tip --template {author}`
AUTHOR_VALID=`echo $AUTHOR | egrep -i '[a-z ]+ (\[[a-z0-9 ]+\] )?<[a-z.+@]+>'`

if [ "x$AUTHOR_VALID" = "x" ]; then
  echo "Changeset author does not conform to the pattern Firstname Lastname <email>"
  echo " eg. John Smith <jsmith@example.com>"
  echo "Please add the ui:username variable to you ~/.hgrc. Here is a sample .hgrc:"
  echo
  echo "[ui]"
  echo "username = John Smith <jsmith@example.com>"
  echo
  hg tip --template {desc} >commit.msg
  echo "Commit message saved in commit.msg"
  exit 1
fi

MSG_OK=`hg tip --template {desc} | wc -c`

if [ $MSG_OK -lt 15 ]; then
  echo "Your commit message is very short, and probably doesn't describe your"
  echo "change very well. Please be a little more verbose."
  echo
  hg tip --template {desc} >commit.msg
  echo "Commit message saved in commit.msg"
  exit 1
fi

exit 0
