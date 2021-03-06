#include "GreatestCommonDivisor.h"
#include <iostream>
using namespace std;
int Gcd(int a, int b)
{
	return (0==b) ? (a) : Gcd(b, a%b);
}

// Returns true if n is a prime number.
bool IsPrime(int n)
{
    // Trivial case 1: small numbers
    if (n <= 1) return false;

    // Trivial case 2: even numbers
    if (n % 2 == 0) return n == 2;

    // Now, we have that n is odd and n >= 3.

    // Try to divide n by every odd number i, starting from 3
    for (int i = 3; ; i += 2) {
        // We only have to try i up to the squre root of n
        if (i > n/i) break;

        // Now, we have i <= n/i < n.
        // If n is divisible by i, n is not prime.
        if (n % i == 0) return false;
    }
    // n has no integer factor in the range (1, n), and thus is prime.
    return true;
}

//death test
void FailedFunc()
{
	std::cerr<<"Failed function";
	_exit(0);
}