#include "cfuhash.h"
//#include <time.h>

#ifndef COMMON
#define COMMON

#define NREQ_RESPONDER_PORT 7891
#define IP_MON_ROUTE_LEN 1024
#define ADDR_MAX_STRLEN 16
#define NAME_MAX_LEN 32
#define NREQ_MAX_LEN 5 + ADDR_MAX_STRLEN  // "NREQ xxx.xxx.xxx.xxx"
#define NREP_MAX_LEN NREQ_MAX_LEN + NAME_MAX_LEN // "NREP xxx.xxx.xxx.xxx name"
#define ACK_MAX_LEN 4 + ADDR_MAX_STRLEN // "ACK xxx.xxx.xxx.xxx"
#define NREQ_TIMEOUT_SECS 10
#define N_TIMERS 100

#define HOSTS_FILES_PATH "/tmp/mininet-wifi/hosts"

// linked list
typedef struct node {
    char *val;
    struct node *next;
} node_t;

extern int total_nreqs;
extern int total_nreps_reveived;
extern int total_nreps;
extern int total_cached_names;
//extern int n_acks;
extern char *own_addr;
extern cfuhash_table_t *pnrs, *timers;
//extern node_t *acks;
extern int timer_ids[100];
extern int n_timers;

// namespread
int *next_free_timer_id(); // move to timer.c?
char *get_hostname();
char *get_name_by_addr(char *address);

// logger
void log_msg(char *message, char *addr);
char *get_timestamp();

// route watcher
void register_nreq(char *dst_addr, char *next_hop, char *requester_addr);
void print_hash_table(cfuhash_table_t *table);
void request_name(char *dst_addr, char *next_hop_addr);

// timer
void make_timer(timer_t *timer_obj, int *timer_id, int timeout_secs);

node_t *new_linked_list(char *val);
void print_list(node_t *head);
void push(node_t * head, char *val);

#endif
