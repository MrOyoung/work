#!/bin/bash
#Please place this script into path of ~utest
# <> content is up to your specific definition.


#set GCOVR path
#GCOVR="../../../gcovr/gcovr"
GCOVR="./gcovr-3.2/scripts/gcovr"

#find -name "*.gcda" -exec rm -r {} \;

#rm -rf out

#make

#echo "find _unit_test and run the utest cases"
#find -name "*_unit_test" -exec {} \;

echo "generate coverage report"
#$GCOVR --html --html-detail -o ./out/coverage/coverage.html
$GCOVR --html --html-detail -o coverage.html
$GCOVR --xml > coverage.xml

#  -e '/usr' \
#  -e '.+/unit_test/.+'  \
#  -e '.+/unit_tests/.+' \
#  -e '.+/gtest/.+'  \  
