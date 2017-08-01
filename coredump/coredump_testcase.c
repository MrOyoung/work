#include <stdio.h>
#include <string.h>

void signal_handle(int signo)
{
	printf("");
}

int main(int argc, char *argv[])
{
	char *array;
	
	memcpy(array, "hello world!", strlen("hello world!"));

	return 0;
}
