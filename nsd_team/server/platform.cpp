#include "platform.h"


// initiate socket 
void socket_init(SOCKET* listen_sock, SOCKADDR_IN* serveraddr, int port)
{

	memset((SOCKADDR_IN*)serveraddr, 0x00, sizeof(SOCKADDR_IN));
	serveraddr->sin_family = AF_INET;
	serveraddr->sin_port = htons(port);
	serveraddr->sin_addr.s_addr = inet_addr(SERVER_ADDR);

	int retval = bind(*listen_sock, (SOCKADDR*)serveraddr, sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) exit(0);
}




// user listening part 
DWORD WINAPI listening_user(LPVOID arg)
{
	SOCKET listen_sock, client_sock;
	SOCKADDR_IN serveraddr, clientaddr;
	HANDLE			hThread;
	DWORD			ThreadId;
	user_internet_info user_struct;

	// make socket
	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) exit(0);

	//socket_init -> sockaddr setting, binding
	socket_init(&listen_sock, &serveraddr, SERVER_USER_PORT);

	int retval = listen(listen_sock, 5);
	if (retval == SOCKET_ERROR) exit(0);

	while (1)
	{
		//accept user's connect
		int addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (sockaddr*)&clientaddr, &addrlen);

		printf("[TCP_SERVER] connection with client(USER) (IP : %s, port : %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		user_struct.client_sock = client_sock;
		user_struct.clientaddr = clientaddr;
		//processuser
		hThread = CreateThread(NULL, 0, ProcessUser, (LPVOID)&user_struct, 0, &ThreadId);
		CloseHandle(hThread);
	}
	return 0;
}


// listening deliver
DWORD WINAPI listening_delivery(LPVOID arg)
{
	SOCKET listen_sock, client_sock;
	SOCKADDR_IN serveraddr, clientaddr;
	HANDLE			hThread;
	DWORD			ThreadId;
	// make socket
	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) exit(0);

	//socket_init -> sockaddr setting, binding
	socket_init(&listen_sock, &serveraddr, SERVER_DELIVERY_PORT);

	int retval = listen(listen_sock, 5);
	if (retval == SOCKET_ERROR) exit(0);

	while (1)
	{
		//accept deliver's connect
		int addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (sockaddr*)&clientaddr, &addrlen);

		printf("[TCP_SERVER] connection with client(Delivery) (IP : %s, port : %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//processdeliver
		hThread = CreateThread(NULL, 0, ProcessDelivery, (LPVOID)client_sock, 0, &ThreadId);
		CloseHandle(hThread);
	}
	return 0;
}


// listening for store
DWORD WINAPI listening_store(LPVOID arg)
{
	SOCKET listen_sock, client_sock;
	SOCKADDR_IN serveraddr, clientaddr;
	HANDLE			hThread;
	DWORD			ThreadId;
	
	// make socket
	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) exit(0);

	//socket_init -> sockaddr setting, binding
	socket_init(&listen_sock, &serveraddr, SERVER_STORE_PORT);

	int retval = listen(listen_sock, 5);
	if (retval == SOCKET_ERROR) exit(0);

	while (1)
	{
		//accept user's connect
		int addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (sockaddr*)&clientaddr, &addrlen);

		printf("[TCP_SERVER] connection with client(store) (IP : %s, port : %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//processuser
		hThread = CreateThread(NULL, 0, ProcessStore, (LPVOID)client_sock, 0, &ThreadId);
		CloseHandle(hThread);
	}


	return 0;
}


DWORD WINAPI ProcessStore(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	AGENT_INFO USERINFO = { 0, };
	char buf[BUFSIZ] = { 0, };
	ORDER_INFO order = { 0, };

	MYSQL_RES* res = { 0, };
	MYSQL_ROW row;

	printf("login_session\n");
	//validate user's login


	if (login_from_user(client_sock, &USERINFO,STORE_LOGIN) != -1) // routine for success
	{
		printf("login Sucess\n");
		strcpy(buf, "1");
		send(client_sock, buf, 1, 0);
		Sleep(1000);
		showmenu(client_sock, SERVER_STORE_PORT);
	}

	else // routine for login failed 
	{
		printf("login Failed\n");
		strcpy(buf, "-1");
		send(client_sock, buf, 2, 0);
		return 0;
	}


	while (true) {
		memset(&buf, '\0', sizeof(buf));
		recv(client_sock, buf, sizeof(buf), 0);

		switch (atoi(buf))
		{
		case 1:
			res = getneworder(USERINFO.ID);

			while ((row = mysql_fetch_row(res)) != NULL)
			{
				order.customer_longitude = atof(row[0]);
				order.customer_latitude = atof(row[1]);
				order.store_latitude = atof(row[2]);
				order.store_longitude = atof(row[3]);
				strcpy(order.store_address, row[4]);
				strcpy(order.store_name, row[5]);
				strcpy(order.order_list, row[6]);
				strcpy(order.deliver_name, row[7]);
				order.id = atoi(row[8]);
				int retval = send(client_sock, (char*)&order, sizeof(order), 0);
				if (retval == SOCKET_ERROR)
					break;
			}

			strcpy(buf, "quit");
			send(client_sock, buf, 4, 0);
			break;
		case 2:
			recv(client_sock, buf, sizeof(buf), 0);
			set_order_delivering(atoi(buf));
			break;
		case 3:
			return 0;
		default:
			break;
		}
	}
	
	return 0;
}


DWORD WINAPI ProcessDelivery(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	AGENT_INFO USERINFO = { 0, };
	printf("login_session\n");
	//validate user's login
	char buf[100] = { 0, };
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);
	ORDER_INFO order = { 0, };
	int id;
	DELIVER_LOC deliver_location;

	if (login_from_user(client_sock, &USERINFO, DELIVER_LOGIN) != -1) // routine for success
	{
		printf("login Sucess\n");
		strcpy(buf, "1");
		send(client_sock, buf, 1, 0);
		deliverystatus(USERINFO.ID, DELIVER_LOGIN);
	}
	else // routing for login failed 
	{
		printf("login Failed\n");
		strcpy(buf, "-1");
		send(client_sock, buf, 2, 0);
		closesocket(client_sock);
		return 0;
	}

	//  request from delivery -> change location, polling request 
	while (1)
	{
		int retval = recv(client_sock, buf, sizeof(buf), 0);
		if (retval == SOCKET_ERROR)
			deliverystatus(USERINFO.ID, DELIVER_LOGOUT);

		switch (atoi(buf))
		{
		case -1:  // delivery finish , change status working -> idle
			deliverystatus(USERINFO.ID, DELIVER_WORK_END);
			break;
		case 0: // change location
			deliver_location = *((DELIVER_LOC *)buf);
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, "update deliver set latitude = %f , longitude = %f where name=\"%s\" AND password = \"%s\"", deliver_location.loc.latitude,deliver_location.loc.longitude,USERINFO.ID, USERINFO.PW);
			mysql_query(conn, buf);
			break;
		case 1: // find info about deliver
			int index = atoi(buf);
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf,"select * from order_info where deliver_name='%s' and status='deliver_set' ", USERINFO.ID);
			mysql_query(conn, buf);
			res = mysql_store_result(conn);

			if ((row = mysql_fetch_row(res)) != NULL) // get the delivery info
			{
				order.customer_longitude = atof(row[0]);
				order.customer_latitude = atof(row[1]);
				order.store_latitude = atof(row[2]);
				order.store_longitude = atof(row[3]);
				strcpy(order.store_address, row[4]);
				strcpy(order.store_name, row[5]);
				strcpy(order.order_list, row[6]);
				strcpy(order.deliver_name, row[7]);
				order.id = atoi(row[8]);
				strcpy((char *)order.customer_ip, row[9]);

				retval = send(client_sock, (char*)&order, sizeof(ORDER_INFO), 0);
				if (retval == SOCKET_ERROR)
					deliverystatus(USERINFO.ID, DELIVER_LOGOUT);
			}
			break;
		}
	}

	return 0;
}


DWORD WINAPI ProcessUser(LPVOID user_struct)
{
	user_internet_info *user_internet;
	user_internet = (user_internet_info *)user_struct;
	AGENT_INFO USERINFO = { 0, };
	char buf[BUFSIZ] = { 0, };
	int order_num;
	printf("login_session\n");

	//validate user's login


	if (login_from_customer(user_internet, &USERINFO, USER_LOGIN) != -1) // routine for success
	{
		printf("login Sucess\n");
		strcpy(buf, "1");
		send(user_internet->client_sock, buf, 1, 0);
		showmenu(user_internet->client_sock, SERVER_USER_PORT);
	}
	else // routing for login failed 
	{
		printf("login Failed\n");
		strcpy(buf, "-1");
		send(user_internet->client_sock, buf, 2, 0);
		return 0;
	}

	// make order and make some order_info data 
	order_num = get_store_menu_fromdb(user_internet->client_sock, USERINFO);
	
	//find deliver
	find_nearby_driver(order_num);

	return 0;
}

// find store menu from Database 
int get_store_menu_fromdb(SOCKET client_sock, AGENT_INFO USERINFO)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	char buf[5000];
	char order_list[BUFSIZ] = { 0, };
	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);
	int id;
	
	//query and get data from database 

	memset(buf, 0x00, sizeof(buf));
	strcpy(buf, "\tID \t STORE\n");

	mysql_query(conn, "SELECT id, name from store");

	// to show list of the store 
	res = mysql_store_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		sprintf(buf, "%s\t %s\t%s\n", buf, row[0], row[1]);
	}

	// send data to client(store info)
	send(client_sock, buf, sizeof(buf), 0);

	//get the user's choice
	memset(buf, 0x00, sizeof(buf));
	recv(client_sock, buf, sizeof(buf), 0);
	int store_no = atoi(buf);
	
	//find menu which customer select 
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select id, name, price from menu where store_id = %d", store_no);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"\tMENU\n\tid\tname\tprice\n");

	while ((row = mysql_fetch_row(res)) != NULL)
	{	
		sprintf(buf,"%s\t %s\t%s\t%s\n",buf,row[0],row[1],row[2]);
	}
	send(client_sock, buf, sizeof(buf), 0); // send menu info to client 

	//get the orde info
	memset(buf, 0x00, sizeof(buf));
	recv(client_sock, order_list, sizeof(order_list), 0);

	// make some info to order_info
	sprintf(buf,"insert into order_info (order_list) VALUES (\"%s\")", order_list);
	mysql_query(conn, buf);

	//get the id of the order
	mysql_query(conn, "select id from order_info");
	res = mysql_store_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		 id = atoi(row[0]);
	}
	

	// get the store info to make order_info 
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select address, latitude, longitude, name from store where id=%d", store_no);
	mysql_query(conn, buf);

	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update order_info set store_address=\"%s\", store_latitude=%f, store_longitude=%f, store_name=\"%s\" WHERE id =%d ", row[0], atof(row[1]), atof(row[2]), row[3] ,id);	
	mysql_query(conn, buf);
	
	//get the user info to make order_info 
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf,"select longitude,latitude from customer where name=\"%s\" AND password=\"%s\"", USERINFO.ID,USERINFO.PW);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update order_info set customer_longitude=%f, customer_latitude= %f where id=%d", atof(row[0]), atof(row[1]), id);
	mysql_query(conn, buf);
	
	// get ip,name
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select ip, port from customer where name='%s' and password='%s'", USERINFO.ID , USERINFO.PW);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	int my_port = atoi(row[1]);
	
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf , "update order_info set customer_ip = '%s', customer_port = '%d', status='deliver_set' where id = %d", row[0], my_port,id);
	mysql_query(conn, buf);

	return id;
}

// to find out deliver
void find_deliver(int order_num)
{

	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;

	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);
	char buf[BUFSIZ] = { 0, };

	sprintf(buf, "update order_info set deliver_name =\"deliverA\", status='deliver_set' where id = %d", order_num );
	mysql_query(conn, buf);
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update deliver set status='delivering' where name = 'deliverA'");
	mysql_query(conn, buf);

}

void find_nearby_driver(int order_num)
{

	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;

	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);
	char buf[BUFSIZ] = { 0, };

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select store_name from order_info where id=%d", order_num);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select latitude, longitude from store where name=\"%s\"", row[0]);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	int store_lati = atoi(row[0]);
	int store_long = atoi(row[1]);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "select name from deliver where %d < latitude and latitude < %d and %d <  latitude and latitude < %d"
		, store_lati - 300, store_lati + 300, store_long - 300, store_long + 300);
	mysql_query(conn, buf);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update order_info set deliver_name =\"%s\", status='deliver_set' where id = %d", row[0], order_num);
	mysql_query(conn, buf);
	sprintf(buf, "update deliver set status='delivering' where name = '%s'", row[0]);
	mysql_query(conn, buf);
}

//show menu for the user(customer, store ) 
void showmenu(SOCKET sock, int port)
{
	char menu[BUFSIZ];
	 
	memset((char*)menu, 0x00, BUFSIZ);

	switch (port)
	{
	case SERVER_USER_PORT:
		strcpy(menu, "1.SHOW STORES\n2.END\n");
		send(sock, menu, sizeof(menu), 0);
		break;
	case SERVER_STORE_PORT:
		strcpy(menu, "1.GET Order LIST\n2.update order to delivering\n3.END\n");
		send(sock, menu, sizeof(menu), 0);
		break;
	default:
		break;
	}

}

//show menu for the user(customer, store ) 
MYSQL_RES* getneworder(char* name)
{
	char buf[BUFSIZ];
	memset((char*)buf, 0x00, BUFSIZ);

	MYSQL* conn;
	MYSQL_RES* res;

	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "SELECT * from order_info where store_name=\"%s\" and status=\"deliver_set\"", name);

	mysql_query(conn, buf);

	res = mysql_store_result(conn);
	return res;
}

void set_order_delivering(int id)
{
	char buf[BUFSIZ];
	memset((char*)buf, 0x00, BUFSIZ);

	MYSQL* conn;
	MYSQL_RES* res;

	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);

	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update order_info set status=\"delivering\" where id=%d", id);

	mysql_query(conn, buf);
	return;
}


// login routine 
int login_from_user(SOCKET client_sock, AGENT_INFO *USERINFO, int flag)
{
	char buf[BUFSIZ], USERID[BUFSIZ], USERPW[BUFSIZ];

	//get data and parse from user's input
	memset(buf, 0x00, sizeof(buf));
	strcpy(buf, "ID와 PW 를 입력하시오(ID,PW) : ");
	send(client_sock, buf, sizeof(buf), 0);
	memset(buf, 0x00, sizeof(buf));
	recv(client_sock, buf, sizeof(buf), 0);

	memset(USERID, 0x00, sizeof(USERID));
	memset(USERPW, 0x00, sizeof(USERPW));
	sscanf(buf, "%[^','],%[^',']", USERID, USERPW);

	// if validate faileed return -1
	if (validatefromdb(USERID, USERPW, USERINFO, flag) == -1) {
		return -1;
	}

	return 1;
}

int login_from_customer(user_internet_info *user, AGENT_INFO* USERINFO, int flag)
{
	char buf[BUFSIZ], USERID[BUFSIZ], USERPW[BUFSIZ];
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	
	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);

	//get data and parse from user's input
	memset(buf, 0x00, sizeof(buf));
	strcpy(buf, "ID와 PW 를 입력하시오(ID,PW) : ");
	send(user->client_sock, buf, sizeof(buf), 0);
	memset(buf, 0x00, sizeof(buf));
	recv(user->client_sock, buf, sizeof(buf), 0);

	memset(USERID, 0x00, sizeof(USERID));
	memset(USERPW, 0x00, sizeof(USERPW));
	sscanf(buf, "%[^','],%[^',']", USERID, USERPW);

	// if validate faileed return -1
	if (validatefromdb(USERID, USERPW, USERINFO, flag) == -1) {
		return -1;
	}
	
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "update customer set ip ='%s' , port = %d where name = '%s' and password='%s' ", inet_ntoa(user->clientaddr.sin_addr), ntohs(user->clientaddr.sin_port), USERID, USERPW);
	mysql_query(conn, buf);


	
	return 1;
}

int validatefromdb(char* id, char* pw, AGENT_INFO *USERINFO,int flag)
{

	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;

	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);

	//query and get data from database distinguise by port 
	switch (flag) {
	case USER_LOGIN:
		mysql_query(conn, "SELECT name, password from customer");
		break;
	case STORE_LOGIN:
		mysql_query(conn, "SELECT name, password from store");
		break;
	case DELIVER_LOGIN:
		mysql_query(conn, "SELECT name, password from deliver");
		break;
	default:
		break;
	}

	res = mysql_store_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		//getting data and validate id,pw
		if (strcmp(id, row[0]) == 0 && strcmp(pw, row[1]) == 0)
		{
			strcpy(USERINFO->ID, row[0]);
			strcpy(USERINFO->PW, row[1]);
			return 0;
		}
	}

	return -1;
}

// change deliver's state 
void deliverystatus(char *id, int status)
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
	char buf[BUFSIZ] = { 0, };
	// connect to db
	conn = mysql_init(NULL);
	conn = mysql_real_connect(conn, DBPATH, DBID, DBPW, DBNAME, 0, NULL, 0);

	
	switch (status) {
		case DELIVER_LOGIN: // off -> idle
			sprintf(buf,"UPDATE deliver SET status='idle' WHERE name='%s'", id);
			mysql_query(conn, buf);
			break;

		case DELIVER_WORK_END: 
			//working ->idle 
			sprintf(buf, "UPDATE deliver SET status='idle' WHERE name='%s'", id);
			mysql_query(conn, buf);
			//order_info: delivering->done
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, "UPDATE order_info SET status='done' where deliver_name ='%s' " , id);
			mysql_query(conn, buf);
			break;
		case DELIVER_LOGOUT: // idle -> off
			sprintf(buf,"UPDATE deliver SET status='off' WHERE name='%s'", id);
			mysql_query(conn, buf);
			break;
		default:
			break;

	}
}