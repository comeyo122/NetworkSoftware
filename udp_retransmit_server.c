// UDPEchoServer.c
// UDP echo server 
//
// 과목명: 네트워크소프트웨설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 융합 네트워크 연구실 (mmcn.ajou.ac.kr)
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define BUFSIZE 512

//데이터 
typedef struct DATA {

	int seq_no;
	char message[1000];
}data;



// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
{
	int retval;
	int ack = 0;
	srand(time(NULL));
	data received_data;
	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("err in sock\n");
		exit(0);
	}
	
	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		printf("err in sock\n");
		exit(0);
	}
	
	// 데이터 통신에 사용할 변수
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int random;

	// 클라이언트와 데이터 통신
	while(1){
		
		// 데이터 받기

		//받을 data 구조체 초기화 
		memset(&received_data, 0x00, sizeof(received_data));
		memset(buf, 0x00, sizeof(buf));

		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, &received_data, sizeof(received_data), 0, (SOCKADDR *)&clientaddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		// 받은 데이터 출력
		printf("[UDP/%s:%d] seq = %d, data = %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ack, received_data.message);
	

		//상대방에게 응답을 보내지 않음 (과제와 일관성을 맞추려고 if else 문으로 구성 
		if ((random = rand() % 101) >= 90) 
		{
		}
		else 
		{
			//상대방에게 응답을 보냄 
			ack += strlen(received_data.message);
			received_data.seq_no = ack;
			retval = sendto(sock, &received_data, sizeof(received_data), 0, (SOCKADDR *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				continue;
			}
		}
		
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
