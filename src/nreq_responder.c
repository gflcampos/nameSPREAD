#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
//#include "common.h"
//#include "cfuhash.h"

void cache_name(char *dst_addr, char *name);
void respond_to_pnrs(char *dst_addr, char *name);

char *msg;

void *listen_for_nreqs() {
    const char *hostname = get_hostname();
    struct sockaddr_in self, requester;
    int s, addr_size = sizeof requester, conn_fd;
    char recv_buf[NREP_MAX_LEN], send_buf[NREP_MAX_LEN];
    // recv_buf may receive NREQs and NREPs, so it must be as large 
    // as the largest between NREQ_MAX_LEN and NREP_MAX_LEN
    
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset((char *) &self, 0, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(NREQ_RESPONDER_PORT);
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(s, (struct sockaddr *) &self, sizeof(self));
    
    asprintf(&msg, "### Waiting for NREQs/NREPs on port %d...\n", NREQ_RESPONDER_PORT);
    log_msg(msg, own_addr);

    while (1) {
        char *requester_addr;
        char *dst_addr;        
        char *name;

        recvfrom(s, recv_buf, NREP_MAX_LEN, 0, (struct sockaddr *) &requester, &addr_size);
        requester_addr = inet_ntoa(requester.sin_addr);
        
        // check if message is NREQ or NREP
        if (strstr(recv_buf, "NREQ")) {
            strtok(recv_buf, " ");
            dst_addr = strtok(NULL, " ");

            asprintf(&msg, "### NREQ: %s wants name for %s\n", requester_addr, dst_addr);
            log_msg(msg, own_addr);
    
            name = get_name_by_addr(dst_addr);//"127.0.0.1");
    
            if (name) { // is the name cached?
                //strcpy(send_buf, name);
                sprintf(send_buf, "NREP %s %s", dst_addr, name);
                //printf("------%s", send_buf);
                requester.sin_port = htons(NREQ_RESPONDER_PORT); // chage the port so that this NREP is received by the NREQ Responder on the requester
                sendto(s, send_buf, NREP_MAX_LEN, 0, (struct sockaddr*) &requester, addr_size);
    
                asprintf(&msg, "### Sent name %s to %s\n", name, requester_addr);
                log_msg(msg, own_addr);
            } else {
                // if name is not cached, then a PNR for that name must already exist in PNRs table
                // so, add requester_addr to the list of requesters of that PNR
                register_nreq(dst_addr, strdup(requester_addr));
                // when available, the response will be sent. If a timeout occurs, 
                // then a timeout notification will be sent instead (by the timeout handler function)
            }
        } else if (strstr(recv_buf, "NREP")) {
            char nrep_msg_copy[sizeof recv_buf];
            strcpy(nrep_msg_copy, recv_buf);

            strtok(recv_buf, " ");
            dst_addr = strtok(NULL, " ");
            name = strtok(NULL, " ");

            // disable timer for dst_addr
            // (currently, timer still expires and is then removed from timers table)

            asprintf(&msg, "*** NREP: received name '%s' for host %s\n", name, dst_addr);
            log_msg(msg, own_addr);

            // cache the name and send it to the interested requesters
            cache_name(dst_addr, name);
            respond_to_pnrs(dst_addr, nrep_msg_copy);

            // remove entry from PNRs table
            cfuhash_delete(pnrs, dst_addr);
        } else {
            asprintf(&msg, "### Received message with unrecognized format: '%s'\n", recv_buf);
            log_msg(msg, own_addr);
        }

        // clear stdout and buffers
        fflush(stdout);
        memset(recv_buf,'\0', NREP_MAX_LEN);
        memset(send_buf,'\0', NREP_MAX_LEN);
    }
}

void cache_name(char *dst_addr, char *name) {
    if (!get_name_by_addr(dst_addr)) {
        FILE *hosts = fopen("/etc/hosts", "a");
        fprintf(hosts, "%s\t%s\n", dst_addr, name);
        fclose(hosts);
    }
}

void respond_to_pnrs(char *dst_addr, char *nrep_msg) {
    node_t *requesters_list = cfuhash_get(pnrs, dst_addr);
    char *requester_addr;
    struct sockaddr_in requester;
    socklen_t addr_size = sizeof requester;
    int s;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset((char *) &requester, 0, sizeof(requester));
    requester.sin_family = AF_INET;
    requester.sin_port = htons(NREQ_RESPONDER_PORT);

    while (requesters_list != NULL) {
        requester_addr = requesters_list->val;
        requesters_list = requesters_list->next;

        if (requester_addr == NULL) // was local request
            continue;

        if (inet_aton(requester_addr, &requester.sin_addr) == 0) {
            perror("Invalid requester addr address");
        }
        sendto(s, nrep_msg, NREP_MAX_LEN, 0, (struct sockaddr *) &requester, addr_size);
        
        asprintf(&msg, "####### Sent name '%s' to waiting requester %s\n", strrchr(nrep_msg, ' ') + 1, requester_addr);
        log_msg(msg, own_addr);
    } 
}
