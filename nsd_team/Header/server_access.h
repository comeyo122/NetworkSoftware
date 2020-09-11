#pragma once
#define _CRT_NONSTDC_NO_DEPRECAT
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_STORE_PORT 9000
#define SERVER_DELIVERY_PORT 9001
#define SERVER_USER_PORT 9002
#define SERVER_USER_P2P_PORT 9500
#define SERVER_ADDR "127.0.0.1"
#define DBNAME "nso"
#define DBID "root"
#define DBPW "asd910130"
#define DBPATH "localhost"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

typedef struct LOCATION
{
	double longitude;
	double latitude;
}LOCATION;

typedef struct DELIVER_LOC
{
	char type[2];
	LOCATION loc;
}DELIVER_LOC;

// data for order_info 
typedef struct ORDER_INFO {
	double customer_longitude;
	double customer_latitude;
	unsigned char customer_ip[20];
	unsigned short customer_port;
	double store_longitude;
	double store_latitude;
	char store_address[30];
	char store_name[30];
	char order_list[70];
	char deliver_name[30];
	int id;

}ORDER_INFO;

void socket_init(SOCKADDR_IN* serveraddr, int port);
int login2server(SOCKET sock);
void showmenu_from_server(SOCKET sock);
void select_store_and_menu(SOCKET sock);