#ifndef __SOCKS__
#define __SOCKS__

#define SER_ERR -1
#define AUTH_ERR -2

typedef struct
{
	short vn;
	short cd;
	unsigned int port;
	char ip[20];
	char user_id[20];
	char domain[50];
} socks_t;

int sub_deamon(int sfd);

#endif
