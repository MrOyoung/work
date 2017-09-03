#g++ -o hello_gmock HelloGmock.cpp -lpthread -lgtest -lgmock
#GMOCK_ROOT=/home/user/gwm/linuxrampup/gtest_rampup/gmock/gmock/
#g++ -I${GMOCK_ROOT}/include/ HelloGmock.cpp -L${GMOCK_ROOT}/build -lgmock -lpthread
#g++ -o hello_gmock HelloGmock.cpp Messenger.cpp HelloWorld.cpp -lpthread -lgtest -lgmock_main
g++ -o hello_gmock HelloGmock.cpp HelloWorld.cpp -lpthread -lgtest -lgmock