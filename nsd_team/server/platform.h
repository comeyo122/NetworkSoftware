#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_STORE_PORT 9000
#define SERVER_DELIVERY_PORT 9001
#define SERVER_USER_PORT 9002

#define SERVER_ADDR "127.0.0.1"
#define DBNAME "nso"
#define DBID "root"
#define DBPW "1234qwer"
#define DBPATH "localhost"
#define DELIVER_LOGIN 0
#define DELIVER_WORK_END 1 
#define DELIVER_WORK_START3
#define DELIVER_LOGOUT 2
#define USER_LOGIN 1
#define STORE_LOGIN 2

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include "mysql.h"

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

// data for socket 
typedef struct MY_SERVER {

	SOCKET listen_sock, client_sock;
	SOCKADDR_IN serveraddr, clientaddr;

}serverinfo;

// data for loged_in user's info 
typedef struct AGENT_INFO {
	char ID[BUFSIZ];
	char PW[BUFSIZ];

}AGENTINFO;

typedef struct user_internet_info {

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
}user_internet_info;

void socket_init(SOCKET* listen_sock, SOCKADDR_IN* serveraddr);
int login_from_user(SOCKET client_sock, AGENTINFO *USERINFO, int flag);
int login_from_customer(user_internet_info *user, AGENTINFO* USERINFO, int flag);
DWORD WINAPI listening_user(LPVOID arg);
DWORD WINAPI listening_delivery(LPVOID arg);
DWORD WINAPI listening_store(LPVOID arg);
DWORD WINAPI ProcessUser(LPVOID arg);
DWORD WINAPI ProcessStore(LPVOID arg);
DWORD WINAPI ProcessDelivery(LPVOID arg);
int validatefromdb(char* id, char* pw, AGENT_INFO *USERINFO, int flag);
void set_order_delivering(int id);
void deliverystatus(char *id, int current_status);
void showmenu(SOCKET sock, int port);
MYSQL_RES* getneworder(char* name);
int get_store_menu_fromdb(SOCKET sock, AGENT_INFO userinfo);
void find_deliver(int order_num);
void find_nearby_driver(int order_num);
