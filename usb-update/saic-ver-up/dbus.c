#include "base.h"
#include "cfg.h"

static DBusConnection *g_conn = NULL;
static pthread_key_t msg_key;

int dbus_init(void)
{
	DBusError err;

	dbus_error_init(&err);
	g_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if(dbus_error_is_set(&err)) {
		print_error(err.message);  
		dbus_error_free(&err);
		return -1;
	}
	if(g_conn == NULL) {
		printf("get connection fail\n");
		return -1;
	}

	int ret =dbus_bus_request_name(g_conn, DBUS_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);  
	if(dbus_error_is_set(&err)) {  
		print_error(err.message);  
		dbus_error_free(&err);
		return -1;
	}
	if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)  {
		printf("not PRIMARY_OWNER\n");
		return -1;  
    }

	pthread_key_create(&msg_key, NULL); 
	return 0;
}

/* return the method message */
int reply_dbus_msg(int ret, char *str)
{
	DBusMessage *msg = (DBusMessage *)pthread_getspecific(msg_key);
	DBusMessage *reply;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;

	reply = dbus_message_new_method_return(msg);
	if (!reply) {
		print_error("dbus_message_new_method_return");
		return -1;
	}

	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ret)) { 
		dbus_message_unref(reply);
		print_error("Out Of Memory!"); 
		return -1;
	}

	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str)) { 
		dbus_message_unref(reply);
		print_error("Out Of Memory!"); 
		return -1;
	}

	if (!dbus_connection_send(g_conn, reply, &serial)) {
		dbus_message_unref(reply);
		print_error("Out Of Memory!");
		return -1;
	}

	dbus_connection_flush(g_conn);
	dbus_message_unref(reply);
	return 0;
}

int dbus_msg_check_no_param(void)
{
	DBusMessage *msg = (DBusMessage *)pthread_getspecific(msg_key);
	DBusMessageIter args;

	/* initialize a DBusMessageIter for reading the arguments of the message passed in
	 *if the message has arguments, return -1	 
	 */
	if (dbus_message_iter_init(msg, &args)) {
		reply_dbus_msg(-1, "param is not need");
		return -1;
	}

	return 0;
}


int dbus_msg_check_get_arg(char **str)
{
	DBusMessage *msg = (DBusMessage *)pthread_getspecific(msg_key);
	DBusMessageIter args;

	if (!dbus_message_iter_init(msg, &args)) {
		reply_dbus_msg(-1, "no param");
		return -1;
	} else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
		reply_dbus_msg(-1, "param type is not string");
		return -1;
	}

	/* reads a basic-typed value from the message iterator */
	dbus_message_iter_get_basic(&args, str);
	return 0;
}

DBusMessage *get_dbus_msg(void)
{
	dbus_connection_read_write(g_conn, 0);  
	return dbus_connection_pop_message(g_conn);
}

static int release_dbus_msg(void)
{
	DBusMessage *msg = (DBusMessage *)pthread_getspecific(msg_key);
	if (!msg)
		return -1;

	dbus_message_unref(msg);
	return 0;
}

int dbus_msg_method(const char* method)
{
	DBusMessage *msg = (DBusMessage *)pthread_getspecific(msg_key);

	/* checkout the method call */
	if (!dbus_message_is_method_call(msg, DBUS_INTERFACE, method))
		return 0;

	if (debug_mode)
		printf("[debug] dbug msg method: %s\n", method);
	
	return 1;
}

static void *dbus_thread(void *arg)
{
	if (pthread_setspecific(msg_key, arg)) {
		print_error("pthread_setspecific");
		return NULL;
	}

	extern int process_dbus_request(void);
	process_dbus_request();

	release_dbus_msg();
	return NULL;
}

int start_dbus_thread(DBusMessage *msg)
{
	pthread_t th;
	if (pthread_create(&th, NULL, dbus_thread, msg) != 0) {
		print_error("pthread_create");
		return -1;
	}

	return 0;
}

