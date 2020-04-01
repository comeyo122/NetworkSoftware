// BcastReceiver.c
// Broadcast Receiver (server)
// Source: 김선우, 윈도우네트워크프로그래밍, 한빛미디어
//
// 과목명: 네트워크소프트웨어설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 통신 네트워크 연구실 (mmcn.ajou.ac.kr)
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUFSIZE 512


int main(int argc, char* argv[])
{
	int retval;
	int optval = true;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		exit(0);
	}
	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(9000);
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	

	if (retval == SOCKET_ERROR) {
		exit(0);
	}
	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 브로드캐스트 데이터 받기
	while (1) {
		// 데이터 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			continue;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port), buf);
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}