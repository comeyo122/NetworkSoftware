// COechoClnt.c
// TCP echo client
//
// 과목명: 네트워크소프트웨어설계
// 아주대학교 소프트웨어학과
// 모바일 멀티미디어 통신 네트워크 연구실 (mmcn.ajou.ac.kr)
//

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")


#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "header.h"

#define BUFSIZE 1500
#define MAX_HANDLE_SIZE	50

char buf[BUFSIZ];

// 소켓 함수 오류 출력 후 종/료
void err_quit(char* msg)
{
	printf("Error [%s] ... program terminated \n", msg);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
	printf("socket function error [%s]\n", msg);
}

DWORD WINAPI ProcessUDPServer(LPVOID arg)
{
	SOCKET				socket = (SOCKET)arg;
	SOCKADDR_IN			clientaddr;
	char				*host_name;
	trap				msg;
	int					addrlen;
	int					retval;

	addrlen = sizeof(clientaddr);
	while (1) {
		ZeroMemory(&msg, sizeof(msg));
		retval = recvfrom(socket, &msg, sizeof(msg), 0, (SOCKADDR*)&clientaddr, &addrlen);
		if (retval == 0) {
			continue;
		}

		host_name = inet_ntoa(clientaddr.sin_addr);

		if (msg.type == SYSTEM_ATTACK && msg.code == SYN_FLOODING) {
			printf("[%s]!! SYN FLOODING ALERT !!\n", host_name);
		}
		memset((trap*)&msg, 0x00, sizeof(msg));
	}
}

void init_udp_socket(SOCKET* udp_sock) {
	SOCKADDR_IN udp_serveraddr;

	*udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (*udp_sock == INVALID_SOCKET) err_quit("socket()");

	ZeroMemory(&udp_serveraddr, sizeof(udp_serveraddr));

	udp_serveraddr.sin_family = AF_INET;
	udp_serveraddr.sin_port = htons(MANAGER_UDP_PORT);
	udp_serveraddr.sin_addr.s_addr = htons(INADDR_ANY);

	int retval = bind(*udp_sock, (SOCKADDR*)&udp_serveraddr, sizeof(udp_serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");
}

int init_tcp_socket(SOCKET* tcp_sock, char* ip) {
	SOCKADDR_IN			serveraddr;

	int retval = 0;
	// tcp socket()
	*tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*tcp_sock == INVALID_SOCKET) err_quit("socket()");

	// server address
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(AGENT_TCP_PORT);
	serveraddr.sin_addr.s_addr = inet_addr(ip);

	// connect()
	do {
		printf("Trying to connect with %s\n", ip);
		retval = connect(*tcp_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	} while (WSAGetLastError() == WSAETIMEDOUT);
	
	printf("%s is connected!\n", ip);

	return retval;
}

char* prefixbps(double bps , char* buf)
{
	if(bps >= 1000000)
		sprintf(buf, "%.2lf mbps", bps/1000000.0);
	else if(bps>= 1000)
		sprintf(buf, "%.2lf kbps", bps / 1000.0);
	else
		sprintf(buf, "%.2lf bps", bps);
	return buf;
}

DWORD WINAPI ProcessTCPClient(LPVOID arg)
{
	SOCKET				sock;
	agent* ag = (agent*)arg;
	char				buf[BUFSIZE + 1];
	char				bpsbuf[BUFSIZ];
	statistic			agent_info;

	init_tcp_socket(&sock, ag->ip);

	// setup stat message
	ZeroMemory(buf, sizeof(buf));
	strcpy(buf, "get_stat");

	// do tcp request to agent
	while (1) {
		int retval = send(sock, buf, sizeof(buf), 0);
		// TODO: error catch
		ZeroMemory(&agent_info, sizeof(agent_info));
		retval = recv(sock, &agent_info, BUFSIZE, 0);

		if (retval == -1) {
			printf("%s is disconnected...\n", ag->ip);
			init_tcp_socket(&sock, ag->ip);
			continue;
		}

		printf("[%s] packet num: %d\t\tbitRate: %s\ttcpNum: %d\ttcpsynNum: %d\tudpNum: %d\tipNum: %d\n",
			ag->ip, agent_info.numofPacket, prefixbps(agent_info.bitrate,bpsbuf), agent_info.numofTCP, agent_info.numofTCPSYN, agent_info.numofUDP, agent_info.numofIP);

		Sleep(1000);
	}
}

int main(int argc, char* argv[])
{
	int					retval;
	SOCKET				sock[MAX_HANDLE_SIZE];
	SOCKET				udp_sock_server;
	SOCKADDR_IN			serveraddr;

	HANDLE				hThread;
	HANDLE				tcpThread[MAX_HANDLE_SIZE];

	DWORD				ThreadId;
	DWORD				TCPThreadId[MAX_HANDLE_SIZE];

	agent				agent[3] = { NULL, NULL, NULL };
	int					len;
	int					udp_port = 0;
	int					udp_addr_len;
	int					i = 0;

	agent[0].ip = "121.169.44.155";
	agent[0].port = AGENT_TCP_PORT;

	agent[1].ip = "220.120.230.43";
	agent[1].port = AGENT_TCP_PORT;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// start udp server
	init_udp_socket(&udp_sock_server);
	hThread = CreateThread(NULL, 0, ProcessUDPServer, (LPVOID)udp_sock_server, 0, &ThreadId);

	for ( i ; i < 2; i++)
	{
		tcpThread[i] = CreateThread(NULL, 0, ProcessTCPClient, (LPVOID)&agent[i], 0, &TCPThreadId[i]);
	}

	WaitForMultipleObjects(i, tcpThread, TRUE, INFINITE);
	WaitForSingleObject(hThread, INFINITE);

	// Close socket and thread handle
	closesocket(sock);
	CloseHandle(hThread);
	CloseHandle(tcpThread);

	// 윈속 종료
	WSACleanup();
	return 0;
}