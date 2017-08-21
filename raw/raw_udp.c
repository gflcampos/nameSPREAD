#include <stdio.h> //for printf
#include <string.h> //memset
#include <sys/socket.h>    //for socket of course
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/ip.h>    //Provides declarations for ip header

int router_alert_opt_val = 1;

//setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val));
//setsockopt(raw_socket, IPPROTO_IP, IP_ROUTER_ALERT, &ip_router_alert, sizeof(ip_router_alert));

// 96 bit (12 bytes) pseudo header needed for udp header checksum calculation 
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};
 
unsigned short csum(unsigned short *ptr,int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;
 
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}
 
int main (void) {
    int raw_socket;
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    
    if (setsockopt(raw_socket, IPPROTO_IP, IP_ROUTER_ALERT, &router_alert_opt_val, sizeof router_alert_opt_val) < 0) {
        perror("setsockopt failed");
    }

    // Datagram to represent the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
     
    // zero out the packet buffer
    memset (datagram, 0, 4096);
     
    // UDP header
    struct udphdr *udph = (struct udphdr *) (datagram);
     
    struct sockaddr_in sin;
    struct pseudo_header psh;
     
    // Data part
    data = datagram + sizeof(struct udphdr);
    strcpy(data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
     
    // some address resolution
    strcpy(source_ip , "192.168.1.2");
     
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("192.168.1.254");
    
    // UDP header
    udph->source = htons (6666);
    udph->dest = htons (8622);
    udph->len = htons(8 + strlen(data)); // udp header size
    udph->check = 0; //leave checksum 0 now, filled later by pseudo header
     
    // Now the UDP checksum using the pseudo header
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );
     
    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
    pseudogram = malloc(psize);
    
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + strlen(data));
    
    udph->check = csum( (unsigned short*) pseudogram , psize);
    int tot_len = sizeof (struct udphdr) + strlen(data);
    
    while (1) {
        // Send the packet
        if (sendto (raw_socket, datagram, tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
            perror("sendto failed");
        } else { // Data sent successfully
            printf ("Packet Sent. Length : %d \n" , tot_len);
        }
    }
    return 0;
}
