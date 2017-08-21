#include "cfuhash.h"
//#include <time.h>

#ifndef COMMON
#define COMMON

#define NREQ_RESPONDER_PORT 7891
#define IP_MON_ROUTE_LEN 1024
#define INET_ADDRSTRLEN 16
#define MAX_NAME_LEN 32
#define NRWQ_TIMEOUT_SECS 3

extern char *own_addr;
extern cfuhash_table_t *pnrs, *timers;

char *get_hostname();
char *get_name_by_addr(char *address);
void log_msg(char *message, char *addr);
char *get_timestamp();

// timer
char *makeTimer(timer_t *timerID, int expire);

// linked list
typedef struct node {
    char* val;
    struct node *next;
} node_t;

void print_list(node_t *head);
node_t *new_linked_list(char *val);
void push(node_t * head, char *val);
char *pop(node_t ** head);
node_t *new_linked_list(char *val);

#endif
