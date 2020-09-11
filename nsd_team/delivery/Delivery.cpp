#include "server_access.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

DELIVER_LOC location;
int send_location = 0;

// Thread function to communicate with a client
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET				client_sock;
	SOCKADDR_IN			clientaddr = *((SOCKADDR_IN*)arg);
	int					addrlen;
	int					retval;
	int					len;

	//make socket 
	client_sock = socket(AF_INET, SOCK_DGRAM, 0);

	while (send_location) {
		// 데이터 보내기
		retval = sendto(client_sock, (char*)&location.loc, sizeof(location.loc), 0,
			(SOCKADDR*)&clientaddr, sizeof(clientaddr));
		if (retval == SOCKET_ERROR) {
			continue;
		}
		Sleep(1000);
	}
	LOCATION loc;
	loc.latitude = -500;
	loc.longitude = -500;
	retval = sendto(client_sock, (char*)&loc, sizeof(loc), 0,
		(SOCKADDR*)&clientaddr, sizeof(clientaddr));
	// closesocket()
	closesocket(client_sock);

	return 0;
}

DWORD WINAPI LocationSimulator(LPVOID arg)
{
	srand((unsigned int)time(NULL));
	float lon, lat;

	lon = ((float)rand() / (float)RAND_MAX * 5.0);
	lat = ((float)rand() / (float)RAND_MAX * 5.0);
	printf("lon : %.4lf / lat : %.4lf\n", lon, lat);

	while (1)
	{
		lon += ((float)rand() / (float)RAND_MAX * 1.0) - 0.5;
		lat += ((float)rand() / (float)RAND_MAX * 1.0) - 0.5;
		location.loc.latitude = lat;
		location.loc.longitude = lon;
		//printf("lon : %.4lf / lat : %.4lf\n", lon, lat);
		Sleep(1000);
	}
	return 0;
}

int main()
{
	HANDLE			hThread;
	DWORD			ThreadId, ThreadId2;

	SOCKET sock;
	SOCKADDR_IN serveraddr, clientaddr;
	char inforequest[10] = { 0, };
	ORDER_INFO order = { 0, };


	// thread for location simulator
	hThread = CreateThread(NULL, 0, LocationSimulator, NULL, 0, &ThreadId2);

	//to initiate socket in windows 
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	printf("\t ***Delivery App started***\n");
	
	//make socket 
	sock = socket(AF_INET, SOCK_STREAM, 0);

	socket_init(&serveraddr, SERVER_DELIVERY_PORT); // initiate socket 

	int retval = connect(sock, (sockaddr*)&serveraddr, sizeof(SOCKADDR_IN));


	int result = login2server(sock); // login procedure 
	if (result == -1) {
		printf("error in login\n exit");
		return 0;
	}

	int tout = 5000; // to make time_out (for polling my status)
	char buf[BUFSIZ];
	strcpy(inforequest, "1");

	while (1)
	{

		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tout, sizeof(tout));
		send(sock, inforequest, sizeof(inforequest), 0); // give that data and polling my new order 

		memset((ORDER_INFO*)&order, 0x00, sizeof(ORDER_INFO));
		int retval = recv(sock, (char*)&order, sizeof(ORDER_INFO), 0);
		strcpy(location.type, "0");
		
		//show info of order
		if (retval != SOCKET_ERROR)
		{
			printf("\t####### order occured####### \n");
			printf("\n\tstore name : %s\n\tstore address(long,lati) : %s (%f , %f)\n", order.store_name, order.store_address, order.store_longitude, order.store_latitude);
			printf("\tuser location(long, lati) : %f , %f\n", order.customer_longitude, order.customer_latitude);

			memset(buf, 0x00, sizeof(buf));
			memset(&clientaddr,0 , sizeof(clientaddr));

			clientaddr.sin_family = AF_INET;
			clientaddr.sin_port = htons(SERVER_USER_P2P_PORT);
			clientaddr.sin_addr.s_addr = inet_addr((char *)order.customer_ip);

			send_location = 1;
			// thread for client
			hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)&clientaddr, 0, &ThreadId);

			printf("\n배달이 완료되면 -1을 입력해 주세요 : ");
			scanf("%s", buf);
			send(sock, buf, sizeof(buf), 0); // will change your status to idle 
		}
		send_location = 0;
		send(sock, (char*)&location, sizeof(location), 0); // for location upload
	}

	return 0;
}