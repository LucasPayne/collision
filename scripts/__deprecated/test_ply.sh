#!/bin/sh

if [ $# -ne 1 ] ; then
    echo give good args
    exit 1
fi

make text_processing
build/text_processing $1

