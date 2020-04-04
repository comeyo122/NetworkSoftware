// UDPEchoServer.c
// UDP echo server 
//
// �����: ��Ʈ��ũ����Ʈ������
// ���ִ��б� ����Ʈ�����а�
// �̵� ��Ƽ�̵�� ���� ��Ʈ��ũ ������ (mmcn.ajou.ac.kr)
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define BUFSIZE 512

//������ 
typedef struct DATA {

	int seq_no;
	char message[1000];
}data;



// ���� �Լ� ���� ���
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
	// ���� �ʱ�ȭ
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
	
	// ������ ��ſ� ����� ����
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int random;

	// Ŭ���̾�Ʈ�� ������ ���
	while(1){
		
		// ������ �ޱ�

		//���� data ����ü �ʱ�ȭ 
		memset(&received_data, 0x00, sizeof(received_data));
		memset(buf, 0x00, sizeof(buf));

		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, &received_data, sizeof(received_data), 0, (SOCKADDR *)&clientaddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		// ���� ������ ���
		printf("[UDP/%s:%d] seq = %d, data = %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ack, received_data.message);
	

		//���濡�� ������ ������ ���� (������ �ϰ����� ���߷��� if else ������ ���� 
		if ((random = rand() % 101) >= 90) 
		{
		}
		else 
		{
			//���濡�� ������ ���� 
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

	// ���� ����
	WSACleanup();
	return 0;
}
