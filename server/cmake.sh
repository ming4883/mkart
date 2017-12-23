#!/bin/bash

builddir=.build

if [[ ! -e $builddir ]]; then

    echo "$builddir not exists" 1>&2
    mkdir $builddir
    cd $builddir

    cmake -G "Unix Makefiles" ..
    cmake ..
    if [ $? -ne 0 ]
    then
        echo "cmake failed" 1>&2
        exit 1
    fi
elif [[ ! -d $builddir ]]; then
    echo "$.build already exists but is not a directory" 1>&2
    exit 1
else
    echo "$.build exists" 1>&2
    cd $builddir
    cmake ..
fi


