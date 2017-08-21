#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "nreq_responder.c"
#include "route_watcher.c"
#include "common.h"
#include "logger.c"

#include "libcfu/cfustring.c"
#include "libcfu/cfuhash.c"
#include "linked_list.c"

char *own_addr;

int main(int argc, char *argv[]) {
    own_addr = argv[1];
    pthread_t tid1, tid2;

    cfuhash_table_t *pnrs = cfuhash_new(30);
	size_t i;
	cfuhash_set_flag(pnrs, CFUHASH_FROZEN_UNTIL_GROWS);
    for (i = 1; i <= 10; i++) {
        char key[16];
        node_t *value = new_linked_list("10.0.0.7");
        push(value, "10.0.0.42");
        sprintf(key, "10.0.0.%zu", i);
        cfuhash_put(pnrs, key, value);
    }
    
    pthread_create(&tid1, NULL, listen_for_nreqs, (void *) pnrs);
    pthread_create(&tid2, NULL, watch_routes, (void *) pnrs);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
}

char *get_hostname() {
    FILE *file_stream;
    char buff[32];
    char *hostname;

    if (!(file_stream = fopen("/etc/hostname", "r")))
        perror("### Could not open /etc/hostname");

    hostname = fgets(buff, sizeof(buff), file_stream);
    fclose(file_stream);

    return strdup(hostname);
}

char *get_name_by_addr(char *address) {
    FILE *in;
    extern FILE *popen();
    char buff[512];
    //char *name;
    char *name;
    if (!(in = popen("cat /etc/hosts", "r"))) {
        exit(1);
    }

    while (fgets(buff, sizeof(buff), in)!=NULL) {
        //asprintf(&msg, "1%s\n", buff);
        //log_msg(msg, own_addr);
        if (strstr(buff, address)) {
            strtok(buff, "\t");
            //asprintf(&msg, "2%s\n", buff);
            //log_msg(msg, own_addr);
            name = strtok(NULL,"\t");
            //asprintf(&msg, "3%s\n", name);
            //log_msg(msg, own_addr);
            name[strlen(name) - 1] = '\0'; // remove trailing newline
            //asprintf(&msg, "4%s\n", name);
            //log_msg(msg, own_addr);
            return strdup(name);
        }
    }

    return NULL;
}