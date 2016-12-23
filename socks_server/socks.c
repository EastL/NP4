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

int socks_request(int sfd, socks_t **socks)
{
	unsigned char buf[1024];
	memset(buf, 0, 1024);
	if (read(sfd, buf, 1024) < 0)
		return SER_ERR;

	for (int i = 0; i < 20; i++)
		printf("%x\n", buf[i]);

	socks_t *temp = malloc(sizeof(socks_t));
	temp->vn = buf[0];
	temp->cd = buf[1];
	temp->port = buf[2] * 256 + buf[3];
	
	char IP[20];
	memset(IP, 0, 20);

	sprintf(IP, "%d.%d.%d.%d", buf[4], buf[5], buf[6], buf[7]);
	strcpy(temp->ip, IP);

	int uid_len = strlen((char*)&buf[8]);
	strcpy(temp->user_id, (char*)&buf[8]);


	printf("VN: %d, CD: %d, DST IP: %s, DST PORT: %d, USERID: %s\n", temp->vn, temp->cd, temp->ip, temp->port, temp->user_id);

	if ((buf[4] == 0) && (buf[5] == 0) && (buf[6] == 0))
	{
		strcpy(temp->domain, (char*)&buf[9+uid_len]);
		printf("domain name: %s\n", temp->domain);
	}
	
	*socks = temp;
	
	return 0;
}

int firewall(int sfd, socks_t **socks)
{
	FILE *fd = fopen("socks.conf", "r");
	char buf[1024];
	int check = 1;


	while(!feof(fd))
	{
		memset(buf, 0, 1024);
		fgets(buf, 1024, fd);
		
		
	}
}

void error_handler(int err)
{
	if (err == SER_ERR)
	{
		printf("server error!\n");
		exit(0);
	}

	else
		return;
}

int sub_deamon(int sfd)
{
	int err;
	socks_t *socks;

	//accept socks request
	err = socks_request(sfd, &socks);

	error_handler(err);

	err = firewall(sfd, &socks);

	return 0;
}
