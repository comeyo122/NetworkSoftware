// CSselectSvr.c
// TCP echo concurrent server using select()
//
// 과목명: 네트워크소프트웨어설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 통신 네트워크 연구실 (mmcn.ajou.ac.kr)
//
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include <winsock.h>

#define BUFSIZE 512

// socket errof display and quit
void err_quit(const char* msg, int code)
{
	printf("socket function error [%s] (error code: %d)... program terminated \n", msg, code);
	exit(-1);
}

// socket errof display and quit
void err_display(const char* msg, int code)
{
	printf("%s (error code: %d)\n", msg, code);
}

void ErrorHandling(const char *message);

int main(int argc, char **argv)
{
	SOCKET        svrSock;
	SOCKADDR_IN   svrAddr;
	u_short       svrPort;
	SOCKET        clntSock;
	SOCKADDR_IN   clntAddr;
	int           i;
	int           addrlen;
	int           retval;
	int			  num_user = 0;
	fd_set readfds, tempfds;

	char          message[BUFSIZE];
	int           msglen;
	TIMEVAL       timeout; //struct timeval timeout. 

	// parameter from command line
	if (argc != 2) {
		svrPort = 9000;
		printf("Usage : %s <port>: default (9000)\n", argv[0]);
	}
	else {
		svrPort = atoi(argv[1]);
	}

	// winsock initialization: Load Winsock 2.2 DLL
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("WSAStartup() error!", -1);

	// server listen socket creation
	svrSock = socket(AF_INET, SOCK_STREAM, 0);
	if (svrSock == INVALID_SOCKET) err_quit("socket()", WSAGetLastError());

	// server address
	ZeroMemory(&svrAddr, sizeof(svrAddr));
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svrAddr.sin_port = htons(svrPort);

	// bind
	retval = bind(svrSock, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
	if (retval == SOCKET_ERROR) err_quit("bind()", WSAGetLastError());

	// listen
	retval = listen(svrSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()", WSAGetLastError());

	// clear the read fd_sets
	FD_ZERO(&readfds);

	// set for connection attempts (on listen socket)
	FD_SET(svrSock, &readfds);

	while (1)
	{
		// copy readfds to tempfds
		tempfds = readfds;

		// timeout value
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		// select()
		retval = select(0, &tempfds, NULL, NULL, &timeout);

		// select error
		if (retval == SOCKET_ERROR) err_quit("select()", WSAGetLastError());

		// timeout event
		if (retval == 0) {
			printf("select() timeout\n");
			continue;
		}

		// client's events occured
		printf("select(): readfds.fd_count=%d:", readfds.fd_count);
		for (i = 0; i < readfds.fd_count; i++)
			printf(" %d", readfds.fd_array[i]);
		printf("\n");

		for (i = 0; i < readfds.fd_count; i++)
		{
			if (FD_ISSET(readfds.fd_array[i], &tempfds))
			{
				if (readfds.fd_array[i] == svrSock) // connection from a new client
				{
					// accept ( )
					addrlen = sizeof(clntAddr);
					clntSock = accept(svrSock, (SOCKADDR*)&clntAddr, &addrlen);
					if (clntSock == INVALID_SOCKET) {
						err_display("accept(): clntSock=INVALID_SOCKET", WSAGetLastError());
						continue;
					}

					// FD_SET( ): add the client's socket to read fd_set
					FD_SET(clntSock, &readfds);
					printf("\n[Select Server] accepted (client socket=%d): client (%s:%d)\n",
						clntSock, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
					num_user++;

				}
				else // echo client-server communication with a client
				{
					// recv ( )
					msglen = recv(readfds.fd_array[i], message, BUFSIZE - 1, 0);

					// get the information of the client that sent the message
					addrlen = sizeof(clntAddr);
					retval = getpeername(readfds.fd_array[i], (SOCKADDR*)&clntAddr, &addrlen);

					// connection close request from the client
					if (msglen == 0)
					{
						// close( ): socket close
						closesocket(readfds.fd_array[i]);

						// FD_CLR( ): clear the socket from read fd_set
						FD_CLR(readfds.fd_array[i], &readfds);

						printf("[Select Server] close socekt (%d) for cleint(%s:%d)  \n", readfds.fd_array[i],
							inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
						num_user--;
					}
					// data from the client
					else
					{
						// send( ): message echo
						message[msglen] = '\0';

						printf("[Select Server] to cleint(%s:%d) echoed message: %s, (current_user_num : %d)\n", inet_ntoa(clntAddr.sin_addr),
							ntohs(clntAddr.sin_port), message, num_user);

						sprintf(message, "%s, (current_user_num :%d)\n", message, num_user);
						send(readfds.fd_array[i], message, strlen(message), 0);
					}
				}
			} //if(FD_ISSET(readfds.fd_array[i], &tempfds)) end
		} //for(i=0; i<readfds.fd_count; i++)  end
	} //while(1) end

	WSACleanup();
	return 0;
}