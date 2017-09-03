#include <stdio.h>
int main(int argc, char const *argv[])
{
	int i, total;
	total = 0;

	for (i = 0; i < 10; i++)
	{
		/* code */
		total += i;
	}

	if (total != 45)
	{
		/* code */
		printf("Failure\n");
	}
	else
		printf("Success\n");
	
}