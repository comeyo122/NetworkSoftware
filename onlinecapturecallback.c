//
// Packet Capture Example: Onlie packet capture
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

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <pcap.h>
#include <pcap/pcap.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma warning(disable:4996)

#define LINE_LEN 16

 // litereals realted to distinguishing protocols
#define ETHERTYPE_IP		0x0800
#define ETH_II_HSIZE		14		// frame size of ethernet v2
#define ETH_802_HSIZE		22		// frame size of IEEE 802.3 ethernet
#define IP_PROTO_IP		    0		// IP
#define IP_PROTO_TCP		6		// TCP
#define IP_PROTO_UDP		17		// UDP
#define	RTPHDR_LEN			12		// Length of basic RTP header
#define CSRCID_LEN			4		// CSRC ID length
#define	EXTHDR_LEN			4		// Extension header length

unsigned long	net_ip_count;
unsigned long	net_etc_count;
unsigned long	trans_tcp_count;
unsigned long	trans_udp_count;
unsigned long	trans_etc_count;

#define SNAPLEN 68        // size of captured packet
#define MAX_CAP_PKT  5000    // max. number of stored pkts

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

char*   iptos(u_long in);

// call back function
void    get_stat(u_char* user, const struct pcap_pkthdr* h, const u_char* p);

int	    numpkt_per_sec, numbyte_per_sec;	// packet and byte rates per second
int	    tot_capured_pk_num = 0;		        // total number of captured packets
long	crnt_sec, prev_sec;		            // time references in second
pcap_t* adhandle;                           // globally defined for callback fucntion

int no = 0;
int main()
{
	pcap_if_t*  alldevs;
	pcap_if_t*  d;

	char                errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr* pkt_hdr;    // captured packet header
	const u_char*       pkt_data;   // caputred packet data
	time_t              local_tv_sec;
	struct tm*          ltime;
	char                timestr[16];

	int		i, ret;			// for general use
	int		ndNum = 0;	// number of network devices
	int		devNum;		// device Id used for online packet capture

	//printf("default device: %s\n", pcap_lookupdev(errbuf));

	/* Retrieve the device list */
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* Print the list */
	printf("\n");
	pcap_addr_t* a;
	for (d = alldevs; d; d = d->next)
	{
		// device name
		printf(" [%d] %s", ++ndNum, d->name);

		// description
		if (d->description)
			printf(" (%s) ", d->description);

		// loopback address
		// printf("\tLoopback: %s\n", (d->flags & PCAP_IF_LOOPBACK) ? "yes" : "no");

		// IP addresses
		for (a = d->addresses; a; a = a->next) {
			if (a->addr->sa_family == AF_INET) {
				if (a->addr)
					printf("[%s]", iptos(((struct sockaddr_in*)a->addr)->sin_addr.s_addr));
				//if (a->netmask)
				//    printf("\tNetmask: %s\n", iptos(((struct sockaddr_in*)a->netmask)->sin_addr.s_addr));
				//if (a->broadaddr)
				//    printf("\tBroadcast Address: %s\n", iptos(((struct sockaddr_in*)a->broadaddr)->sin_addr.s_addr));
				//if (a->dstaddr)
				//    printf("\tDestination Address: %s\n", iptos(((struct sockaddr_in*)a->dstaddr)->sin_addr.s_addr));
				break;
			}
		}
		printf(" flag=%d\n", (int)d->flags);
	}
	printf("\n");
	/* error ? */
	if (ndNum == 0)
	{
		printf("\nNo interfaces found! Make sure Npcap is installed.\n");
		return -1;
	}

	/* select device for online packet capture application */
	printf(" Enter the interface number (1-%d):", ndNum);
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
		65536,     // portion of the packet to capture. 
					// 65536 grants that the whole packet will be captured on all the MACs.
		1,         // promiscuous mode
		1000,      // read timeout
		errbuf)     // error buffer
		) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by Npcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("\n Selected device %s is available\n\n", d->description);
	pcap_freealldevs(alldevs);

	// start the capture
	numpkt_per_sec = numbyte_per_sec = net_ip_count = trans_tcp_count = trans_udp_count = 0;
	printf("<no>\t\t<time>\t\t\t<Source>\t<Destination>\t<type>\t<Protocol>\t<Length>\t<info>\n");
	pcap_loop(adhandle, 	// capture device handler
		-1, 	   	// forever
		get_stat,   // callback function
		NULL);      // no arguments


/* Close the handle */
//pcap_close(adhandle);

	return 0;
}

/* From tcptraceroute, convert a numeric IP address to a string : source Npcap SDK */
#define IPTOSBUFFERS	12
char* iptos(u_long in)
{
	static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
	static short which;
	u_char* p;

	p = (u_char*)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}

void get_stat(u_char* user, const struct pcap_pkthdr* h, const u_char* p)
{
	time_t rawtime;
	struct tm* ltime;
	char timestr[50];
	unsigned short type;

	time(&rawtime);
	ltime = localtime(&rawtime);
	strcpy(timestr, asctime(ltime));
	timestr[strlen(timestr) - 1] = '\0';

	// convert the timestamp to readable format
//	ltime = localtime((const time_t*)&h->ts.tv_sec);
	//strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

	// check time difference in second
	crnt_sec = h->ts.tv_sec;

	no++;

	printf("%d\t %s    %02d.%d.%d.%d    %d.%d.%d.%d\t    %0x", no, timestr, p[26], p[27], p[28], p[29], p[30], p[31], p[32], p[33], pntoh16(&p[12]));


	numpkt_per_sec = numbyte_per_sec = net_ip_count = trans_tcp_count = trans_udp_count = 0;

	prev_sec = crnt_sec;
	numpkt_per_sec++;
	numbyte_per_sec += h->len;
	if ((type = pntoh16(&p[12])) == 0x0800) {
		net_ip_count++;
		if (p[23] == IP_PROTO_UDP)
		{
			trans_udp_count++;
			printf("\t UDP\t %d\ %d -> %d \n", h->len, pntoh16(&p[34]), pntoh16(&p[36]));
		}
		else if (p[23] == IP_PROTO_TCP)
		{
			trans_tcp_count++;
			printf("\t TCP\t %d\ %d -> %d \n", h->len, pntoh16(&p[34]), pntoh16(&p[36]));
		}
		else
		{
			printf("   not TCP/UDP\n");
			trans_etc_count++;
		}
	}
	else
	{
		printf("   not TCP/UDP\n");
	}

	if (tot_capured_pk_num++ > MAX_CAP_PKT) {
		printf("\n\n %d-packets were captured ...\n", tot_capured_pk_num);

		// close all devices and files
		pcap_close(adhandle);
		exit(0);
	}
}