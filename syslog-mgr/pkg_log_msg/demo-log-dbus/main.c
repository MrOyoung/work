#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <sys/time.h>

#include "config.h"

int main(int argc, char *argv[])
{
	DBusConnection *bus;
	int pos = 0;

	/*create a dbus handler*/
	if (!(bus = get_dbus("com.saic.WindowsManager"))){
		return -1;
	}

	/* set the monitor rules */
	if (dbus_add_match(bus, "type='signal',path='/com/saic/WindowsManager'") < 0){
		return -1;
	}

	DEMO_set_dbus_object(bus);

	while (1)
	{
		DEMO_LOG_INFO("info : %d", pos++);
		sleep(1);

		DEMO_LOG_WARN("warn : %d", pos++);
		sleep(1);

		DEMO_LOG_ERROR("error : %d", pos++);
		sleep(1);

		DEMO_LOG_EMERG("emerg : %d", pos++);		
		sleep(1);
	}
	return 0;
}
