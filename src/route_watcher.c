#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include "common.h"
//#include "cfuhash.h"

//void *request_name(void *arguments);
//void request_name(char *dst_addr, char *next_hop_addr);
void copy_file_content(char *from_path, char *to_path);
void remove_name(char *addr);
void print_hash_table(cfuhash_table_t *table);
/* void print_pnrs();
void print_timers(); */
void register_nreq(char *dst_addr, char *next_hop, char *requester_addr);

char *msg;
struct request_args {
    char *next_hop;
    char *dst_addr;
};

void *watch_routes() {
    FILE *in;
    extern FILE *popen();
    char buf[IP_MON_ROUTE_LEN];
    int addr_size;

    if (!(in = popen("ip monitor route", "r"))) {
        asprintf(&msg, "[ ERROR ] %s\n", "ip monitor route");
        log_msg(msg, own_addr);
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
                asprintf(&msg, "[ROUTE+] New route added: %s", route_copy);
                log_msg(msg, own_addr);
                
                name = get_name_by_addr(dst_addr);
                if (!name) { // name for dst_addr is not known
                    if (!cfuhash_get(pnrs, dst_addr)) {
                        //pthread_t tid;
                        struct request_args args;
                        args.next_hop = next_hop;
                        args.dst_addr = dst_addr;

                        // register the request in PNRs table
                        register_nreq(dst_addr, next_hop, NULL);
                        //print_hash_table(pnrs);
                        
                        // request name from next hop in new thread
                        //pthread_create(&tid, NULL, request_name, (void*) &args);
                        request_name(dst_addr, next_hop);
                    } else { // name for dst_addr is not known but has already been requested
                        asprintf(&msg, "[ROUTE] A PNR for host %s already exists\n", dst_addr);
                        log_msg(msg, own_addr);
                        request_name(dst_addr, next_hop);
                    }
                } else { // name for dst_addr is cached
                    asprintf(&msg, "[ROUTE] A name for host %s is already cached: %s\n", dst_addr, name);
                    log_msg(msg, own_addr);
                }
            } else { // route removal detected
                strtok(buf, " ");
                dst_addr = strtok(NULL, " ");
                asprintf(&msg, "[ROUTE-] Route removed: %s", route_copy);
                log_msg(msg, own_addr);

                if (cfuhash_get(pnrs, dst_addr)) // remove pnr if it exists
                    cfuhash_delete(pnrs, dst_addr);
                if (!get_name_by_addr(dst_addr)) // remove name if cached
                    remove_name(dst_addr);
            }
        }
    }
    pclose(in);
}

void remove_name(char *addr) {
    FILE *in, *out;
    extern FILE *popen();
    char line[100], line_copy[100];
    char *cached_addr;
    char *hostname = get_hostname();
    char *hosts_file_path, *new_hosts_file_path;

    asprintf(&hosts_file_path, "%s/hosts-%s", HOSTS_FILES_PATH, hostname);    
    asprintf(&new_hosts_file_path, "%s/hosts-%s.aux", HOSTS_FILES_PATH, hostname);

    if (!(in = popen("cat /etc/hosts", "r"))) {
        asprintf(&msg, "%s", "[ ERROR ] removing name: could read hosts file\n");
        log_msg(msg, own_addr);
    }
    
    if (!(out = fopen(new_hosts_file_path, "w"))) {
        asprintf(&msg, "%s", "[ ERROR ] removing name: could not create new hosts file\n");
        log_msg(msg, own_addr);
    }

    while (fgets(line, sizeof(line), in) != NULL) {
        strcpy(line_copy, line);
        cached_addr = strtok(line, "\t");

        if (strcmp(cached_addr, addr) != 0) {
            fprintf(out, "%s", line_copy);
        }
    }

    fclose(in);
    fclose(out);

    copy_file_content(new_hosts_file_path, hosts_file_path);

    // TODO: check if /etc/hosts is being written to
    //char *cmd;
    //asprintf(&cmd, "mv -f %s %s", new_hosts_file_path, hosts_file_path);
    //system(cmd);

    //free(cmd);
    free(hosts_file_path);
    free(new_hosts_file_path);

    asprintf(&msg, "[NAME-] Removed name for host %s\n", addr);
    log_msg(msg, own_addr);
}

// copy file content (overwriting)
void copy_file_content(char *from_path, char *to_path) {
    FILE *in, *out;
    char line[100];

    if (!(in = fopen(from_path, "r"))) {
        asprintf(&msg, "[ ERROR ] Could not open file %s for reading\n", from_path);
        log_msg(msg, own_addr);
    }

    if (!(out = fopen(to_path, "w"))) {
        asprintf(&msg, "[ ERROR ] Could not open file %s for writing\n", to_path);
        log_msg(msg, own_addr);
    }

    while (fgets(line, sizeof(line), in) != NULL) {
        fprintf(out, "%s", line);
    }

    fclose(in);
    fclose(out);
}

/*
If a pending NREQ for dst_addr already exists in the PNRs table,
add requester_addr to the associated requesters list.
Otherwise, create a new pair in PNRs table assoviating dst_addr
with a new list of requesters that includes requester_addr.
*/
void register_nreq(char *dst_addr, char *next_hop, char *requester_addr) {
    node_t *requesters_list;

    if ((requesters_list = cfuhash_get(pnrs, dst_addr)) != NULL) {
        if (exists_in_list(requesters_list, requester_addr) == 0) {
            asprintf(&msg, "»»»»»»» checking list %s\n", "done");
            log_msg(msg, own_addr);
            push(requesters_list, requester_addr);
            //printf("*** Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
            asprintf(&msg, "[PNR++] Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
            log_msg(msg, own_addr);
        } else {
            //printf("*** Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
            asprintf(&msg, "[PNR] %s already is a requester for %s in PNRs table\n", requester_addr, dst_addr);
            log_msg(msg, own_addr);
        }
    } else {
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
        //printf("*** Started timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        asprintf(&msg, "[TIMER+] Started timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        log_msg(msg, own_addr);
        //printf("%zu timer entries\n", cfuhash_num_entries(timers));
        //print_hash_table(timers);

        if (requester_addr != NULL) { // this happens when a node is asked for a name of a dst for which it has a route but has not had time to register an NREQ yet
            asprintf(&msg, "[PNR_WARNING] New PNR for %s (from %s) created but NREQ was not sent!\n", dst_addr, requester_addr);
            log_msg(msg, own_addr);
        }
        //printf("*** Added %s as a requester for %s in PNRs table\n", requester_addr, dst_addr);
        asprintf(&msg, "[PNR+] Created new PNR for %s with %s as 1st requester\n", dst_addr, requester_addr);
        log_msg(msg, own_addr);
    }
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
