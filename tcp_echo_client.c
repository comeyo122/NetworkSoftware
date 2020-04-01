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
#include <string.h>

#define BUFSIZE 1500

//MSS 1500byte, CL -> Server로 보내는구조체 
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

//입력한 메세지를 파싱해서 구조체로 넣어주는 함수 
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
	int retval;
	toserver MSG;
	tocl response;
	SOCKET sock;
	SOCKADDR_IN serveraddr;
	char buf[BUFSIZE + 1];
	int len;


	// 윈속 초기화
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

	// 서버와 데이터 통신
	while (1) {

		// 구조체 초기화
		memset(&MSG, 0x00, sizeof(MSG));
		memset(&response, 0x00, sizeof(response));

		// 데이터 입력
		ZeroMemory(buf, sizeof(buf));
		printf("\n[보낼 데이터] ");
		fgets(buf, BUFSIZE + 1, stdin);



		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		//구조체에 파싱을 통한 데이터 처리
		if (strcmp(buf, "QUIT") == 0)
			strcpy(MSG.msg, "QUIT");
		else
			MSG = process_data(buf);

		// 데이터 보내기
		retval = send(sock, (const char*)&MSG, sizeof(MSG), 0);
		if (retval == SOCKET_ERROR) {
			exit(0);
			break;
		}

		// 데이터 받기
		retval = recv(sock, (char *)&response, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			exit(0);
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("%d bytes echo(%s)\n", response.len, response.msg);
		if (strcmp(response.msg, "QUIT") == 0 && MSG.stno == 0)
			break;
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}