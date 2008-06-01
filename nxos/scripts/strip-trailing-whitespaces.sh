#!/bin/sh
#
# Trailing whitespace is the work of the devil and VI users. This
# script scans all source code files and strips out trailing
# whitespace.

SYSTEM=`uname -s`

if [ "$SYSTEM" = "Darwin" ]; then
  # Apple's sed is RETARDED. There is no other way to say this, it is
  # simply retarded. -i requires an extension, and there is no way
  # that we can see to not have it generate backups. What does one do
  # then? Give it the finger.
  #
  # TODO: OSX sed apparently fails to catch tabs with the expression
  # that works with GNU sed. Figure out a way around that.
  for i in `find . -name '*.[chS]'`; do
    sed -i ".re-bak" -E -e 's/ +$//g' $i
  done
  find . -name '*.re-bak' -print0 | xargs -0 rm
else
  # Assume GNU sed by default. Nice, sane, normal GNU sed.
  for i in `find . -name '*.[chS]'`; do
    echo $i
    sed -i -r -e 's/[ \t]+$//g' $i
  done
fi

