#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "libcfu/cfustring.c"
#include "libcfu/cfuhash.c"
#include "linked_list.c"
#include "timer.c"
#include "logger.c"
#include "nreq_responder.c"
#include "route_watcher.c"

#include "common.h"

int total_nreqs = 0;
int total_nreps_reveived = 0;
int total_nreps = 0;
int total_cached_names = 0;
//int n_acks = 0;
char *own_addr;
cfuhash_table_t *pnrs, *timers;
//node_t *acks;
int timer_ids[N_TIMERS] = {0};
int n_timers = 0;

void *log_info();

int main(int argc, char *argv[]) {
    own_addr = argv[1];
    //acks = new_linked_list("");
    pthread_t tid1, tid2, tid3;

    pnrs = cfuhash_new(30);
    timers = cfuhash_new(30);
    cfuhash_set_flag(pnrs, CFUHASH_FROZEN_UNTIL_GROWS);
    cfuhash_set_flag(timers, CFUHASH_FROZEN_UNTIL_GROWS);
    
    pthread_create(&tid1, NULL, listen_for_nreqs, NULL);
    pthread_create(&tid2, NULL, watch_routes, NULL);
    pthread_create(&tid3, NULL, log_info, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);    
}

void *log_info() {
    while(1) {
        sleep(10);
        asprintf(&msg, "[INFO] NREQs_sent: %d NREQs_received: %d NREPs_sent: %d Names_cached: %d\n", total_nreqs, total_nreps_reveived, total_nreps, total_cached_names);
        log_msg(msg, own_addr);
    }
}


char *get_hostname() {
    FILE *file_stream;
    char buff[32];
    char *hostname;

    if (!(file_stream = fopen("/etc/hostname", "r"))) {
        asprintf(&msg, "[ ERROR ] %s\n", "Could not open /etc/hostname");
        log_msg(msg, own_addr);
        perror("Could not open /etc/hostname");
    }

    hostname = fgets(buff, sizeof(buff), file_stream);
    hostname[strlen(hostname) - 1] = '\0'; // remove trailing newline
    fclose(file_stream);
    
    return strdup(hostname);
}

char *get_name_by_addr(char *addr) { //10.0.0.10
    FILE *in;
    extern FILE *popen();
    char buff[512];
    char *cached_addr, *cached_name;

    while (!(in = popen("cat /etc/hosts", "r"))) {
        asprintf(&msg, "[WAIT] %s\n", "Trying to read from /etc/hosts...");
        log_msg(msg, own_addr);
        sleep(1);
        //exit(1);
    }

    while (fgets(buff, sizeof(buff), in)!=NULL) {
        //asprintf(&msg, "1%s\n", buff);
        //log_msg(msg, own_addr);
        cached_addr = strtok(buff, "\t");

        if (strcmp(cached_addr, addr) == 0) {
            //asprintf(&msg, "2%s\n", buff);
            //log_msg(msg, own_addr);
            cached_name = strtok(NULL,"\t");
            //asprintf(&msg, "3%s\n", cached_name);
            //log_msg(msg, own_addr);
            cached_name[strlen(cached_name) - 1] = '\0'; // remove trailing newline
            //asprintf(&msg, "4%s\n", cached_name);
            //log_msg(msg, own_addr);
            pclose(in);
            return strdup(cached_name);
        }
    }
    pclose(in);
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

//void *request_name(void *arguments) {
 void request_name(char *dst_addr, char *next_hop_addr) {
    //struct request_args *args = (struct request_args *)arguments;
    //char *dst_addr = args->dst_addr, *next_hop_addr = (char*) args->next_hop;//"127.0.0.1";
    int s;
    char send_buf[NREQ_MAX_LEN];//, recv_buf[NREP_MAX_LEN];
    struct sockaddr_in next_hop;
    socklen_t addr_size;
    
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    memset((char *) &next_hop, 0, sizeof(next_hop));
    next_hop.sin_family = AF_INET;
    next_hop.sin_port = htons(NREQ_RESPONDER_PORT);
    if (inet_aton(next_hop_addr, &next_hop.sin_addr) == 0) {
        asprintf(&msg, "[ ERROR ] Invalid next hop address: %s\n", next_hop_addr);
        log_msg(msg, own_addr);
        perror("Invalid next hop address");
    }
    
    addr_size = sizeof next_hop;
    sprintf(send_buf, "NREQ %s", dst_addr);
    sendto(s, send_buf, strlen(send_buf) + 1, 0, (struct sockaddr *) &next_hop, addr_size);
    
    total_nreqs++;
    asprintf(&msg, "[NREQ->] Requesting name for %s from %s...\n", dst_addr, next_hop_addr);
    log_msg(msg, own_addr);
    /* // TODO: maybe stop here and receive response on NREQ responder
    memset(recv_buf,'\0', NREP_MAX_LEN); // clear the buffer (?)
    recvfrom(s, recv_buf, NREP_MAX_LEN, 0, (struct sockaddr *) &next_hop, &addr_size);
    
    asprintf(&msg, "*** Received name for host %s: %s\n", dst_addr, recv_buf);
    log_msg(msg, own_addr);
    
    // cache received name
    FILE *out = fopen("/etc/hosts", "a");
    fprintf(out, "%s\t%s\n", dst_addr, recv_buf);
    fclose(out);
    
    // TODO: send name to interested requesters, if any */
}
