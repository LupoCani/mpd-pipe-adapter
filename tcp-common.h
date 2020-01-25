#pragma once

#include "byte-utils.h"

#include <unistd.h>
#include <stdio.h>

#include <netinet/in.h> 
#include <arpa/inet.h>

#include <sys/socket.h> 
#include <sys/types.h>

#include <poll.h>

#define DIR_IN 0
#define DIR_OUT 1

#define POLLNOBLOCK (POLLIN)

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

int simple_poll(int fd, short mask, short *rmask, int timeout)
{
	struct pollfd poller;
	
	poller.fd = fd;
	poller.events = mask;
	
	int result = poll(&poller, 1, timeout);
	
	if (rmask)
		*rmask = poller.revents;
	
	return result;
}

int setup_ip_sockaddr(SA_IN *addr, short direction, short port, char * addr_str)
{	
	int res = 1;
	if (!addr)
		return 0;
		
	if (!addr_str)
		addr_str = "127.0.0.1";
		
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	
	if (direction == DIR_OUT)
		res = inet_pton(AF_INET, addr_str, &(addr->sin_addr));
	else
		addr->sin_addr.s_addr = htonl(INADDR_ANY);
		
	return res;
}

void print_bitmask(short mask)
{
	if (mask & POLLIN)		printf("POLLIN\n");
	if (mask & POLLPRI)		printf("POLLPRI\n");
	if (mask & POLLOUT)		printf("POLLOUT\n");
	if (mask & POLLERR)		printf("POLLERR\n");
	if (mask & POLLHUP)		printf("POLLHUP\n");
	if (mask & POLLNVAL)	printf("POLLNVAL\n");
}


int read_exact(int sock, byte *buffer, size_t size, int timeout)
{
	struct pollfd poller;
	poller.fd = sock;
	poller.events = POLLNOBLOCK;
	
	size_t head = 0;
	int bytes_read = 0, poll_result = 0;
	
	while (head < size)
	{
		poll_result = poll(&poller, 1, timeout);
		
		if ((poller.revents & POLLNOBLOCK) && (poll_result > 0))
			bytes_read = read(sock, &buffer[head], size - head);
		else
			return -1;
		
		if (bytes_read == 0)
			break;
		else if (bytes_read > 0)
			head += bytes_read;
		else
			return 0;
	}
	//printf("Read %u bytes.\n", head);
	return head;
}
