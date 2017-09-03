#include <gtest/gtest.h>
#include "Hello_gtest.h"

//int Gcd(int a, int b)
//{
//	return (0==b) ? (a) : Gcd(b, a%b);
//}

int IsPrimeArray[] = {3, 4, 5, 6, 7, 8, 9, 10};

TEST(GcdTest, IntTest)
{
	EXPECT_EQ(1, Gcd(2, 5));
	EXPECT_EQ(2, Gcd(2, 5));
	EXPECT_EQ(2, Gcd(2, 4));
	EXPECT_EQ(3, Gcd(6, 9));
}


//必须添加一个类，继承testing::TestWithParam<T>，其中T就是你需要参数化的参数类型
class IsPrimeParamTest : public::testing::TestWithParam<int>
{};

TEST_P(IsPrimeParamTest, HandleTrueReturn)
{
	int n = GetParam();
	EXPECT_TRUE(IsPrime(n));
	// 第一个参数是测试案例的前缀，可以任意取。 
	// 第二个参数是测试案例的名称，需要和之前定义的参数化的类的名称相同，如：IsPrimeParamTest 
	// 第三个参数是可以理解为参数生成器，上面的例子使用test::Values表示使用括号内的参数。Google提供了一系列的参数生成的函数：
// Range(begin, end[, step])	范围在begin~end之间，步长为step，不包括end
// Values(v1, v2, ..., vN)	v1,v2到vN的值
// ValuesIn(container) and ValuesIn(begin, end)	从一个C类型的数组或是STL容器，或是迭代器中取值
// Bool()	取false 和 true 两个值
// Combine(g1, g2, ..., gN)	
// 这个比较强悍，它将g1,g2,...gN进行排列组合，g1,g2,...gN本身是一个参数生成器，每次分别从g1,g2,..gN中各取出一个值，组合成一个元组(Tuple)作为一个参数。
// 说明：这个功能只在提供了<tr1/tuple>头的系统中有效。gtest会自动去判断是否支持tr/tuple，如果你的系统确实支持，而gtest判断错误的话，你可以重新定义宏GTEST_HAS_TR1_TUPLE=1。
}

//INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Values(3, 5, 11, 23, 17));
//INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Range(3, 23, 1));
//INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::ValuesIn(IsPrimeArray));
//INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Bool());


//重要：编写死亡测试案例时，TEST的第一个参数，即testcase_name，请使用DeathTest后缀。
//原因是gtest会优先运行死亡测试案例，应该是为线程安全考虑。
TEST(FailedFuncDeathTest, Demo)
{
	EXPECT_DEATH(FailedFunc(), "*Failed");
	EXPECT_DEATH(FailedFunc(), "*FAAA");
	EXPECT_DEATH(FailedFunc(), "");
}


int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


//g++ Hello_gtest.cpp -lgtest_main -lgtest -lpthread -o hell_gtest
