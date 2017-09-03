/************************************************************
 **filename		:base.h
 **description	:declare function
 **autor		:Rocky
 **date			:2017/08/08
 ************************************************************/

#ifndef _BASE_H_
#define _BASE_H_


/****************** syslog_mgr ****************/
extern void *SLMGR_start_record(void *argv);


/****************** syslog_monitor ****************/
extern void *SLMGR_start_monitor_logdir(void *argv);


/****************** get kmsg ****************/
extern void *SLMGR_start_monitor_kmsg(void *arg);


/****************** record ****************/
extern const char *SLMGR_logdir_path;
extern int SLMGR_record_log(unsigned int msg_id, unsigned char *log_msg, unsigned int log_size);

#endif //_BASE_H_
