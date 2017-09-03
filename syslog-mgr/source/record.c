/************************************************************
 **filename		: record.c
 **description	: record the log msg
 **autor		: Rocky
 **date			: 2017/08/09
 ************************************************************/
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "Module_Def.h"
#include "Debug.h"

#include "base.h"
#include "config.h"
#include "time_str.h"

//const char *SLMGR_logdir_path = "/run/demo-log";
const char *SLMGR_logdir_path = "./demo-log";


/* gloable varaible each module has it's member, containing the basal info */
static struct MODULE_LOG_INFO module_arr[MODULE_SUM];

/* get module name according to module num */
static char *SLMGR_get_module_name(const _MODULE_NAME module_num)
{
	static char *num2name[] = {
		/* background service */
		SERVICE_CONTENTPROVIDER_NICK, SERVICE_MUTIMEDIA_NICK, SERVICE_SYSLOG_NICK,
		SERVICE_WINDOW_NICK, SERVICE_ENVIRONMENT_NICK, SERVICE_POWER_NICK,
		SERVICE_UPGRADE_NICK, 

		/* application */
		ACTIVITY_CLUSTER_NICK, ACTIVITY_LVDS_NICK,

		/* background service manager */
		SERVICEMGR_CONTENTPROVIDER_NICK, SERVICEMGR_MUTIMEDIA_NICK, SERVICEMGR_SYSLOG_NICK,
		SERVICEMGR_WINDOW_NICK, SERVICEMGR_ENVIRONMENT_NICK, SERVICEMGR_POWER_NICK,
		SERVICEMGR_UPGRADE_NICK, 

		/* application manager */
		ACTIVITYMGR_CLUSTER_NICK, ACTIVITYMGR_LVDS_NICK,

		/* hardware manager */
		HARDWAREMGR_BUS_NICK, HARDWAREMGR_LCD_NICK, HARDWAREMGR_POWER_NICK,
		HARDWAREMGR_STORAGE_NICK, HARDWAREMGR_CORE_NICK, HAREWAREMGR_LVDS_NICK,
		HAREWAREMGR_USBSTATE_NICK,

		/*kanzi applications*/
		KANZIAPP_SPEED_NICK, KANZIAPP_TACH_NICK, KANZIAPP_POPUP_NICK, KANZIAPP_TABICON_NICK,
		KANZIAPP_BACKGROUND_NICK
	};

	return num2name[module_num];	
}

static inline int SLMGR_get_filename(arg_out char *pstr)
{
	time_t 					tm;
	struct tm 				tv;
	struct timeval			utm;
	char 					*p = pstr;

	tm = time(NULL);
	localtime_r(&tm, &tv);
	gettimeofday(&utm, NULL);

	/* year */
	DIGITAL4_INC(p, tv.tm_year + 1900);
	*p++ = '-';

	/* mon */
	DIGITAL2_INC(p, tv.tm_mon + 1);
	*p++ = '-';

	/* day */
	DIGITAL2_INC(p, tv.tm_mday);
	*p++ = '-';

	/* hour */
	DIGITAL2_INC(p, tv.tm_hour);
	*p++ = '-';

	/* min */
	DIGITAL2_INC(p, tv.tm_min);
	*p++ = '-';

	/* sec */
	DIGITAL2_INC(p, tv.tm_sec);
	*p++ = '-';

	/* msec */
	DIGITAL3_INC(p, utm.tv_usec);

	*p++ = '.';
	*p++ = 'l';
	*p++ = 'o';
	*p++ = 'g';
	*p   = '\0';

	return p - pstr;
}


static int SLMGR_release_map(const _MODULE_NAME name)
{
	struct MODULE_LOG_INFO *mod_ptr = &module_arr[name];

	/* unmap files or devices into memory */
	if (mod_ptr->map_ptr)
	{
		munmap(mod_ptr->map_ptr, SIZE_PER_LOGFILE);
	}

	if (mod_ptr->fd)
	{
		/* truncate a file to a specified length */
		if (-1 == ftruncate(mod_ptr->fd, mod_ptr->map_pos))
		{
			perRtn("ftruncate error", -1);
		}

		close(mod_ptr->fd);
	}

	return 0;
}


static int SLMGR_create_map(const _MODULE_NAME name)
{
	SLMGR_LOG("create map\n");
	
	struct MODULE_LOG_INFO *mod_ptr = &module_arr[name];

	/* initial the pos value */
	mod_ptr->map_pos = 0;

	mod_ptr->fd = open(mod_ptr->path, O_RDWR | O_CREAT, 00600);
	if (-1 == mod_ptr->fd)
	{
		perRtn("fopen error", -1);
	}

	SLMGR_LOG("create map\n");
	/* truncate a file to a specified length */
	if (-1 == ftruncate(mod_ptr->fd, SIZE_PER_LOGFILE))
	{
		perRtn("ftruncate error", -1);
	}

	mod_ptr->map_ptr = (char *)mmap(
			NULL, 
			SIZE_PER_LOGFILE,
			PROT_WRITE,
			MAP_SHARED,
			mod_ptr->fd,
			0
			);
	if (-1 == (int)mod_ptr->map_ptr)
	{
		perRtn("mmap error", -1);
	}

	SLMGR_LOG("create map\n");
	return 0;
}

/* use func access() to examine pathname */
static inline int SLMGR_examine_path(const char *pathname)
{
	if (NULL == pathname)	{
		errRtn("pathname is empty", -1);
	}

	if ((access(pathname, W_OK | F_OK) != 0) && (ENOENT == errno))	{
		/* the path does not exist */
		if (-1 == mkdir(pathname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))	{
			perRtn("mkdir", -1);
		}
	}

	return 0;
}


static int SLMGR_flush_module_info(_MODULE_NAME name)
{
	struct MODULE_LOG_INFO 	*mod_ptr = &module_arr[name]; 

	/* get the newer file log name,format : xxxx-xx-xx-h:m:s:ms.log */
	SLMGR_get_filename(mod_ptr->name);
	SLMGR_LOG("filename is %s\n", mod_ptr->name);

	/* if module dir does not exist, create it, format : "log/WindowManager" eg.*/
	sprintf(mod_ptr->path, "%s/%s%c", 
			SLMGR_logdir_path, 
			SLMGR_get_module_name(name),
			'\0');

	SLMGR_examine_path((const char *)mod_ptr->path);	

	/* get the absolute path of today's log file */
	sprintf(mod_ptr->path, "%s/%s/%s%c",
			SLMGR_logdir_path,
			SLMGR_get_module_name(name),
			mod_ptr->name,
			'\0');
	SLMGR_LOG("path is %s\n", mod_ptr->path);

	return SLMGR_create_map(name);
}


/* 
 *Judge that the day changes or not 
 *	whether the log file is big enough to record current log message
 *In this case,we do not record log in day, just in file-size
 */
static int SLMGR_check_module_info(_MODULE_NAME name, unsigned int log_size)
{
	struct MODULE_LOG_INFO *mod_ptr = &module_arr[name]; 

	if (mod_ptr->fd)
	{
		/* whether map left size is big enough or not */
		if (log_size < (SIZE_PER_LOGFILE - mod_ptr->map_pos))
			return 1;

		/* release the map */
		SLMGR_release_map(name);

		SLMGR_LOG("############## no space left in the log file  ###############\n");
	}
	SLMGR_LOG("################ new file ##############\n");

	/* flush the info */
	return SLMGR_flush_module_info(name);
}


static int SLMGR_write_log(_MODULE_NAME name, unsigned char *log_msg, unsigned int log_size)
{
	struct MODULE_LOG_INFO *mod_ptr = &module_arr[name]; 

	if (-1 == SLMGR_check_module_info(name, log_size)) {
		return -1;
	}

	SLMGR_LOG("%s log_size = %d\nlog is %s\n", get_time(), log_size, log_msg);
	memcpy(mod_ptr->map_ptr + mod_ptr->map_pos, log_msg, log_size);

	/* synchronize a file with a memory map */
	//if (-1 == msync(mod_ptr->map_ptr + mod_ptr->map_pos,
	//			log_size,
	//			MS_ASYNC))
	if (-1 == msync(mod_ptr->map_ptr,
				SIZE_PER_LOGFILE,
				MS_ASYNC))
	{
		perRtn("msync error", -1);
	}

	mod_ptr->map_pos += log_size;

	return 0;
}

/* record the log msg */
int SLMGR_record_log(unsigned int msg_id, unsigned char *log_msg, unsigned int log_size)
{
	/* module_name = msg_id now */
	_MODULE_NAME name = msg_id; 

	/* check the parameter */
	if ((name < 0) || (name > MODULE_SUM)) {
		errRtn("module name error", -1);	
	}

	if (-1 == SLMGR_examine_path(SLMGR_logdir_path)) {
		exit(-1);
	}

	return SLMGR_write_log(name, log_msg, log_size);
}

