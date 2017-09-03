/************************************************************
 *filename	 	:	main.c
 *description 	:	socket api	
 *author		:	rocky
 *date		 	:	2017/08/09
 ************************************************************/

#include "socket.h"
#include "../time_str.h"

#define MAX_ONE(maxfd,newfd) \
	(maxfd = ((newfd > maxfd) ? (newfd) : (maxfd)))


int socket_create(struct socket_info *info, const char *addr, const int port)
{
	if (!info || !addr || (port <= 0))
	{
		printf("para error\n");
		return -1;
	}

	if (-1 == (info->sockfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("socket error");
		return -1;
	}

	info->in.sin_family = AF_INET;
	info->in.sin_addr.s_addr = inet_addr(addr);	/* INADDR_ANY : loop */
	info->in.sin_port = htons(port);

	/* used in accept(), the third parameter */
	info->size = sizeof(struct sockaddr_in);

	if (SOCKET_CLIENT == info->type)
	{
		/* There is no need to bind and listen for socket client */
		printf("SOCKET_CLIENT\n");
		return 0;
	}
	printf("SOCKET_SERVER\n");

	if (-1 == bind(info->sockfd, (struct sockaddr *)&info->in, sizeof(info->in)))
	{
		perror("bind error");
		return -1;
	}

	if (info->backlog)
	{
		if (-1 == listen(info->sockfd, info->backlog))
		{
			perror("listen error");
			return -1;
		}
	}

	return 0;
}

/* close the connection fd */
inline void socket_close(int sockfd)
{
	if (sockfd)
	{
		close(sockfd);
	}
}


int socket_connect(const struct socket_info info)
{
	return connect(info.sockfd, (struct sockaddr *)&info.in, sizeof(struct sockaddr));
}


static inline int socket_accept(const struct socket_info server_info, struct socket_info *client_info)
{
	return accept(server_info.sockfd, (struct sockaddr *)&client_info->in, &client_info->size);
}


/* recv size : use the default value - 1024 */
static inline int socket_recv(struct socket_info *info)
{
	info->retsize = recv(info->sockfd, info->buf, BUFFER_SIZE, 0);
	return info->retsize;
}


int socket_send(struct socket_info *info, unsigned char *buf, unsigned int size)
{
	info->retsize = send(info->sockfd, buf, size, 0);
	return info->retsize;
}

static inline struct socket_info *socket_info_alloc(unsigned int buf_size)
{
	/* use the default value 1024 when buf_size is zero */
	if (0 == buf_size)
		buf_size = BUFFER_SIZE;

	return (struct socket_info *)calloc(1, sizeof(struct socket_info) + buf_size );
}

void socket_info_release(struct socket_info *info)
{
	if( !info )
		return ;

	socket_close(info->sockfd);
	
	return free(info);
}


int socket_dispatch_timeout(const struct socket_info info, socket_data_handle handler, struct timeval *tm)
{
	int retval;
	int index;
	int maxindex = -1;

	struct socket_info *conn = socket_info_alloc(0);

	fd_set read_set;
	fd_set read_set_update;

	FD_ZERO(&read_set_update);
	FD_SET(info.sockfd, &read_set_update);

	int maxfd;
	maxfd = info.sockfd;

	int client[FD_SETSIZE];
	memset(client, -1, sizeof(client));

	while (1)
	{
		/* The read_set will bzero after select()
		so we must reset the read_set */
		read_set = read_set_update;
		printf("wait for .....\n");
		retval = select(maxfd + 1, &read_set, NULL, NULL, tm);
		if (-1 == retval)
		{
			perror("select error");
			break;
		}

		if (0 == retval)
			continue;

		if (FD_ISSET(info.sockfd, &read_set))
		{
			/* new client connection */
			if (-1 == (conn->sockfd = socket_accept(info, conn)))
			{
				perror("aceept error");
				continue;
			}
			printf("new client connection, client fd = %d\n", conn->sockfd);

			for (index = 0; index < FD_SETSIZE; index++)
			{
				if (-1 == client[index])
				{
					client[index] = conn->sockfd;
					break;
				}
			}

			if (FD_SETSIZE == index)
			{
				printf("too many conns\n");
				exit(-1);
			}

			/* add the new fd into the read_set */
			FD_SET(client[index], &read_set_update);

			/* max used file discriptor */
			MAX_ONE(maxfd, client[index]);
			printf("maxfd = %d\n", maxfd);	

			/* max index */
			MAX_ONE(maxindex, index);
			printf("maxindex = %d\n", maxindex);

			/* no more readable fd */
			if (0 == --retval)
				continue;
		}

		/* new message from client */
		for (index = 0; index <= maxindex; index++)
		{
			if (info.sockfd == (conn->sockfd = client[index]))
				continue;

			if (!FD_ISSET(client[index], &read_set))
				continue;

			if (-1 == socket_recv(conn))
				printf("recv error \n");
			else if (0 == conn->retsize)
			{
				printf("remove client fd = %d\n", client[index]);
				/* connection closed by client */
				FD_CLR(client[index], &read_set_update);
				socket_close(client[index]);
				client[index] = -1;
			}
			else
			{
				printf("#####################################\n");
				fprintf(stderr, "%s conn->retsize = %d fd = %d\n", get_time(), client[index], conn->retsize);
				printf("conn->buf is %s \n", conn->buf);
				handler((void *)conn->buf, conn->retsize);
				printf("#####################################\n");
			}
			
			/* no more readable fd */
			if (0 == --retval)
				break;
		}
	}//end of while(1)

	socket_info_release(conn);	

	return 0;
}

