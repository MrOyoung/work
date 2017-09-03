/************************************************************
 **filename:syslog_monitor.c
 **autor:Rocky
 **date:2017/05/24
 **description:
 ************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "Debug.h"
#include "config.h"

#include "base.h"

static int SLMGR_scan_logdir(const char *dirpath);

/* start to monitor log directory */
void *SLMGR_start_monitor_logdir(void *argv)
{
	while ( 1 )	{
		/* loop */
		SLMGR_scan_logdir(SLMGR_logdir_path);

		sleep(5);
	}

	return NULL;
}

static int SLMGR_get_file_size(const char *dirpath, const char *filename);
static int SLMGR_get_oldest_file(char *older_file ,const char *filename);
static int SLMGR_remove_oldest_file(const char *filename);

/*********************************************************************************
 **Function		: _do_scan_logdir
 **Descriptor	: a call-back function
 **		Use the func readdir() to scan the parameter(dirpath)
 **			if the member is a directory, reference this func;
 **			if the member is a directory, record the variable among struct LOG_DIR_UNIT
 **		After readdir(), do check the size of the file in the directory
 **Autor		: Rocky
 **Date			: 2017-05-26
 **Version		: log_mgr V0.1
 **********************************************************************************/
static int SLMGR_scan_logdir(const char *dirpath)
{
	if (!dirpath)	{
		printf("dirpath is empty\n");
		return -1;
	}

	DIR 				*dir_ptr;
	char 				deeper_dir[PATH_MAX] = {0};
	struct dirent		*direntp;
	struct LOG_DIR_UNIT mod_dir_unit;

	bzero(&mod_dir_unit, sizeof(struct LOG_DIR_UNIT));

	if (NULL == (dir_ptr = opendir((char *)dirpath)))	{
		perror("opendir error");
		return -1;
	}

	while ((direntp = readdir(dir_ptr)) != NULL) {

		if ((!strcmp(direntp->d_name, ".")) || (!strcmp(direntp->d_name, "..")))	{
			continue;
		}

		sprintf(deeper_dir, "%s/%s", dirpath, direntp->d_name);
		if (DT_DIR == direntp->d_type) {
			//directory
			SLMGR_scan_logdir(deeper_dir);
		}
		else if (DT_REG == direntp->d_type) {
			//regular file
			mod_dir_unit.file_size_sum += SLMGR_get_file_size(dirpath, direntp->d_name); 
			mod_dir_unit.file_num_sum++;

			SLMGR_get_oldest_file(mod_dir_unit.oldest_file, deeper_dir);//保存时间戳最旧的log文件
		}

	}/* end of while() */

	/* After scaning a module directory, delete the oldest file 
	 *	if the sum size of the log-file is bigger than DIRSIZE_MAX 
	 *	or the num of the log-file is bigger than FILENO_MAX 
	 */
	//if ((mod_dir_unit.file_size_sum > DIRSIZE_MAX) || (mod_dir_unit.file_num_sum > FILENO_MAX))

	/* After scaning a module directory, delete the oldest file
	 *	if the sum size of the log-file is bigger than DIRSIZE_MAX  
	 */
	if ((mod_dir_unit.file_size_sum > DIRSIZE_MAX)) {
		SLMGR_remove_oldest_file(mod_dir_unit.oldest_file);
	}

	closedir(dir_ptr);

	return 0;
}


/* get the size of the file */
static int SLMGR_get_file_size(const char *dirpath, const char *filename)
{
	char		logfile_ptr[PATH_MAX] = {0};
	struct stat info;

	sprintf(logfile_ptr, "%s/%s", dirpath, filename);

	if (-1 == stat(logfile_ptr, &info)) {
		perror("stat error");
		return 0;
	}

	return info.st_size;
}


/* if older_file is empty or newer than filename, replace it by filename */
static int SLMGR_get_oldest_file(char *older_file ,const char *filename)
{
	if ((0 == strlen(older_file)) || 
		(strcmp(older_file, filename) > 0)) {

		bzero(older_file, strlen(older_file));
		strncpy(older_file, filename, strlen(filename));
	}

	return 0;
}


static int SLMGR_remove_oldest_file(const char *filename)
{
	if (strlen(filename)) {
		if (-1 == remove(filename))
			perRtn("remove error", -1);

		printf("remove %s success\n", filename);
	}

	return 0;
}

