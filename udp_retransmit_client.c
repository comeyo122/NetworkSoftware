// UDPEchoClient.c
// UDP echo client
//
// 과목명: 네트워크소프트웨설계
// 아주대학교 소프트웨어학과
// 이동 멀티미디어 융합 네트워크 연구실 (mmcn.ajou.ac.kr)
//
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 데이터 입력해주는 함수
void processdata(data *my_data, int seqno, char *buf) {

	char *ptr;
	//내가 ack받은 seq_no
	my_data->seq_no = seqno;
	strcpy(my_data->message, buf);

}


int main(int argc, char* argv[])
{
	int retval;
	int seq_no = 0;
	data msg;
	int N1 = 0, N2 = 0, i=0;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("err in sock\n");
		exit(0);
	}

	// 소켓 주소 구조체 초기화
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (i<100) {

		memset(buf, 0x00, sizeof(buf));
		// 데이터 입력
		printf("\n[보낼 데이터] seq = %d, msg = ", seq_no);
		fgets(buf, BUFSIZE + 1, stdin);

		

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// 데이터 삽입
		memset(&msg, 0x00, sizeof(msg));
		processdata(&msg, seq_no, buf);

		// 데이터 보내기


		//소켓에 Timeout 설정 
		int tout = 1000;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(tout));
		++N1;
		retval = sendto(sock, &msg, sizeof(msg), 0, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
		
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
		}

		

		// 데이터 받기
		addrlen = sizeof(peeraddr);
		while (1) {
			retval = recvfrom(sock, &msg, sizeof(msg), 0, (SOCKADDR *)&peeraddr, &addrlen);
			//재전송 루틴
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					++N2;
					printf("[시간초과, 재전송]  seq = %d, msg = %s\n", seq_no, msg.message);
					setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(tout));
					sendto(sock, &msg, sizeof(msg), 0, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
					continue;
				}
			}
			break;
		}
		printf("[받은 데이터] ACK = %d , msg = %s\n", msg.seq_no,msg.message);

		seq_no += strlen(msg.message);
		if (strcmp(buf, "QUIT") == 0) {
			break;
		}
		i++;
	}
	printf("P=0.1 -- N1 : %d, N2 : %d, N3 : %d\n", N1, N2, N1 + N2);
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
