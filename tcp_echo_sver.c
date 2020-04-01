// COechoSvr.cpp
// TCP echo server
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
#define BUFSIZE 1500

//MSS 1500 byte, CL -> Server로 보내는 구조체 
typedef struct TOSERVER {

	int stno;
	int namelen;
	char name[30];
	char msg[1462];
}toserver;

//Server -> Client로 보내는 구조체 
typedef struct TOCLIENT {

	int len;
	char msg[1462];
}tocl;

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char* argv[])
{
	// 데이터 통신에 사용할 변수
	SOCKET listen_sock;
	SOCKET client_sock;
	SOCKADDR_IN serveraddr;
	SOCKADDR_IN clientaddr;
	toserver fromcl;
	tocl response;

	int		addrlen;
	char	buf[BUFSIZE + 1];
	int		retval, msglen;



	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) exit(0);

	// server address
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
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

		printf("\n[TCP Server] Client accepted : IP addr=%s, port=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {


			//구조체 초기화
			memset(&fromcl, 0x00, sizeof(fromcl));
			memset(&response, 0x00, sizeof(response));

			// 데이터 받기
			msglen = recv(client_sock, (char *)&fromcl, sizeof(fromcl), 0);
			if (msglen == SOCKET_ERROR) {
				exit(0);
				break;
			}


			// 받은 데이터 출력

			buf[msglen] = '\0';
			printf("[TCP/%s:%d]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));


			if (strcmp(fromcl.msg, "QUIT") == 0 && fromcl.stno == 0) {
				//단순히 QUIT만 보내서 종료 
				printf("메세지:QUIT\n");
				response.len = 4;
				strcpy(response.msg, "QUIT");
				retval = send(client_sock, (const char*)&response, sizeof(response), 0);
				break;
			}
			else
			{
				// 일반적인 경우거나, 정상적인 에코메세지로서의 QUIT을 보냈을 경우
				printf("학번:%d, 이름:%s, 메시지:%s\n", ntohl(fromcl.stno), fromcl.name, fromcl.msg);

				// 데이터 처리
				response.len = strlen(fromcl.msg);
				strcpy(response.msg, fromcl.msg);
			}
			// 데이터 보내기
			retval = send(client_sock, (const char*)&response, sizeof(response), 0);
			if (retval == SOCKET_ERROR) {
				exit(0);
				break;
			}

		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);
	// 윈속 종료
	WSACleanup();
	return 0;
}