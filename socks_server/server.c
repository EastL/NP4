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

#define PORT 3421

int main()
{
	//set server socket
	int sockfd;
	struct sockaddr_in mysocket;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&mysocket, sizeof(mysocket));
	mysocket.sin_family = AF_INET;
	mysocket.sin_addr.s_addr = INADDR_ANY;
	mysocket.sin_port = htons(PORT);

	int sock_opt = 1 ;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&sock_opt,sizeof(sock_opt));
	bind(sockfd, (struct sockaddr*) &mysocket, sizeof(mysocket));
	listen(sockfd, 20);

	printf("listening port : %d\n", PORT);

	//set accept client socket
	int clientfd;
	struct sockaddr_in client_socket;
	int cl_len = sizeof(client_socket);


	while(1)
	{
		if ((clientfd = accept(sockfd, (struct sockaddr*)&client_socket, &cl_len)) < 0)
		{
			printf("Accept error!");
			continue;
		}

		printf("Accept client fd : %d\n", clientfd);

		int chpid = fork();

		if (chpid == 0)
		{
			sub_deamon(clientfd);
			close(clientfd);
		}
		
		else
			close(clientfd);
	}

	return 0;
}
