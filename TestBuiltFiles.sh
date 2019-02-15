#! /bin/bash

curr_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $curr_path

rm ./TestLog/*

for buildfile in `find -name build`
do
    tmparray=(${buildfile//// })  
    echo "Testing file $buildfile/${tmparray[2]} "
    cd  $buildfile

    if [ ${tmparray[2]} == "LLVM_Learner_Libs" ]
    then
        cd $curr_path
        continue
    fi

    for testobj in `find ../../../App/ -name *.c`
    do
        ./LLVM* $testobj f >> ../../../TestLog/testlog_${tmparray[2]}
    done
    cd $curr_path
done
