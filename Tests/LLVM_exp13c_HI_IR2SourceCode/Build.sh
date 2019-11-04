#! /bin/bash

curr_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $curr_path

if [ ! -d ./build ]
then
    mkdir build
fi

cd build
rm -rf *
cmake .. 
make -j4
cd ..