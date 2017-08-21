#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "cfuhash.h"

char *msg;

void *listen_for_nreqs(void *pnrs) {
    //cfuhash_table_t *pnrs = (cfuhash_table_t *)hast_table;
    cfuhash_put(pnrs, "hello", "world");
    cfuhash_pretty_print(pnrs, stdout);
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
        recvfrom(s, req_buf, INET_ADDRSTRLEN, 0, (struct sockaddr *) &requester, &requester_size);

        asprintf(&msg, "### NREQ: %s wants name for %s\n", inet_ntoa(requester.sin_addr), req_buf);
        log_msg(msg, own_addr);

        // TODO: name is cached ? respond : in PNRs? <<<<<<<<<<<<<<<<<<<<<<<<
        strcpy(res_buf, hostname);
        sendto(s, res_buf, MAX_NAME_LEN, 0, (struct sockaddr*) &requester, requester_size);

        asprintf(&msg, "### Sent name %s to %s\n", hostname, inet_ntoa(requester.sin_addr));
        log_msg(msg, own_addr);

        // clear stdout and buffers
        fflush(stdout);
        memset(req_buf,'\0', INET_ADDRSTRLEN);
        memset(res_buf,'\0', MAX_NAME_LEN);
    }
}
