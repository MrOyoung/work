#include "sqrt.h"
#include "gtest/gtest.h"

TEST(SqrtTest, Zero)
{
	EXPECT_EQ(0, sqrt(0));	
}

TEST(SqrtTest, Positive)
{
	EXPECT_EQ(100, sqrt(10000));
	EXPECT_EQ(1000, sqrt(1000009));
	EXPECT_EQ(99, sqrt(9810));
}

TEST(SqrtTest, Negative)
{
	int i = -1;
	EXPECT_EQ(0, sqrt(i));
}
