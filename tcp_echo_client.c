// COechoClnt.c
// TCP echo client
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
#include <string.h>

#define BUFSIZE 1500

//MSS 1500byte, CL -> Server�� �����±���ü 
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

//�Է��� �޼����� �Ľ��ؼ� ����ü�� �־��ִ� �Լ� 
toserver process_data(char *msg) {

	int i = 0;
	toserver data;
	char *tmp;

	memset(&data, 0x00, sizeof(data));
	tmp = strtok(msg, ",");

	while (tmp != NULL) {

		if (i == 0)
			data.stno = htonl(atoi(tmp));
		else if (i == 1)
			data.namelen = htonl(atoi(tmp));
		else if (i == 2)
			strcpy(data.name, tmp);
		else
			strcpy(data.msg, tmp);

		tmp = strtok(NULL, ",");
		i++;
	}
	return data;
}

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
	int retval;
	toserver MSG;
	tocl response;
	SOCKET sock;
	SOCKADDR_IN serveraddr;
	char buf[BUFSIZE + 1];
	int len;


	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) exit(0);

	// server address
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect()
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		exit(0);

	// ������ ������ ���
	while (1) {

		// ����ü �ʱ�ȭ
		memset(&MSG, 0x00, sizeof(MSG));
		memset(&response, 0x00, sizeof(response));

		// ������ �Է�
		ZeroMemory(buf, sizeof(buf));
		printf("\n[���� ������] ");
		fgets(buf, BUFSIZE + 1, stdin);



		// '\n' ���� ����
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		//����ü�� �Ľ��� ���� ������ ó��
		if (strcmp(buf, "QUIT") == 0)
			strcpy(MSG.msg, "QUIT");
		else
			MSG = process_data(buf);

		// ������ ������
		retval = send(sock, (const char*)&MSG, sizeof(MSG), 0);
		if (retval == SOCKET_ERROR) {
			exit(0);
			break;
		}

		// ������ �ޱ�
		retval = recv(sock, (char *)&response, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			exit(0);
			break;
		}
		else if (retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("%d bytes echo(%s)\n", response.len, response.msg);
		if (strcmp(response.msg, "QUIT") == 0 && MSG.stno == 0)
			break;
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}