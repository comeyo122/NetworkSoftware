
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
#include <pcap.h>
#include <pcap/pcap.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agent.h"

#pragma warning(disable:4996)


char* iptos(u_long in);

// call back function
void    get_stat(u_char* user, const struct pcap_pkthdr* h, const u_char* p);

struct timeval	init_time_sec;
int	    tot_capured_pk_num = 1;		        // total number of captured packets
pcap_t* adhandle;                           // globally defined for callback fucntion


DWORD WINAPI ProcessPacketCapture(LPVOID arg)
{
    adhandle = (pcap_t*)arg;
    // start the capture
    pcap_loop(adhandle, 	// capture device handler
        -1, 	   	// forever
        get_stat,   // callback function
        NULL);      // no arguments


//Close the handle 
    pcap_close(adhandle);

    return 0;
}


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
    struct tm* ltime;
    char timestr[16];
    unsigned short type;
    long sec, usec;
    unsigned int source_ip, dest_ip;
    unsigned short sport, dport;
    unsigned char* ip_output;
    unsigned char ip_proto, ip_hdr_len;
    char id[MAX_ID_LEN];

    int	ip_offset = 14;
    int source_offset = 12;
    int dest_offset = 16;
    int trans_offset;

    id[0] = 0;

    type = pntoh16(&p[12]);

    switch (type) {
    case 0x0800: // IPv4
        id[1] = 0;
        break;
    case 0x0806: // ARP
        id[1] = 1;
        break;
        /*
    case 0x86DD: // IPv6
        printf("IPv6\t");
        break;
        */
    default:
        id[1] = 2;
        break;
    }

    ip_proto = p[ip_offset + 9];		// protocol above IP
    ip_hdr_len = p[ip_offset] & 0x0f;	// IP header length

    if (ip_proto == IP_PROTO_UDP || ip_proto == IP_PROTO_TCP)
    {
        trans_offset = ip_offset + (ip_hdr_len * 4);
        sport = pntoh16(&p[trans_offset]);
        dport = pntoh16(&p[trans_offset + 2]);

        id[3] = 0;
        if (ip_proto == IP_PROTO_TCP)
        {
            id[2] = 1;
            if (p[trans_offset + 13] == 0x02)
                id[3] = 1;
        }
        else if (ip_proto == IP_PROTO_UDP)
        {
            id[2] = 0;
        }
    }
    else
    {
        id[2] = 2;
    }

    agent_info.sumofPktLen += h->len;
    agent_info.numofPacket += 1;

    if (id[1] == 0)
        agent_info.numofIP += 1;
    else if (id[1] == 1)
        agent_info.numofARP += 1;

    if (id[2] == 1)
    {
        agent_info.numofTCP += 1;
        if (id[3] == 1)
            agent_info.numofTCPSYN += 1;
    }
    else if (id[2] == 0)
        agent_info.numofUDP += 1;
}
