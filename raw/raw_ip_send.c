#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

// 96 bit (12 bytes) pseudo header needed for udp header checksum calculation 
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};

struct ip_option {
    unsigned int type : 8;
    unsigned int length : 8;
    unsigned int value : 16;
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
    char datagram[4096], // datagram to represent the packet
         source_ip[32],
         *data,
         *pseudogram;
    
    raw_socket = socket(AF_INET, SOCK_RAW, 200);

    if ( raw_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    int on = 1;
    if (setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("Error setting IP router alert option for socket");
    }

    // zero out the packet buffer
    memset (datagram, 0, 4096);
    
    // IP header
    struct iphdr *iph = (struct iphdr *) datagram;

    // IP option
    struct ip_option *router_alert = (struct ip_option *) (datagram + sizeof (struct ip));
    
    // UDP header
    //struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip) + sizeof (struct ip_option));
    
    struct sockaddr_in sin;
    struct pseudo_header psh;
    
    // data
    data = datagram + sizeof(struct iphdr) + sizeof (struct ip_option);// + sizeof(struct udphdr);
    strcpy(data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    
    // some address resolution
    strcpy(source_ip , "10.0.0.1");
    
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("10.0.0.3");
    
    // IP header
    iph->ihl = 6; // 5 by default
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct ip_option) + /*sizeof (struct udphdr) +*/ strlen(data);
    iph->id = htonl (54321); // Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = 200;
    iph->check = 0; // set to 0 before calculating checksum
    iph->saddr = inet_addr( source_ip ); // spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;
    // IP header checksum
    iph->check = csum ((unsigned short *) datagram, iph->tot_len);
    
    // IP router alert option
    router_alert->type = 148;   // (8 bits) 1001 0100
    router_alert->length = 4;   // (8 bits) 0000 0100
    router_alert->value = 0;    // (16 bits)
    /*
    // UDP header
    udph->source = htons (6666);
    udph->dest = htons (8622);
    udph->len = htons(8 + strlen(data)); // udp header size
    udph->check = 0; // set to 0 before calculating checksum
    // UDP checksum using the pseudo header
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + strlen(data));
    
    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
    pseudogram = malloc(psize);
    
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + strlen(data));
    
    udph->check = csum( (unsigned short*) pseudogram , psize);
    */

    //while (1)
    {
        if (sendto (raw_socket, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
            perror("sendto failed");
        } else {
            printf ("Packet Sent. Length : %d \n" , iph->tot_len);
        }
    }
    return 0;
}
