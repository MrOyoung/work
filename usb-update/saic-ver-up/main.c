#include "base.h"
#include "cfg.h"


extern int update_init(char *fileset);
static int do_listfile(void)
{
	extern int dbus_msg_check_no_param(void);
	if (dbus_msg_check_no_param())
		return -1;

	extern int get_version_fileset(void);
	if (get_version_fileset())
		return -1;

	return 0;
}

static int do_update(void)
{
	char *ver_file;

	extern int dbus_msg_check_get_arg(char **str);
	if (dbus_msg_check_get_arg(&ver_file))
		return -1;

	extern int parse_ver_up_file(char *str);
	if (parse_ver_up_file(ver_file))
		return -1;

	extern int update_version(void);
	if (update_version())
		return -1;

	reply_dbus_msg(0, "");
	return 0;
}

static int do_runnew(void)
{
	extern int dbus_msg_check_no_param(void);
	if (dbus_msg_check_no_param())
		return -1;

	extern int set_up_ver_run(void);
	if (set_up_ver_run()) {
		reply_dbus_msg(0, "write cfg to mmc fail");
		return -1;
	}

	reply_dbus_msg(0, "");
	return 0;
}

static int do_setvalid(void)
{
	extern int dbus_msg_check_no_param(void);
	if (dbus_msg_check_no_param())
		return -1;

	extern int set_cur_ver_valid(void);
	if (set_cur_ver_valid()) {
		reply_dbus_msg(0, "write cfg to mmc fail");
		return -1;
	}

	reply_dbus_msg(0, "");
	return 0;
}

static int do_query(void)
{
	extern int dbus_msg_check_no_param(void);
	if (dbus_msg_check_no_param())
		return -1;

	extern int get_update_process(void);
	get_update_process();

	return 0;
}

int process_dbus_request(void)
{
	extern int dbus_msg_method(const char* method);

	if (dbus_msg_method("listfile"))
		do_listfile();
	else if (dbus_msg_method("update"))
		do_update();
	else if (dbus_msg_method("runnew"))
		do_runnew();
	else if (dbus_msg_method("setvalid"))
		do_setvalid();
	else if (dbus_msg_method("query"))
		do_query();

	return 0;
}

int debug_mode = 0;

int main(int argc ,char** argv)
{
	if (argc > 2) {
		printf("Usage: %s [debug]\n", argv[0]);
		return -1;
	}

	if ((argc == 2) && !strcmp(argv[1], "debug"))
		debug_mode = 1;

	extern int version_update_init(void);
	if (version_update_init())
		return -1;

	while(1) {
		extern DBusMessage *get_dbus_msg(void);
		DBusMessage *msg = get_dbus_msg();
		if (!msg) {
			sleep(1);
			continue;
		}

		extern int start_dbus_thread(DBusMessage *msg);
		start_dbus_thread(msg);
	}

	return 0;
}

