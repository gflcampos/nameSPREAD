#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
//#include "common.h"
//#include "cfuhash.h"

char *msg;

void *listen_for_nreqs() {
    //cfuhash_put(pnrs, "hello", "world");
    //cfuhash_pretty_print(pnrs, stdout);
    const char *hostname = get_hostname();
    struct sockaddr_in self, requester;
    int s, requester_size = sizeof requester, conn_fd;
    char req_buf[INET_ADDRSTRLEN], res_buf[MAX_NAME_LEN];

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset((char *) &self, 0, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(NREQ_RESPONDER_PORT);
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(s, (struct sockaddr *) &self, sizeof(self));
    
    asprintf(&msg, "### Waiting for NREQs on port %d...\n", NREQ_RESPONDER_PORT);
    log_msg(msg, own_addr);

    while (1) {
        char *requester_addr;
        char *name;

        recvfrom(s, req_buf, INET_ADDRSTRLEN, 0, (struct sockaddr *) &requester, &requester_size);
        requester_addr = inet_ntoa(requester.sin_addr);

        asprintf(&msg, "### NREQ: %s wants name for %s\n", requester_addr, req_buf);
        log_msg(msg, own_addr);

        name = get_name_by_addr(req_buf);

        if (name) { // is the name cached?
            strcpy(res_buf, name);
            sendto(s, res_buf, MAX_NAME_LEN, 0, (struct sockaddr*) &requester, requester_size);

            asprintf(&msg, "### Sent name %s to %s\n", name, requester_addr);
            log_msg(msg, own_addr);
        } else {
            // if name is not cached, then a PNR for that name must already exist in PNRs table
            // so, add requester_addr to the list of requesters of that PNR
            register_nreq(req_buf, requester_addr);
            // when available, the response will be sent. If a timeout occurs, 
            // then a timeout notification will be sent instead (by the timeout handler function)
        }       

        // clear stdout and buffers
        fflush(stdout);
        memset(req_buf,'\0', INET_ADDRSTRLEN);
        memset(res_buf,'\0', MAX_NAME_LEN);
    }
}
