#!/bin/bash

GCOVR=../gcovr

if [ $# -lt 1 ]
then
	echo "Usage : $0 binary"
	exit
fi

#run the binary
./$1

sleep 1

filter='.+/include/.+'

output=../output

${GCOVR} --html --html-details -e ${filter} -o ${output}/coverage.html
