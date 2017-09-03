#include <iostream>
using namespace std;

#include <limits.h>
#include "gtest/gtest.h"

#include "works.h"
#include "uart.h"


TEST(GTEST_DEMO, return_int_para_int_int)
{  
init_buffer();

   printf("starting test......\n");
   //EXPECT_EQ(17, add(7, 10));
   EXPECT_EQ(0, open_uart("/dev/ttymxc1"));
   printf("GTEST_DEMO done!\n");
}
