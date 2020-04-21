// CSthreadSvr.cpp
// TCP echo concurrent server using multithread
//
// 과목명: 네트워크소프트웨어설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 통신 네트워크 연구실 (mmcn.ajou.ac.kr)
//
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
#define BUFSIZE 512


int user_num = 0;

// socket errof display and quit
void err_quit(const char* msg)
{
	printf("socket function error [%s] (error code: %d)... program terminated \n", msg, WSAGetLastError());
	exit(-1);
}

// socket errof display and quit
void err_display(const char* msg, int code)
{
	printf("%s (error code: %d)\n", msg, code);
}


// Thread function to communicate with a client
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET				client_sock = (SOCKET)arg;
	char				buf[BUFSIZE + 1];
	SOCKADDR_IN			clientaddr;
	int					addrlen;
	int					retval;

	user_num++;

	// obtain the client's information
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);

	while (1) {
		// receive data from the client
		memset(buf, 0x00, sizeof(buf));
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("socket error: recv()", WSAGetLastError());
			break;
		}
		else if (retval == 0) {
			break;
		}
		// desply the received data
		buf[retval] = '\0';
		printf("Message from client (%s:%d): %s, (current user num : %d) \n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf, user_num);
		buf[retval] = NULL;
		// echo the received data
		sprintf(buf, "%s, (current user num : %d)", buf, user_num);

		printf("test buf: %s\n", buf);
		buf[strlen(buf)] = '\n';
		retval = send(client_sock, buf, strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("socket error: send()", WSAGetLastError());
			break;
		}
	}

	// closesocket()
	closesocket(client_sock);
	printf("[TCP Server] connection terminated: client (%s:%d)\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	user_num--;

	return 0;
}

int main(int argc, char* argv[])
{
	SOCKADDR_IN		serveraddr, clientaddr;		// server and client address
	SOCKET			listen_sock, client_sock;	// sockets for listen and client communication
	int				addrlen;
	int				retval;
	HANDLE			hThread;
	DWORD			ThreadId;

	// winsock initialization
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// server IP address and port number
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind()
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	printf("# Concurrent server using multi-thread \n");
	printf("# Network SW Design Course by Ajou University \n\n");

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("socket error: accept()", WSAGetLastError());
			continue;
		}

		printf("[TCP Server] connection establisehd to Clinet (IP: %s, port: %d)\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// thread for client
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, &ThreadId);
		if (hThread == NULL)
			err_display("error: failure of thread creation!!!", GetLastError());
		else
			CloseHandle(hThread);
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}