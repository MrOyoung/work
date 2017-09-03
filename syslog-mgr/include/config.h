/************************************************************
**filename:config.h
**autor:Rocky
**date:2017/07/07
**description:the config message
************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

/*PATH_MAX*/
#include <dirent.h>

#include <time.h>

#define arg_in
#define arg_out

#define SEC_PER_DAY 		(24 * 60 * 60)

//#define SIZE_PER_LOGFILE	(500 * 1024)
#define SIZE_PER_LOGFILE	(5 * 1024)

/* buf size */
#define LOG_BUF_SIZE 1024

#define KBSIZE (1024)
#define MBSIZE (1024 * 1024)
#define GBSIZE (1024 * 1024 * 1024)

#define FILENO_MAX		10
#define DIRSIZE_MAX		9 * (MBSIZE)
//#define DIRSIZE_MAX		2 * (KBSIZE)


/******************* get_kmsg ***************/
#define ZTE_SYSLOG_DIR_FROM		"/proc/kmsg"
#define ZTE_SYSLOG_DIR_TO		"/home/root/log/kmsg.txt"


#define LOCAL_IP	"127.0.0.1"
#define LOCAL_PORT	9999

/* syslog-monitor message of each module-log directory */
struct LOG_DIR_UNIT
{
	int		file_size_sum;
	int		file_num_sum;
	char	file_path[PATH_MAX];
	char	oldest_file[PATH_MAX];
};

/* syslog-mgr structure : module dir info */
struct MODULE_LOG_INFO {
	int 			fd;
	unsigned int	map_pos;
	char 			*map_ptr;
	time_t	 		tm;
	char 			name[NAME_MAX];
	char 			path[PATH_MAX];
};


#endif //CONFIG_H_

