#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include "socks.h"

int socks_request(int sfd)
{
	char buf[1024];
	memset(buf, 0, 1024);
	read(sfd, buf, 1024);	
	printf("%s\n", buf);
	return 0;
}

int sub_deamon(int sfd)
{
	int err;

	//check owner and set public key
	err = socks_request(sfd);


	return 0;
}
