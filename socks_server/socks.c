#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include "socks.h"
#include "util.h"

int socks_request(int sfd, socks_t **socks)
{
	unsigned char buf[1024];
	memset(buf, 0, 1024);
	if (read(sfd, buf, 1024) < 0)
		return SER_ERR;

	socks_t *temp = malloc(sizeof(socks_t));
	temp->vn = buf[0];
	temp->cd = buf[1];
	temp->port = buf[2] * 256 + buf[3];

	char IP[20];
	memset(IP, 0, 20);

	temp->cport[0] = buf[2];
	temp->cport[1] = buf[3];
	temp->cip[0] = buf[4];
	temp->cip[1] = buf[5];
	temp->cip[2] = buf[6];
	temp->cip[3] = buf[7];
	
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

void socks_reply(int sfd, unsigned char cd, socks_t *socks)
{
	char buf[8];
	memset(buf, 0, 8);

	char null = '\0';

	sprintf(buf, "%c%c%c%c%c%c%c%c", null, cd, socks->cport[0],  socks->cport[1], socks->cip[0], socks->cip[1], socks->cip[2], socks->cip[3]);

	write(sfd, buf, 8);
}

int proxy(int src, int dst)
{
	char buf[MAXBUF];
	int src_close, dst_close;
	int recv_len;
	int len;
	int nfds = getdtablesize();
	fd_set rfds, afds;

	FD_ZERO(&afds);
	FD_SET(src, &afds);
	FD_SET(dst, &afds);
	
	while (1)
	{
		memcpy(&rfds, &afds, sizeof(rfds));
		if(select(nfds, &rfds, NULL, NULL, NULL) < 0)
			return SER_ERR;

		memset(buf, 0, MAXBUF);
		if (FD_ISSET(src, &rfds))
		{
			recv_len = read(src, buf, MAXBUF);
			if (recv_len > 0)
				write(dst, buf, recv_len);

			else if (recv_len == 0)
				FD_CLR(src, &afds);

			else
				return SER_ERR;
		}

		memset(buf, 0, MAXBUF);
		if (FD_ISSET(dst, &rfds))
		{
			recv_len = read(dst, buf, MAXBUF);
			if (recv_len > 0)
				write(src, buf, recv_len);

			else if (recv_len == 0)
				FD_CLR(dst, &afds);

			else
				return SER_ERR;
		}
	}
}

int firewall(int sfd, socks_t **socks)
{
	FILE *fd = fopen("socks.conf", "r");
	int check = 0;
	char buf[50];
	int ip[4];

	while(1)
	{
		memset(buf, 0, 50);
		fgets(buf, 50, fd);

		if (strlen(buf) == 0)
			break;

		if (buf[0] == '*')
		{
			check = 1;
			break;
		}

		buf[strlen(buf)-1] = '\0';

		printf("%s\n", buf);
		printf("%s\n", ((*socks)->ip));
		check = regular_match((*socks)->ip, buf);
		if (check == 1)
			break;
	}

	fclose(fd);

	return check;
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
	int err, pass;
	socks_t *socks;

	//accept socks request
	err = socks_request(sfd, &socks);

	error_handler(err);

	if (firewall(sfd, &socks) == 1)
	{
		printf("pass firewall\n");
		if (socks->cd == CONNECT)
		{
			//create client socket
			int client_s = socket(AF_INET, SOCK_STREAM, 0);

			//create server socket struct
			struct sockaddr_in server;
			memset(&server, 0, sizeof(server));

			server.sin_addr.s_addr = inet_addr(socks->ip);
			server.sin_family = AF_INET;
			server.sin_port = htons(socks->port);

			//connect to server
			int err = 0;
			err = connect(client_s, (struct sockaddr *)&server, sizeof(server));
			if (err < 0)
			{
				printf("connect error!\n");
				socks_reply(sfd, (unsigned char)FAILED, socks);
				return -1;
			}

			printf("connect successful\n");
			socks_reply(sfd, (unsigned char)GRANTED, socks);

			proxy(sfd, client_s);
			error_handler(err);

			close(sfd);
			close(client_s);
		}
	}

	return 0;
}
