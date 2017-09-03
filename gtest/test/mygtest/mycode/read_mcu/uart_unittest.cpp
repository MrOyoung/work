#include "uart.h"
#include "gtest/gtest.h"

TEST(UartTest, null)
{
	EXPECT_EQ(1, open_uart(NULL));
}

TEST(UartTest, right_dev)
{
	EXPECT_EQ(1, open_uart("/dev/ttymxc1"));
}
