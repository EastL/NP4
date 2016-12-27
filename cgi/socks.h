#ifndef __SOCKS__
#define __SOCKS__
#include "util.h"

char *make_up_packet(host_t *host);
void get_socks(int s, host_t *host);

#endif
