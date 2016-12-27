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
#include "host.h"
#include "util.h"

char *make_up_packet(host_t *host)
{
	char **ip_arr;
	size_t ip_size;
	unsigned char vn = (unsigned char)4;
	unsigned char cd = (unsigned char)1;
	unsigned char port[2];
	unsigned char ip[4];
	int i;

	port[0] = (unsigned char)(host->port / 256);
	port[1] = (unsigned char)(host->port % 256);

	split(&ip_arr, host->ip, ".", &ip_size);
	if (ip_size != 4)
		return NULL;

	for (i = 0; i < 4; i++)
		ip[i] = (unsigned char)atoi(ip_arr[i]);	

	char *ret = malloc(9);
	memset(ret, 0, 9);
	
	sprintf(ret, "%c%c%c%c%c%c%c%c", vn, cd, port[0], port[1], ip[0], ip[1], ip[2], ip[3]);

	return ret;
}

void get_socks(int s, host_t *host)
{
	char buf[8];
	memset(buf, 0, 8);

	read(s, buf, 8);

	host->cd = buf[1];
}
