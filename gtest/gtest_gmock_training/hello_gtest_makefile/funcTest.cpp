#include <iostream>
using namespace std;

#include <limits>
#include "gtest/gtest.h"
#include "func.h"

TEST(AdditionTest,twoValues)
{
    EXPECT_EQ(3,func(1, 2));
}
