#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include "common.h"
//#include "cfuhash.h"

void *request_name();
void print_hash_table(cfuhash_table_t *table);
/* void print_pnrs();
void print_timers(); */
void register_nreq(char *dst_addr, char *requester_addr);

char *msg;
struct request_args {
    char *next_hop;
    char *dst_addr;
};

void *watch_routes() {
    //sleep(5);
    //cfuhash_pretty_print(pnrs, stdout);
    FILE *in;
    extern FILE *popen();
    char buf[IP_MON_ROUTE_LEN];
    int addr_size;

    if (!(in = popen("ip monitor route", "r"))) {
        exit(1);
    }

    while (fgets(buf, sizeof(buf), in)!=NULL) {
        char route_copy[sizeof buf];
        char *name, *dst_addr, *next_hop;
        strcpy(route_copy, buf);

        if (strstr(buf, "via")) {
            if (!strstr(buf, "Deleted")) { // new route detected
                dst_addr = strtok(buf, " ");
                strtok(NULL, " "); // skip 'via'
                next_hop = strtok(NULL, " ");
                asprintf(&msg, "*** New route added: %s", route_copy);
                log_msg(msg, own_addr);
                
                name = get_name_by_addr(dst_addr);
                if (!name) { // name for dst_addr is not known
                    pthread_t tid;
                    struct request_args args;
                    args.next_hop = next_hop;
                    args.dst_addr = dst_addr;

                    // register the request in PNRs table
                    register_nreq(dst_addr, NULL);
                    print_hash_table(pnrs);
                    
                    // request name from next hop in new thread
                    pthread_create(&tid, NULL, request_name, (void*) &args);
                    asprintf(&msg, "*** Requesting name for %s...\n", dst_addr);
                    log_msg(msg, own_addr);

                } else { // name for dst_addr is cached
                    asprintf(&msg, "*** A name for host %s is already cached: %s\n", dst_addr, name);
                    log_msg(msg, own_addr);
                }
            } else { // route deletion detected
                strtok(buf, " ");
                dst_addr = strtok(NULL, " ");
                asprintf(&msg, "*** Route removed: %s", route_copy);
                log_msg(msg, own_addr);
            }
        }
    }
    pclose(in);
}

void *request_name(void *arguments) {
    struct request_args *args = (struct request_args *)arguments;
    char *dst_addr = args->dst_addr, *next_hop_addr = "127.0.0.1";//(char*) args->next_hop;
    int s;
    char res_buf[MAX_NAME_LEN];
    struct sockaddr_in next_hop;
    socklen_t addr_size;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    memset((char *)&next_hop, 0, sizeof(next_hop));
    next_hop.sin_family = AF_INET;
    next_hop.sin_port = htons(NREQ_RESPONDER_PORT);
    if (inet_aton(next_hop_addr, &next_hop.sin_addr) == 0) {
        perror("Invalid next hop address.");
    }

    addr_size = sizeof next_hop;
    
    sendto(s, dst_addr, INET_ADDRSTRLEN, 0, (struct sockaddr *) &next_hop, addr_size);
    memset(res_buf,'\0', MAX_NAME_LEN); // clear the buffer (?)
    recvfrom(s, res_buf, MAX_NAME_LEN, 0, (struct sockaddr *) &next_hop, &addr_size);

    asprintf(&msg, "*** Received name for host %s: %s\n", dst_addr, res_buf);
    log_msg(msg, own_addr);

    // cache received name
    FILE *out = fopen("/etc/hosts", "a");
    fprintf(out, "%s\t%s\n", dst_addr, res_buf);
    fclose(out);
    puts("HELLO5");

    // TODO: send name to interested requesters, if any <<<<<<<<<<<<<<<<<<<<<<<<
}

/*
If a pending NREQ for dst_addr already exists in the PNRs table,
add requester_addr to the associated requesters list.
Otherwise, create a new pair in PNRs table assoviating dst_addr
with a new list of requesters that includes requester_addr.
*/
void register_nreq(char *dst_addr, char *requester_addr) {
    node_t *requesters_list;

    if ((requesters_list = cfuhash_get(pnrs, dst_addr)) != NULL) {
        push(requesters_list, requester_addr);
    } else {

        if (requester_addr != NULL) {
            asprintf(&msg, "[FIX THIS] ### New PNR for %s (from %s) created but NREQ was not sent!\n", dst_addr, requester_addr);
            log_msg(msg, own_addr);
        }

        // create new list of requesters for dst_addr
        requesters_list = new_linked_list(requester_addr);
        // associate dst_addr to requesters_list in PNRs table
        cfuhash_put(pnrs, dst_addr, requesters_list);

        // create new timer
        timer_t timer_obj;
        int *timer_id_index = next_free_timer_id();
        char timer_id_str[12];
        //printf("~~~new timer_id=%d\n", *timer_id_index);
        make_timer(&timer_obj, &(timer_ids[*timer_id_index - 1]), NREQ_TIMEOUT_SECS);
        
        // associate timer_id (as string) to dst_addr in timers table
        sprintf(timer_id_str, "%d", *timer_id_index);
        cfuhash_put(timers, timer_id_str, strdup(dst_addr));
        printf("*** Started timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        asprintf(&msg, "*** Started timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        log_msg(msg, own_addr);
        //printf("%zu timer entries\n", cfuhash_num_entries(timers));
        print_hash_table(timers);
    }
    printf("*** Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
    asprintf(&msg, "*** Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
    log_msg(msg, own_addr);
}

void print_hash_table(cfuhash_table_t *table) {
    size_t num_keys = 0;
    void **keys = NULL;
    char *key;
    size_t i = 0;

    keys = cfuhash_keys(table, &num_keys, 1);

    puts("{");
    for (i = 0; i < num_keys; i++) {
        key = keys[i];
        printf("\t%s -> ", key);
        if (table == pnrs)
            print_list(cfuhash_get(table, key));
        else if (table == timers)
            printf("%s", (char*)cfuhash_get(table, key));
        printf("%s", "\n");
	}
    puts("}\n");
}

/* void print_pnrs() {
    size_t num_keys = 0;
    void **keys = NULL;
    char *key;
    size_t i = 0;

    keys = cfuhash_keys(pnrs, &num_keys, 1);

    puts("{");
    for (i = 0; i < num_keys; i++) {
        key = keys[i];
        printf("\t%s -> ", key);
        print_list(cfuhash_get(pnrs, key));
        printf("%s", "\n");
	}
    puts("}");    
}

void print_timers() {
    size_t num_keys = 0;
    void **keys = NULL;
    char *key;
    size_t i = 0;

    keys = cfuhash_keys(timers, &num_keys, 1);

    puts("{");
    for (i = 0; i < num_keys; i++) {
        key = keys[i];
        printf("\t%s -> ", key);
        printf("%s", (char*)cfuhash_get(timers, key));
        printf("%s", "\n");
	}
    puts("}");
} */
