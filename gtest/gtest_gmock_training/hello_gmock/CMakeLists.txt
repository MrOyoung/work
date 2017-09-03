#CMAKE MINIMUM REQUIRED(VERSION 2.8)
#include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
#link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
#add_definitions(-std=c++0x)
#set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall")


project(hello_gmock)
set(SRC_LIST	HelloGmock.cpp 
   				HelloWorld.cpp)
set(GMOCK_PATH /home/user/gwm/linuxrampup/gmock_rampup/gmock/gmock)
set(GMOCK_PATH /home/user/gwm/linuxrampup/gmock_rampup/gmock/)

LINK_DIRECTORIES(/usr/lib/)
include_directories (
#	${PROJECT_SOURCE_DIR}/include
	/usr/include/
	${GMOCK_PATH}/gmock/
	${GMOCK_PATH}/gmock/include
	${GMOCK_PATH}/gmock/gtest
	${GMOCK_PATH}/gmock/gtest/include
)
FIND_PACKAGE(Threads REQUIRED)
ADD_EXECUTABLE(hello_gmock ${SRC_LIST})
#TARGET_LINK_LIBRARIES(hello_gtest libgtest.a libgtest_main.a libpthread)
#TARGET_LINK_LIBRARIES(hello_gtest libgtest.a libgtest_main.a pthread)
#TARGET_LINK_LIBRARIES(hello_gtest libgtest.a libgtest_main.a libpthread.a)
#TARGET_LINK_LIBRARIES(hello_gtest libgtest.a libgtest_main.a)
#TARGET_LINK_LIBRARIES(hello_gtest libgtest.a libgtest_main.a ${CMAKE_THREAD_LIBS_INIT})
#TARGET_LINK_LIBRARIES(hello_gmock libgmock_main.a libgtest.a pthread)
TARGET_LINK_LIBRARIES(hello_gmock gmock gtest pthread)
