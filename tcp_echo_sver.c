// COechoSvr.cpp
// TCP echo server
//
// �����: ��Ʈ��ũ����Ʈ�����
// ���ִ��б� ����Ʈ�����а�
// ����� ��Ƽ�̵�� ��� ��Ʈ��ũ ������ (mmcn.ajou.ac.kr)
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 1500

//MSS 1500 byte, CL -> Server�� ������ ����ü 
typedef struct TOSERVER {

	int stno;
	int namelen;
	char name[30];
	char msg[1462];
}toserver;

//Server -> Client�� ������ ����ü 
typedef struct TOCLIENT {

	int len;
	char msg[1462];
}tocl;

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char* argv[])
{
	// ������ ��ſ� ����� ����
	SOCKET listen_sock;
	SOCKET client_sock;
	SOCKADDR_IN serveraddr;
	SOCKADDR_IN clientaddr;
	toserver fromcl;
	tocl response;

	int		addrlen;
	char	buf[BUFSIZE + 1];
	int		retval, msglen;



	// ���� �ʱ�ȭ
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

		// Ŭ���̾�Ʈ�� ������ ���
		while (1) {


			//����ü �ʱ�ȭ
			memset(&fromcl, 0x00, sizeof(fromcl));
			memset(&response, 0x00, sizeof(response));

			// ������ �ޱ�
			msglen = recv(client_sock, (char *)&fromcl, sizeof(fromcl), 0);
			if (msglen == SOCKET_ERROR) {
				exit(0);
				break;
			}


			// ���� ������ ���

			buf[msglen] = '\0';
			printf("[TCP/%s:%d]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));


			if (strcmp(fromcl.msg, "QUIT") == 0 && fromcl.stno == 0) {
				//�ܼ��� QUIT�� ������ ���� 
				printf("�޼���:QUIT\n");
				response.len = 4;
				strcpy(response.msg, "QUIT");
				retval = send(client_sock, (const char*)&response, sizeof(response), 0);
				break;
			}
			else
			{
				// �Ϲ����� ���ų�, �������� ���ڸ޼����μ��� QUIT�� ������ ���
				printf("�й�:%d, �̸�:%s, �޽���:%s\n", ntohl(fromcl.stno), fromcl.name, fromcl.msg);

				// ������ ó��
				response.len = strlen(fromcl.msg);
				strcpy(response.msg, fromcl.msg);
			}
			// ������ ������
			retval = send(client_sock, (const char*)&response, sizeof(response), 0);
			if (retval == SOCKET_ERROR) {
				exit(0);
				break;
			}

		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);
	// ���� ����
	WSACleanup();
	return 0;
}