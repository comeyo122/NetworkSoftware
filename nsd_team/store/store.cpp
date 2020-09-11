#include "server_access.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	SOCKET sock;
	SOCKADDR_IN serveraddr;

	char select[BUFSIZ] = { 0, };
	char buf[BUFSIZ] = { 0, };
	ORDER_INFO order = { 0, };


	//init socket
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	printf("\t ***Store App started***\n");

	sock = socket(AF_INET, SOCK_STREAM, 0);
	// socket initiate procedure
	socket_init(&serveraddr, SERVER_STORE_PORT);

	int retval = connect(sock, (sockaddr*)&serveraddr, sizeof(SOCKADDR_IN));

	//login to server 
	int result = login2server(sock);
	if (result == -1) {
		printf("error in login\n exit");
		return 0;
	}
	//get menu from server
	showmenu_from_server(sock);

	while (true) {
		memset(&order, '\0', sizeof(order));
		

		//dummy code
		printf("select your menu : ");
		scanf("%s", &select);
		
		send(sock, select, sizeof(select), 0);

		switch (atoi(select))
		{
		case 1:
			printf("-----------------------NEW ORDER--------------------\n");
			while (true) {
				int retval = recv(sock, (char *)&order, sizeof(order), 0);
				if (!strncmp((char*)&order, "quit",4)) break;
				printf("[NEW ORDER - id: %d] menu: %s\tcustomer location: (%.5f, %.5f)\tdeliver: %s\n"
					,order.id, order.order_list, order.customer_latitude, order.customer_longitude, order.deliver_name);
			}
			printf("---------------------------------------------------\n");

			break;
		case 2:
			printf("order which Deliver deperatured : ");
			scanf("%s", &buf);
			send(sock, buf, sizeof(buf), 0);
			printf("Selected order's status updated to 'delivering'!\n");
			break;
		case 3:
			return 0;
			break;
		default:
			break;
		}

	}

	return 0;
}