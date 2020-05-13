//
// Packet Capture Example: Analysis of Captured Data
//
// Network Software Design
// Department of Software and Computer Engineering, Ajou University
// by Byeong-hee Roh
//

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <conio.h>
#pragma warning(disable:4996)

#define LINE_LEN 16

// litereals realted to distinguishing protocols
#define ETHERTYPE_IP      0x0800
#define ETH_II_HSIZE      14      // frame size of ethernet v2
#define ETH_802_HSIZE      22      // frame size of IEEE 802.3 ethernet
#define IP_PROTO_IP          0      // IP
#define IP_PROTO_TCP      6      // TCP
#define IP_PROTO_UDP      17      // UDP
#define   RTPHDR_LEN         12      // Length of basic RTP header
#define CSRCID_LEN         4      // CSRC ID length
#define   EXTHDR_LEN         4      // Extension header length

unsigned long   net_ip_count;
unsigned long   net_etc_count;
unsigned long   trans_tcp_count;
unsigned long   trans_udp_count;
unsigned long   trans_etc_count;

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

/* MAC address */
typedef struct mac_address {
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
	u_char byte5;
	u_char byte6;
} mac_address;

typedef struct eth_header {
	mac_address smac;
	mac_address dmac;
	u_short type;
} eth_header;


void do_ip_traffic_analysis(unsigned char *buffer)
{
	unsigned char ip_ver, ip_hdr_len, ip_proto, urg,ack,psh,rst,syn,fin;
	int   ip_offset = 14;
	
	ip_ver = buffer[ip_offset] >> 4;      // IP version
	ip_hdr_len = buffer[ip_offset] & 0x0f;   // IP header length
	ip_proto = buffer[ip_offset + 9];      // protocol above IP

	printf("Source-Ip : %d.%d.%d.%d , Dest-Ip : %d.%d.%d.%d, protocol:%d \n", buffer[26], buffer[27], buffer[28], buffer[29], buffer[30], buffer[31], buffer[32], buffer[33], buffer[23]);
	short dstport, srcport;

	dstport = pntoh16(&buffer[36]);
	srcport = pntoh16(&buffer[34]);
	
	//46
	printf("Src port : %d, Dst port : %d\n", srcport, dstport);

	if (ip_proto == IP_PROTO_UDP)
		trans_udp_count++;
	else if (ip_proto == IP_PROTO_TCP)
	{
		ip_offset = 47;
		trans_tcp_count++;
		urg = buffer[ip_offset] & 0x20;
		if (urg != 0x00)
			printf("URG : SET");
		else
			printf("USG : NOT SET");
		ack = buffer[ip_offset] & 0x10;
		if (ack != 0x00)
			printf(", ACK : SET");
		else
			printf(", ACK : NOT SET");
		psh = buffer[ip_offset] & 0x8;
		if (psh != 0x00)
			printf(", PSH : SET");
		else
			printf(", PSH : NOT SET");
		rst = buffer[ip_offset] & 0x4;
		if (rst != 0x00)
			printf(", RST : SET");
		else
			printf(", RST : NOT SET");
		syn = buffer[ip_offset] & 0x2;
		if (syn != 0x00)
			printf(", SYN : SET");
		else
			printf(", SYN : NOT SET");
		fin = buffer[ip_offset] & 0x1;
		if (fin != 0x00)
			printf(", FIN : SET\n");
		else
			printf(" FIN : NOT SET\n");
	}
	else
		trans_etc_count++;

}

void do_traffic_analysis(unsigned char *buffer)
{
	unsigned short type;

	// ethernet type check
	type = pntoh16(&buffer[12]);


	eth_header *eh;
	eh = (eth_header *)(buffer);

	printf("Source Mac : %0x-%0x-%0x-%0x-%0x-%0x , Destination Mac : %0x-%0x-%0x-%0x-%0x-%0x , Type : %0x\n", eh->dmac.byte1, eh->dmac.byte2, eh->dmac.byte3, eh->dmac.byte4, eh->dmac.byte5, eh->dmac.byte6
		, eh->smac.byte1, eh->smac.byte2, eh->smac.byte3, eh->smac.byte4, eh->smac.byte5, eh->smac.byte6, ntohs(eh->type));


	if (type == 0x0800)
	{
		net_ip_count++;
		do_ip_traffic_analysis(buffer);
	}
	else
		net_etc_count++;
}

int main(int argc, char **argv)
{
	struct pcap_file_header   fhdr;
	struct pcap_pkthdr       chdr;
	unsigned char         buffer[2000];
	FILE               *fin;
	int                  i = 0;
	char   a;

	fin = fopen("C:\\Users\\Administrator\\Desktop\\capture.pcap", "rb");

	fread((char *)&fhdr, sizeof(fhdr), 1, fin);

	while (fread((char *)&chdr, sizeof(chdr), 1, fin) != 0 && _getch()) // Packet header
	{
		fread(buffer, sizeof(unsigned char), chdr.caplen, fin);
		do_traffic_analysis(buffer);
		i++;
	}
	fclose(fin);

	printf("#total number of packets : %d\n", i);
	printf("IP packets: %d\n", net_ip_count);
	printf("non-IP packets: %d\n", net_etc_count);
	printf("TCP packets: %d\n", trans_tcp_count);

	return 0;
}