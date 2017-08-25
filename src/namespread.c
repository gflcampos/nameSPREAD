#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "libcfu/cfustring.c"
#include "libcfu/cfuhash.c"
#include "linked_list.c"
#include "timer.c"
#include "logger.c"
#include "nreq_responder.c"
#include "route_watcher.c"

#include "common.h"

char *own_addr;
cfuhash_table_t *pnrs, *timers;
int timer_ids[N_TIMERS] = {0};
int n_timers = 0;

int main(int argc, char *argv[]) {
    own_addr = argv[1];
    pthread_t tid1, tid2, tid3;

    pnrs = cfuhash_new(30);
    timers = cfuhash_new(30);
    cfuhash_set_flag(pnrs, CFUHASH_FROZEN_UNTIL_GROWS);
    cfuhash_set_flag(timers, CFUHASH_FROZEN_UNTIL_GROWS);
    
    pthread_create(&tid1, NULL, listen_for_nreqs, NULL);
    pthread_create(&tid2, NULL, watch_routes, NULL);
    //pthread_create(&tid3, NULL, log_pnrs, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    //pthread_join(tid3, NULL);    
}

char *get_hostname() {
    FILE *file_stream;
    char buff[32];
    char *hostname;

    if (!(file_stream = fopen("/etc/hostname", "r")))
        perror("### Could not open /etc/hostname");

    hostname = fgets(buff, sizeof(buff), file_stream);
    hostname[strlen(hostname) - 1] = '\0'; // remove trailing newline
    fclose(file_stream);
    
    return strdup(hostname);
}

char *get_name_by_addr(char *address) { //10.0.0.10
    FILE *in;
    extern FILE *popen();
    char buff[512];
    char *cached_addr, *cached_name;
    if (!(in = popen("cat /etc/hosts", "r"))) {
        exit(1);
    }

    while (fgets(buff, sizeof(buff), in)!=NULL) {
        //asprintf(&msg, "1%s\n", buff);
        //log_msg(msg, own_addr);
        cached_addr = strtok(buff, "\t");

        if (strcmp(cached_addr, address) == 0) {
            //asprintf(&msg, "2%s\n", buff);
            //log_msg(msg, own_addr);
            cached_name = strtok(NULL,"\t");
            //asprintf(&msg, "3%s\n", cached_name);
            //log_msg(msg, own_addr);
            cached_name[strlen(cached_name) - 1] = '\0'; // remove trailing newline
            //asprintf(&msg, "4%s\n", cached_name);
            //log_msg(msg, own_addr);
            return strdup(cached_name);
        }
    }

    return NULL;
}

int *next_free_timer_id() {
    int i;

    for (i = 0; i < N_TIMERS; i++) {
        if (timer_ids[i] == 0) {
            timer_ids[i] = i + 1;
            return &(timer_ids[i]);
        }
    }
    return NULL;
}
