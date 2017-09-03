#include "demo_log.h"

int main(int argc, char *argv[])
{
	int pos = 0;

	printf("begin...\n");

	while (1)
	{
		DEMO_LOG_INFO("info : %d", pos++);
		usleep(200000);

		DEMO_LOG_WARN("warn : %d", pos++);
		usleep(200000);

		DEMO_LOG_ERROR("error : %d", pos++);
		usleep(200000);

		DEMO_LOG_EMERG("emerg : %d", pos++);		
		usleep(200000);
	}
	return 0;
}
