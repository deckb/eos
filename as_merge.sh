#!/bin/bash
set -e
if [ $# -ne "1" ]; then
    echo "need to specify a version"
    exit 1
else
    version=$1
fi

#
git checkout master
# pull down changes
git pull https://github.com/eosio/eos.git v$version

# tag
git tag -a v$version -m "merge v$version"
git commit
# push changes
git push origin master
