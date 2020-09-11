#include "platform.h"


int main()
{

	HANDLE			hThread[3];
	DWORD			ThreadId[3];

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// thread for general user
	hThread[0] = CreateThread(NULL, 0, listening_user, NULL, 0, &ThreadId[0]);
	CloseHandle(hThread[0]);

	//thread for store
	hThread[1] = CreateThread(NULL, 0, listening_store, NULL, 0, &ThreadId[1]);
	CloseHandle(hThread[1]);

	//thread for delivery
	hThread[2] = CreateThread(NULL, 0, listening_delivery, NULL, 0, &ThreadId[2]);
	CloseHandle(hThread[2]);

	WaitForMultipleObjects(3, hThread, TRUE, INFINITE);



	while (1)
	{  

	}

	return 0;
}