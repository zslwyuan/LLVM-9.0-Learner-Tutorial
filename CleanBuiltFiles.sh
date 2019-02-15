#! /bin/bash

curr_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $curr_path

for buildfile in `find -name build`
do
    echo "removing files in $buildfile/* "
    rm $buildfile/* -rf
done

for archivefile in `find -name *.a`
do
    echo "removing file $archivefile"
    rm $archivefile
done
