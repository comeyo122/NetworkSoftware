#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <pcap.h>
#include <pcap/pcap.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "agent.h"
#pragma comment(lib, "ws2_32.lib")
#define BUFSIZE 512

void TCP_SERVER();

pcap_t* adhandle;                           // globally defined for callback fucntion
statistic agent_info;
int read_flag = 0;



// quit the program 
void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// display error 
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// Open Device Handler 
pcap_t* open_device_handle()
{
	pcap_if_t*  alldevs;
	pcap_if_t*  d;
	pcap_t* adhandle;
	char    errbuf[PCAP_ERRBUF_SIZE];
	int i,ndNum = 0 , devNum;

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


		// IP addresses
		for (a = d->addresses; a; a = a->next) {
			if (a->addr->sa_family == AF_INET) {
				if (a->addr)
					printf("[%s]", iptos(((struct sockaddr_in*)a->addr)->sin_addr.s_addr));

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

	return adhandle;
}

DWORD WINAPI udp_trap(LPVOID arg)
{
	int retval=0;
	//pre_defined msg
	trap trap_msg;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		exit(0);
		//err_quit("socket()");
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(MANAGER_UDP_PORT);
	serveraddr.sin_addr.s_addr = inet_addr("3.34.2.200");

	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[2];
	int len;
	char * test;

	
	// Check TCP Syn, check SynFlooding
	while (1) {


		if (agent_info.numofTCPSYN >= 3000) {
			memset((trap*)&trap_msg, 0x00, sizeof(trap_msg));
			trap_msg.type = SYSTEM_ATTACK;
			trap_msg.code = SYN_FLOODING;

		// send trap message to Manager(UDP)
			retval = sendto(sock, (char *) &trap_msg, sizeof(trap_msg), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr));

			printf("detected syn flooding\n");

		}
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		Sleep(1000);
	}

	// closesocket()
	closesocket(sock);

	//cleanup Socket
	WSACleanup();
	return 0;
}




int main(int argc, char* argv[])
{
	SOCKET			client_sock;	// sockets for listen and client communication
	HANDLE			hThread, CapThread;
	DWORD			ThreadId;
	pcap_t*			adhandle;
	// winsock initialization
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;
	
	//malloc Agent_info
	memset((statistic*)&agent_info, 0x00, sizeof(agent_info));
	
	//open device hgandler 
	adhandle = open_device_handle();

	
	//Create Packet_capture Thread 
	CapThread = CreateThread(NULL, 0, ProcessPacketCapture, (LPVOID)adhandle, 0, &ThreadId);
	if (CapThread == NULL)
		err_display("error: failure of thread creation!!!");
	else
		CloseHandle(CapThread);
	
	hThread = CreateThread(NULL, 0, udp_trap, (LPVOID)NULL, 0, &ThreadId);
	if (hThread == NULL)
		err_display("error: failure of thread creation!!!");
	else
		CloseHandle(hThread);
	
	

	//##########TCP SERVER routine ############ 
	TCP_SERVER();
	
	WSACleanup();
	
	return 0;
}


// routine for Management
void TCP_SERVER() {

	
	SOCKET listen_sock, client_sock;
	SOCKADDR_IN serveraddr, clientaddr;
	int		addrlen, retval;
	char buffer[2000];

	clock_t currect_clock, previous_clock;

	currect_clock = 0;
	previous_clock = clock();

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) exit(0);

	// server address
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(AGENT_TCP_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	// bind()
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) exit(0);

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) exit(0);


	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			exit(0);
			continue;
		}

		printf("\n[AGENT-SERVER Connected]\n");

		// give Agent_info to Manager_Server
		while (1) {
			retval = recv(client_sock, buffer, sizeof(buffer), 0);

			currect_clock = clock();
			agent_info.bitrate = (double)(agent_info.sumofPktLen * 8.0) / ((double)(currect_clock - previous_clock) / CLOCKS_PER_SEC);


			// 구조체 정보 전달 
			retval = send(client_sock, (const char*)&agent_info, sizeof(agent_info), 0);


			previous_clock = currect_clock;
			
			//구조체 초기화 
			memset(&agent_info, 0x00, sizeof(agent_info));
			if (retval == SOCKET_ERROR) {
				break;
			}

		}

		// closesocket()
		closesocket(client_sock);
		printf("[Agent-Server connection End]");
	}

	// closesocket()
	closesocket(listen_sock);
}