#include <stdio.h>
#include <pthread.h>

#include "base.h"

int debug_mode = 1;

int main(int argc, char *argv[])
{
	udisk_hotplug_monitor(NULL);

	return 0;
}
