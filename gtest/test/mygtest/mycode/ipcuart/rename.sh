#!/bin/sh

for i in `ls *.c`
do

	head=`echo $i | awk -F. '{printf("%s", $1)}'`
	echo $head

	mv $i ${head}.cpp	

	sleep 1
done
