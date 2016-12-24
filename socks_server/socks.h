#ifndef __SOCKS__
#define __SOCKS__

#define SER_ERR -1
#define AUTH_ERR -2
#define CONNECT 1
#define BIND 2
#define GRANTED 90
#define FAILED 91

typedef struct
{
	short vn;
	short cd;
	unsigned int port;
	char ip[20];
	char user_id[20];
	char domain[50];
	char cport[2];
	char cip[4];
} socks_t;

int sub_deamon(int sfd);

#endif
