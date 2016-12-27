#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"
#include "host.h"
#include "socks.h"

void html_head()
{
	printf("Content-Type: text/html\n\n");
	fflush(stdout);
	printf("<html>\n");
	fflush(stdout);
	printf("<head>\n");
	fflush(stdout);
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
	fflush(stdout);
	printf("<title>Network Programming Homework 3</title>\n");
	fflush(stdout);
	printf("</head>\n");
	fflush(stdout);
	printf("<body bgcolor=#336699>\n");
	fflush(stdout);
	printf("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
	fflush(stdout);
	printf("<table width=\"800\" border=\"1\">\n");
	fflush(stdout);
}

int main()
{
	setenv("QUERY_STRING", "h1=127.0.0.1&p1=13420&f1=t1.txt&sh1=127.0.0.1&sp1=3421&h2=&p2=&f2=&sh2=&sp2=&h3=&p3=&f3=&sh3=&sp3=&h4=&p4=&f4=&sh4=&sp4=&h5=&p5=&f5=&sh5=&sp5=", 1);

	char *query = getenv("QUERY_STRING");
	char **s_array;
	size_t counter;
	int nhost = 0;
	
	printf("q:%s\n", query);
	chdir("/u/other/2017_1/104522052/public_html");
	split(&s_array, query, "&", &counter);
	//printf("%d\n", counter);
	printf("arr1:%s\n", s_array[0]);

	host_t *host[6];

	//init host
	int i = 0;
	for (;i < 6; i++)
		host[i] = NULL;

	for (i = 1; i < 6; i++)
	{
		host_t *temp = malloc(sizeof(host_t));
		memset(temp, 0, sizeof(host_t));

		int index = i - 1;
		if (strlen(s_array[index*3]) > 3 && strlen(s_array[index*3 + 1]) > 3 && strlen(s_array[index*3 + 2]) > 3)
		{
			strcpy(temp->ip, &s_array[index*5][3]);
			temp->port = atoi(&s_array[index*5+1][3]);
			strcpy(temp->file, &s_array[index*5+2][3]);
			temp->file_fd = fopen(temp->file, "r");
			strcpy(temp->sock_ip, &s_array[index*5+3][4]);
			temp->sock_port = atoi(&s_array[index*5+4][4]);
			temp->connected = 0;
			host[i] = temp;
			nhost++;
		}
	
		else
			free(temp);

	}

	//open non-blocking socket 

	fd_set afds, rfds;
	int nfds = getdtablesize();
	FD_ZERO(&afds);

	for (i = 1; i < 6; i++)
	{
		if (host[i] != NULL)
		{
			//connect to sock server
			int s = socket(AF_INET , SOCK_STREAM , 0);
			struct sockaddr_in server;

			memset(&server, 0, sizeof(server));
			server.sin_addr.s_addr = inet_addr(host[i]->sock_ip);
			server.sin_family = AF_INET;
			server.sin_port = htons(host[i]->sock_port);

			if (connect(s, (struct sockaddr *)&server , sizeof(server)) < 0)
			{
				printf("<h2>connect sock server error.</h2>\n");
				return -1;
			}
			
			//make up packet
			char *send_buf;
			send_buf = make_up_packet(host[i]);

			for (int J = 0; J < 9; J++)
				printf("%x\n", send_buf[J]);
			
			//send socks
			write(s, send_buf, 9);

			//recv socks
			get_socks(s, host[i]);

			if (host[i]->cd == 90)
				host[i]->connected = 1;

			int flag = fcntl(s, F_GETFL, 0);
			fcntl(s, F_SETFL, flag | O_NONBLOCK);

			//set to host
			host[i]->sock_fd = s;
			host[i]->server = server;

			//set afds
			FD_SET(s, &afds);
		}
	}

	//html

	html_head();

	printf("<tr>\n");
	for (i = 1; i < 6; i++)
	{
		if (host[i] != NULL)
		{
			printf("<td>");
			fflush(stdout);
			printf("%s", host[i]->ip);
			fflush(stdout);
			printf("</td>");
			fflush(stdout);
		}
	}
	printf("</tr>\n");
	fflush(stdout);

	printf("<tr>\n");
	fflush(stdout);
	for (i = 1; i < 6; i++)
	{
		if (host[i] != NULL)
		{
			printf("<td");
			fflush(stdout);
			printf(" valign=\"top\" id=\"m%d\"", i-1);
			fflush(stdout);
			printf("></td>");
			fflush(stdout);
		}
	}
	printf("</tr>\n");
	fflush(stdout);
	printf("</table>\n");
	fflush(stdout);

	i = 0;
	for (; i < 6; i++)
	{
		if (host[i] != NULL)
		{
			printf("%s\n", host[i]->ip);
			printf("%d\n", host[i]->port);
			printf("%s\n", host[i]->file);
			printf("%d\n", host[i]->sock_fd);
			printf("%s\n", host[i]->sock_ip);
			printf("%d\n", host[i]->sock_port);
		}
	}

/*
	int complete = 0;
	while (1)
	{
		memcpy(&rfds, &afds, sizeof(afds));
		select(nfds, &rfds, NULL, NULL, NULL);

		for (i = 1; i < 6; i++)
		{
			if (host[i] != NULL && host[i]->connected == 0)
			{
				if (connect(host[i]->sock_fd, (struct sockaddr *)&(host[i]->server), sizeof(host[i]->server)) < 0)
				{
					if(errno == EISCONN)
						host[i]->connected = 1;	
				}
			}

			if (host[i] != NULL && FD_ISSET(host[i]->sock_fd, &rfds))
			{
				//recv msg, end with /r/n
				char msg[10010];
				memset(msg, 0, 10010);
				int counter = 0;
				char buf[2];
				memset(buf, 0, 2);
				while (read(host[i]->sock_fd, buf, 1) > 0)
				{
					if (buf[0] == '\r')
						continue;
					msg[counter++] = buf[0];
					if (buf[0] == '\n')
						break;
				}
				

				if (counter == 0)
					continue;

				//output msg
				//must print cmd
				if (strncmp(msg, "% ", 2) == 0)
				{
					//read cmd file
					char next_cmd[10010];
					memset(next_cmd, 0, 10010);
					fgets(next_cmd, 10010, host[i]->file_fd);
					write(host[i]->sock_fd, next_cmd, strlen(next_cmd));
					if (strncmp(next_cmd, "exit", 4) == 0)
					{
						complete++;
						FD_CLR(host[i]->sock_fd, &afds);
						close(host[i]->sock_fd);
						free(host[i]);
						host[i] = NULL;
					}
					replace_html(next_cmd);
					printf("<script>document.all['m%d'].innerHTML += \"%% <b> %s</b>\";</script>\n", i-1, next_cmd);
					fflush(stdout);
				}
				
				else
				{
					replace_html(msg);
					printf("<script>document.all['m%d'].innerHTML += \"%s\";</script>\n", i-1, msg);
					fflush(stdout);
				}
			}
		}

		if (complete == nhost)
			break;
	}
*/	

	
	return 0;
}
