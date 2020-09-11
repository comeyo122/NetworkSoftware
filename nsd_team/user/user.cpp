#include "server_access.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	SOCKET sock, udpsock;
	SOCKADDR_IN serveraddr, udpaddr, udpcladdr;
	int select;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;


	printf("\t ***User App started***\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	socket_init(&serveraddr, SERVER_USER_PORT);

	int retval = connect(sock, (sockaddr*)&serveraddr, sizeof(SOCKADDR_IN));

	int result = login2server(sock);

	if (result == -1) {
		printf("error in login\nexit");
		return 0;
	}

	while (true) {
		//get menu from server
		showmenu_from_server(sock);

		printf("select your menu : ");
		scanf(" %d", &select);

		if (select == 2)
			return 0;

		// get store and menu from server
		select_store_and_menu(sock);

		udpsock = socket(AF_INET, SOCK_DGRAM, 0);

		memset(&udpaddr, 0x00, sizeof(SOCKADDR_IN));
		memset(&udpcladdr, 0x00, sizeof(SOCKADDR_IN));
		udpaddr.sin_family = AF_INET;
		udpaddr.sin_port = htons(SERVER_USER_P2P_PORT);
		udpaddr.sin_addr.s_addr = htonl(INADDR_ANY);

		retval = bind(udpsock, (SOCKADDR*)&udpaddr, sizeof(udpaddr));

		int alen = sizeof(udpcladdr);

		printf("Start tracking deliver\n\n");

		while (true) {
			LOCATION deliver_location = { 0, };
			recvfrom(udpsock, (char*)&deliver_location, sizeof(deliver_location), 0, (SOCKADDR*)&udpcladdr, &alen);
			if (deliver_location.latitude == -500 && deliver_location.longitude == -500) break;
			printf("[Deliver Position] %.5f, %.5f\n", deliver_location.latitude, deliver_location.longitude);
			Sleep(1000);
		}

		printf("Deliver is done!!\n\n\n");
	}

	return 0;
}