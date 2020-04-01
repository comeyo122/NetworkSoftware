// BcastReceiver.c
// Broadcast Receiver (server)
// Source: �輱��, �������Ʈ��ũ���α׷���, �Ѻ��̵��
//
// �����: ��Ʈ��ũ����Ʈ�����
// ���ִ��б� ����Ʈ�����а�
// �̵� ��Ƽ�̵�� ��� ��Ʈ��ũ ������ (mmcn.ajou.ac.kr)
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

	// ���� �ʱ�ȭ
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
	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// ��ε�ĳ��Ʈ ������ �ޱ�
	while (1) {
		// ������ �ޱ�
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			continue;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr),
			ntohs(peeraddr.sin_port), buf);
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}