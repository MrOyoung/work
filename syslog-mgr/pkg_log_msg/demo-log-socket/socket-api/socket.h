/************************************************************
*filename	 :	socket.h
*description :	socket head file	
*author		 :	rocky
*date		 :	2017/08/09
************************************************************/

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/select.h>

#include <arpa/inet.h>
#include <netinet/in.h>


#define BUFFER_SIZE 1024

enum socket_type
{
	SOCKET_CLIENT,
	SOCKET_SERVER
};

struct socket_info
{
	int sockfd;
	int backlog;			/* maximum connection */
	enum socket_type type;	/* socket type */
	socklen_t size;			/* accept() parameter */

	unsigned int retsize;

	struct sockaddr_in in;

	unsigned char buf[0];
};


typedef void *(*socket_data_handle)(void *data, unsigned int size);


int socket_create(struct socket_info *info, const char *addr, const int port);
int socket_connect(const struct socket_info info);

int socket_send(struct socket_info *info, unsigned char *buf, unsigned int size);
int socket_dispatch_timeout(const struct socket_info info, socket_data_handle handler, struct timeval *tm);


void socket_info_release(struct socket_info *info);
void socket_close(int sockfd);

#endif //_SOCKET_H_
