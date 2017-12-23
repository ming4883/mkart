#!/bin/bash

builddir=.build

if [[ ! -e $builddir ]]; then

    echo "$builddir not exists, please run cmake.sh first" 1>&2
    exit 1

elif [[ ! -d $builddir ]]; then
    echo "$.build already exists but is not a directory" 1>&2
    exit 1    
fi

cd $builddir
make

if [ $? -ne 0 ]
then
    exit 1
fi

printf "\033[0;32mSUCCEEDED :-)"

source /work/rpi_config.sh
scp v4l2-server ${RPI_USERNAME}@${RPI_HOSTNAME}:/home/${RPI_USERNAME}/
