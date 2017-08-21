#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/ip.h>
//#include <net/ip.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

void processPacket(unsigned char* , int);
void printIpHeader(unsigned char* , int);

struct sockaddr_in source, dest;
int router_alert_opt_val = 1;

int main (void) {
    struct sockaddr_in namedEntity;
    struct sockaddr saddr;
    int raw_socket, saddr_size = sizeof saddr, data_size;
    char recv_buff[4096];
    
    raw_socket = socket(AF_INET, SOCK_RAW, 200);
    
    if ( raw_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (setsockopt(raw_socket, IPPROTO_IP, IP_ROUTER_ALERT, &router_alert_opt_val, sizeof(router_alert_opt_val)) < 0) {
        perror("Error setting IP router alert option for socket");
    }

    /*
    namedEntity.sin_family = AF_INET;
    //namedEntity.sin_port = htons(nePort);
    namedEntity.sin_addr.s_addr = inet_addr("10.0.0.2");
    memset(namedEntity.sin_zero, '\0', sizeof namedEntity.sin_zero);
    
    bind(raw_socket, (struct sockaddr *)&namedEntity, sizeof(namedEntity));    
    */
    while (1) {
        printf("Waiting for packets...\n");
        data_size = recvfrom (raw_socket, recv_buff, sizeof(recv_buff), 0, &saddr, &saddr_size);
        
        if (data_size < 0) {
            perror("recvfrom error, failed to get packets\n");
            return 1;
        }

        printf("Packet received\n");
        processPacket(recv_buff, data_size);
    }

    close(raw_socket);
    return 0;
}

void processPacket(unsigned char* buffer, int size) {
    printIpHeader(buffer, size);
}

void printIpHeader(unsigned char* Buffer, int Size) {
    unsigned short iphdrlen;
         
    struct iphdr *iph = (struct iphdr *)Buffer;
    iphdrlen =iph->ihl*4;
     
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
     
    printf("\n");
    printf("IP Header\n");
    printf("   |-IP Version        : %d\n",(unsigned int)iph->version);
    printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
    printf("   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
    printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
    printf("   |-Identification    : %d\n",ntohs(iph->id));
    //fprintf("   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
    //fprintf("   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
    //fprintf("   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
    printf("   |-TTL      : %d\n",(unsigned int)iph->ttl);
    printf("   |-Protocol : %d\n",(unsigned int)iph->protocol);
    printf("   |-Checksum : %d\n",ntohs(iph->check));
    printf("   |-Source IP        : %d\n",inet_ntoa(source.sin_addr));
    printf("   |-Destination IP   : %d\n",inet_ntoa(dest.sin_addr));
}