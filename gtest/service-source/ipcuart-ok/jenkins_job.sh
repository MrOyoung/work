#!/bin/bash

make -f makefile_cpp clean
make -f makefile_cpp
./ipc_uart_utest
#gcovr --xml-pretty --root=`pwd` > coverage.xml 
gcovr --html --html-details
