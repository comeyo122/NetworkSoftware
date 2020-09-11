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
#define IPTOSBUFFERS	12

#define SNAPLEN 68        // size of captured packet
#define MAX_CAP_PKT  5000    // max. number of stored pkts

#define MAX_ID_LEN 10

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



#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "header.h"
extern statistic agent_info;
#pragma warning(disable:4996)

DWORD WINAPI ProcessPacketCapture(LPVOID arg);
// call back function
char* iptos(u_long in);