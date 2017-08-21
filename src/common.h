#ifndef GLOBAL_H
#define GLOBAL_H

#define NREQ_RESPONDER_PORT 7891
#define IP_MON_ROUTE_LEN 1024
#define INET_ADDRSTRLEN 16
#define MAX_NAME_LEN 32

extern char *own_addr;

char *get_hostname();
char *get_name_by_addr(char *address);
void log_msg(char *message, char *addr);
char *get_timestamp();

#endif
