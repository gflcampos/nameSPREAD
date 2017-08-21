#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"
#include "cfuhash.h"

void *request_name();
void add_to_pending_nreqs(char *dst_addr, char *requester_addr);
char *msg;

struct request_args {
    char *next_hop;
    char *dst_addr;
};

void *watch_routes(void *pnrs) {
    //cfuhash_table_t *pnrs = (cfuhash_table_t *)hast_table;
    sleep(5);
    cfuhash_pretty_print(pnrs, stdout);
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
            if (!strstr(buf, "Deleted")) { // route was added
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

                    // add request to PNRs
                    add_to_pending_nreqs(dst_addr, "");

                    // request name from next hop
                    pthread_create(&tid, NULL, request_name, (void*) &args);
                    asprintf(&msg, "*** Requesting name for %s...\n", dst_addr);
                    log_msg(msg, own_addr);

                } else { // name for dst_addr is cached
                    asprintf(&msg, "*** A name for host %s is already cached: %s\n", dst_addr, name);
                    log_msg(msg, own_addr);
                }
            } else { //route was deleted
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
    char *dst_addr = args->dst_addr, *next_hop_addr = (char*) args->next_hop;
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
    memset(res_buf,'\0', MAX_NAME_LEN); // clear the buffer???????
    recvfrom(s, res_buf, MAX_NAME_LEN, 0, (struct sockaddr *) &next_hop, &addr_size);

    asprintf(&msg, "*** Received name for host %s: %s\n", dst_addr, res_buf);
    log_msg(msg, own_addr);

    // cache received name
    FILE *out = fopen("/etc/hosts", "a");
    fprintf(out, "%s\t%s\n", dst_addr, res_buf);
    fclose(out);
    puts("HELLO5");

    // TODO: RESPOND TO PNRs <<<<<<<<<<<<<<<<<<<<<<<<

}

void add_to_pending_nreqs(char *dst_addr, char *requester_addr) {
    ;
}