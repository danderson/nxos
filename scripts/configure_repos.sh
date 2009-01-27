#!/bin/bash
#
# Copyright (c) 2009 the NxOS developers
#
# See AUTHORS for a full list of the developers.
#
# Redistribution of this file is permitted under
# the terms of the GNU Public License (GPL) version 2.
#
# Sets up a freshly cloned repository with a few useful things.

REPOS_URL="git://natulte.net/nxos"

echo `pwd`

if [ ! -f "scripts/configure_repos.sh" ]; then
  echo "Please run from the root of your repository."
  exit 1
fi

git remote add mainline git://github.com/danderson/nxos.git

cat <<EOF >.git/hooks/pre-commit
#!/bin/bash

ALL_OK=1

check_result () {
  if [ \$? -ne 0 ]; then
    ALL_OK=0
    echo
  fi
}

./scripts/copyright_report.py
check_result

./scripts/trailing_whitespace_report.py
check_result

./scripts/metadata_report.sh
check_result

if [ \$ALL_OK -ne 1 ]; then
  echo "Precommit hooks failed, see above."
  exit 1
else
  exit 0
fi
EOF

chmod +x .git/hooks/pre-commit
