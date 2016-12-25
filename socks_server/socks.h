#ifndef __SOCKS__
#define __SOCKS__

#define SER_ERR -1
#define AUTH_ERR -2
#define CONNECT 1
#define BIND 2
#define GRANTED 90
#define FAILED 91
#define MAXBUF 1000000

typedef struct
{
	short vn;
	short cd;
	unsigned int port;
	unsigned char ip[20];
	unsigned char user_id[20];
	unsigned char domain[50];
	unsigned char cport[2];
	unsigned char cip[4];
} socks_t;

int sub_deamon(int sfd);

#endif
