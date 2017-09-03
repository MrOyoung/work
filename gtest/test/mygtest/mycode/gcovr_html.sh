#!/bin/bash

GCOVR=../gcovr

output=../output

#gtest reports output file
GTEST_OUTPUT=didata.gtest

#checkout parameter
if [ $# -lt 2 ]
then
	echo "[ Failed ] : Usage: $0 user_dir bin_file"
	exit
fi

#chang work path
if [ -d $1 ]
then
	cd $1
else
	echo "[ Failed ] : no such directory"
	exit
fi

#run the binary file
#./$2 > ${output}/${GTEST_OUTPUT}
./$2

filter='.+/include/.+'

${GCOVR} --html --html-details -e ${filter} -o ${output}/coverage.html
