/*
*filename 	: Debug.h
*descriptor	: define interface for debugging
*author		: Rocky
*date		: 20170619
*/

#ifndef _ROCKY_DEBUG_H_ 
#define _ROCKY_DEBUG_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define perRtn(arg, val) 	do { perror(arg); return val; } while(0)
#define errRtn(arg, val) 	do { printf(arg); return val; } while(0)
#define errExit(arg)		do { perror(arg); exit(EXIT_FAILURE); } while(0)

#define SLMGR_LOG(fmt, args...)		printf("[FILE:%s.c]  [Line:%d] "fmt"", __FILE__, __LINE__, ##args);			

#endif //_ROCKY_DEBUG_H
