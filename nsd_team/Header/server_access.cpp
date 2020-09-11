#include "server_access.h"
#include <stdio.h>

// initiate socket 
void socket_init(SOCKADDR_IN* serveraddr, int port)
{
	memset((SOCKADDR_IN*)serveraddr, 0x00, sizeof(SOCKADDR_IN));
	serveraddr->sin_family = AF_INET;
	serveraddr->sin_port = htons(port);
	serveraddr->sin_addr.s_addr = inet_addr(SERVER_ADDR);

}

// login to server 
int login2server(SOCKET sock)
{
	char buf[BUFSIZ];

	memset(buf, 0x00, sizeof(buf));
	recv(sock, buf, sizeof(buf), 0);
	printf("%s", buf);
	memset(buf, 0x00, sizeof(buf));
	scanf("%[^\n]", buf);
	send(sock, buf, sizeof(buf), 0);

	memset(buf, 0x00, sizeof(buf));
	recv(sock, buf, sizeof(buf), 0);

	int result = atoi(buf);
	return result;

}
// recv my_menu from server
void showmenu_from_server(SOCKET sock)
{
	char menu[BUFSIZ];

	memset((char*)menu, 0x00, sizeof(menu));
	int retval = recv(sock, menu, sizeof(menu), 0);
	printf("%s", menu);
}


// for user, give selected menu and store to server
void select_store_and_menu(SOCKET sock)
{

	char buf[5000] = { 0, };

	recv(sock, buf, sizeof(buf), 0);
	printf(" %s\n",buf);

	memset(buf, 0x00, sizeof(buf));
	printf("select your choice : ");
	scanf("%s", buf);
	send(sock, buf, sizeof(buf), 0);
	memset(buf, 0x00, sizeof(buf));
	recv(sock, buf, sizeof(buf), 0);

	printf("%s\n", buf);
	memset(buf, 0x00, sizeof(buf));

	printf("select menu to order (menu1,menu2,menu3,etc) : ");
	scanf("%s", buf);
	send(sock, buf, sizeof(buf), 0);

	printf("Order Done!!\n\n");
}